#include "stdafx.h"
#include "EditorMaterialConverter.h"
#include "DefaultShaderParameters.h"
#include "OgreViewportWidget.h"
#include "Shader.h"
#include "Parameter.h"

DefaultShaderParameters default_shader_parameters("database/DefaultShaderParameterDatabase.xml");

EditorMaterialConverter::EditorMaterialConverter() {

}

Ogre::Material *EditorMaterialConverter::convertMaterial(LibGens::Material *material, LibGens::ShaderLibrary *shader_library) {
	if (material) {
		Ogre::String material_name = material->getName();
		Ogre::Material *ogre_material = Ogre::MaterialManager::getSingleton().create(material_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).get();
		if (ogre_material) {
			Ogre::Technique *technique = ogre_material->getTechnique(0);
			Ogre::Pass *pass = technique->getPass(0);

			// Force it to use 16 total texture units
			for (size_t i = 0; i < 16; i++) {
				Ogre::TextureUnitState *texture_unit_state = pass->createTextureUnitState("black.dds");
			}

			// Search for shaders based on the material's shader
			updateMaterialShaderParameters(shader_library, ogre_material, material, false);

			// Create low-end technique
			Ogre::Technique *low_end_technique=ogre_material->createTechnique();
			low_end_technique->setSchemeName(OgreViewportWidget::LowEndTechniqueName);
			pass = low_end_technique->createPass();

			LibGens::Texture *texture=material->getTextureByUnit(LIBGENS_MATERIAL_TEXTURE_UNIT_DIFFUSE);
			if (texture) {
				Ogre::TextureUnitState *state=pass->createTextureUnitState(texture->getName() + LIBGENS_TEXTURE_FILE_EXTENSION);
				state->setTextureAddressingMode(Ogre::TextureUnitState::TAM_WRAP);
			}
		}
	}

	return NULL;
}

void EditorMaterialConverter::updateMaterialShaderParameters(LibGens::ShaderLibrary *shader_library, Ogre::Material *ogre_material, LibGens::Material *material, bool no_gi) {
	Ogre::Pass *pass = ogre_material->getTechnique(0)->getPass(0);
	if (!pass) return;

	// Search for shaders based on the material's shader
	string shader_name = material->getShader();
	LibGens::Shader *vertex_shader=NULL;
	LibGens::Shader *pixel_shader=NULL;
	shader_library->getMaterialShaders(shader_name, vertex_shader, pixel_shader, false, no_gi, false);

	if (vertex_shader && pixel_shader) {
		// Vertex Shader
		string vertex_shader_name = vertex_shader->getShaderFilename();
		if (!vertex_shader->hasExtra()) {
			Ogre::GpuProgramPtr vertex = Ogre::GpuProgramManager::getSingletonPtr()->createProgram(vertex_shader_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
																								   vertex_shader_name + ".wvu.asm", Ogre::GPT_VERTEX_PROGRAM, "vs_3_0");
			vertex_shader->setExtra(vertex_shader_name);
		}
		pass->setVertexProgram(vertex_shader_name);

		
		if (pass->hasVertexProgram()) {
			Ogre::GpuProgramParametersSharedPtr vp_parameters = pass->getVertexProgramParameters();
			vector<string> shader_parameter_filenames=vertex_shader->getShaderParameterFilenames();

			vp_parameters->setTransposeMatrices(true);

			for (size_t i=0; i < shader_parameter_filenames.size(); i++) {
				LibGens::ShaderParams *shader_params=shader_library->getVertexShaderParams(shader_parameter_filenames[i]);

				if (shader_params) {
					setShaderParameters(pass, vp_parameters, material, shader_params);
				}
			}
		}

		// Pixel Shader
		string pixel_shader_name = pixel_shader->getShaderFilename();
		if (!pixel_shader->hasExtra()) {
			Ogre::GpuProgramPtr shader = Ogre::GpuProgramManager::getSingletonPtr()->createProgram(pixel_shader_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
																								   pixel_shader_name + ".wpu.asm", Ogre::GPT_FRAGMENT_PROGRAM, "ps_3_0");
			pixel_shader->setExtra(pixel_shader_name);
		}
		pass->setFragmentProgram(pixel_shader_name);

		if (pass->hasFragmentProgram()) {
			Ogre::GpuProgramParametersSharedPtr fp_parameters = pass->getFragmentProgramParameters();
			vector<string> shader_parameter_filenames=pixel_shader->getShaderParameterFilenames();

			fp_parameters->setTransposeMatrices(true);

			for (size_t i=0; i < shader_parameter_filenames.size(); i++) {
				LibGens::ShaderParams *shader_params=shader_library->getPixelShaderParams(shader_parameter_filenames[i]);

				if (shader_params) {
					setShaderParameters(pass, fp_parameters, material, shader_params);
				}
			}
		}
	}
	else {
		pass->setVertexProgram("SysDummyVS");
		pass->setFragmentProgram("SysDummyPS");
	}
}


void EditorMaterialConverter::setShaderParameters(Ogre::Pass *pass, Ogre::GpuProgramParametersSharedPtr program_params, LibGens::Material *material, LibGens::ShaderParams *shader_params) {
	vector<string> texture_units_used;
	texture_units_used.clear();

	for (size_t slot=0; slot<5; slot++) {
		vector<LibGens::ShaderParam *> shader_param_list=shader_params->getParameterList(slot);

		for (size_t param=0; param<shader_param_list.size(); param++) {
			LibGens::ShaderParam *shader_parameter = shader_param_list[param];

			unsigned char index = shader_parameter->getIndex();
			unsigned char size = shader_parameter->getSize();

			if (slot == 0) {
				LibGens::Parameter *material_parameter = material->getParameterByName(shader_parameter->getName());

				if (material_parameter) {
					LibGens::Color color = material_parameter->getColor();
					
					if (shader_parameter->getName() == "diffuse")  color.a = 1.0;
					if (shader_parameter->getName() == "specular") color.a = 1.0;
					if (shader_parameter->getName() == "ambient")  color.a = 1.0;

					program_params->setConstant((size_t)index, Ogre::Vector4(color.r, color.g, color.b, color.a));
					continue;
				}

				bool found_in_list = false;
				string shader_parameter_name = shader_parameter->getName();

				for (list<DefaultShaderParameter *>::iterator it = default_shader_parameters.parameters.begin(); it != default_shader_parameters.parameters.end(); it++) {
					if ((*it)->name == shader_parameter_name) {
						program_params->setConstant((size_t)index, Ogre::Vector4((*it)->r, (*it)->g, (*it)->b, (*it)->a));
						found_in_list = true;
						break;
					}
				}

				if (found_in_list) {
					continue;
				}
				
				if (shader_parameter->getName() == "g_MtxProjection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_PROJECTION_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxInvProjection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_INVERSE_PROJECTION_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxView") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_VIEW_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxInvView") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_INVERSE_VIEW_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxWorld") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_WORLD_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxWorldIT") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxPrevView") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_VIEW_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxPrevWorld") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_WORLD_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxLightViewProjection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxVerticalLightViewProjection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX);
				}
				else if (shader_parameter->getName() == "g_MtxBillboardY") {
					// Incomplete
				}
				else if (shader_parameter->getName() == "g_MtxPalette") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY_3x4);
				}
				else if (shader_parameter->getName() == "g_MtxPrevPalette") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY_3x4);
				}
				else if (shader_parameter->getName() == "g_MtxNodeInit") {
					program_params->setConstant((size_t)index, Ogre::Matrix4::IDENTITY);
				}
				else if (shader_parameter->getName() == "g_EyePosition") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_CAMERA_POSITION);
				}
				else if (shader_parameter->getName() == "g_EyeDirection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_VIEW_DIRECTION);
				}
				else if (shader_parameter->getName() == "g_ViewportSize") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_VIEWPORT_SIZE);
				}
				else if (shader_parameter->getName() == "g_CameraNearFarAspect") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgAmbientColor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgGroundColor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgSkyColor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgPowerGlossLevel") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgEmissionPower") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgGlobalLight_Direction") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 0);
				}
				else if (shader_parameter->getName() == "mrgGlobalLight_Direction_View") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION_VIEW_SPACE, 0);
				}
				else if (shader_parameter->getName() == "mrgGlobalLight_Diffuse") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 0);
				}
				else if (shader_parameter->getName() == "mrgGlobalLight_Specular") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR, 0);
				}
				else if (shader_parameter->getName() == "mrgLocallightIndexArray") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgLocalLight0_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 1);
				}
				else if (shader_parameter->getName() == "mrgLocalLight0_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 1);
				}
				else if (shader_parameter->getName() == "mrgLocalLight0_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 1);
				}
				else if (shader_parameter->getName() == "mrgLocalLight0_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 1);
				}
				else if (shader_parameter->getName() == "mrgLocalLight1_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 2);
				}
				else if (shader_parameter->getName() == "mrgLocalLight1_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 2);
				}
				else if (shader_parameter->getName() == "mrgLocalLight1_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 2);
				}
				else if (shader_parameter->getName() == "mrgLocalLight1_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 2);
				}
				else if (shader_parameter->getName() == "mrgLocalLight2_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 3);
				}
				else if (shader_parameter->getName() == "mrgLocalLight2_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 3);
				}
				else if (shader_parameter->getName() == "mrgLocalLight2_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 3);
				}
				else if (shader_parameter->getName() == "mrgLocalLight2_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 3);
				}
				else if (shader_parameter->getName() == "mrgLocalLight3_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 4);
				}
				else if (shader_parameter->getName() == "mrgLocalLight3_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 4);
				}
				else if (shader_parameter->getName() == "mrgLocalLight3_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 4);
				}
				else if (shader_parameter->getName() == "mrgLocalLight3_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 4);
				}
				else if (shader_parameter->getName() == "mrgLocalLight4_Position") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 5);
				}
				else if (shader_parameter->getName() == "mrgLocalLight4_Color") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR, 5);
				}
				else if (shader_parameter->getName() == "mrgLocalLight4_Range") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 5);
				}
				else if (shader_parameter->getName() == "mrgLocalLight4_Attribute") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_ATTENUATION, 5);
				}
				else if (shader_parameter->getName() == "mrgEyeLight_Diffuse") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 1));
				}
				else if (shader_parameter->getName() == "mrgEyeLight_Specular") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0.1, 0.1, 0.1, 1));
				}
				else if (shader_parameter->getName() == "mrgEyeLight_Range") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 0, 40));
				}
				else if (shader_parameter->getName() == "mrgEyeLight_Attribute") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 2));
				}
				else if (shader_parameter->getName() == "mrgLuminanceRange") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgInShadowScale") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgInShadowScaleLF") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_ShadowMapParams") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgVsmEpsilon") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgColourCompressFactor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_BackGroundScale") {
					float background_scale = 1.0f;
					program_params->setConstant((size_t)index, Ogre::Vector4(background_scale));
				}
				else if (shader_parameter->getName() == "g_GIModeParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_OffsetParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_WaterParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_IceParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_GI0Scale") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "g_GI1Scale") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "g_MotionBlur_AlphaRef_VelocityLimit_VelocityCutoff_BlurMagnitude") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgDebugDistortionParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgEdgeEmissionParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_DebugValue") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgGIAtlasParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgTexcoordIndex") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgTexcoordOffset") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgFresnelParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgMorphWeight") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "mrgZOffsetRate") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_IndexCount") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_TransColorMask") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_ChaosWaveParamEx") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_ChaosWaveParamY") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_ChaosWaveParamXZ") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_ChaosWaveParamXY") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_ChaosWaveParamZX") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_IgnoreLightParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_LightScattering_Ray_Mie_Ray2_Mie2") {
					LibGens::Color lsrm(0.291, 0.96, 1, 1);
					program_params->setConstant((size_t)index, Ogre::Vector4(lsrm.r, lsrm.g, lsrm.r / 16.587812802827, lsrm.g / 12.672096307931));
				}
				else if (shader_parameter->getName() == "g_LightScattering_ConstG_FogDensity") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0.0, 0.0, 0.0, 0.0));
				}
				else if (shader_parameter->getName() == "g_LightScatteringFarNearScale") {
					LibGens::Color lsfn(3200, 380, 1.2, 114);
					program_params->setConstant((size_t)index, Ogre::Vector4(1.0/lsfn.r, lsfn.g, lsfn.b, lsfn.a));
				}
				else if (shader_parameter->getName() == "g_LightScatteringColor") {
					LibGens::Color lsc(0.11, 0.35, 0.760001, 1);
					program_params->setConstant((size_t)index, Ogre::Vector4(lsc.r, lsc.g, lsc.b, 1));
				}
				else if (shader_parameter->getName() == "g_LightScatteringMode") {
					program_params->setConstant((size_t)index, Ogre::Vector4(4, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "g_VerticalLightDirection") {
					program_params->setAutoConstant((size_t)index, Ogre::GpuProgramParameters::ACT_LIGHT_POSITION, 0);
				}
				else if (shader_parameter->getName() == "g_aLightField") {
					float lightfield_cube[24];
					for (size_t i=0; i<24; i++) {
						lightfield_cube[i] = 0.9;
					}
					program_params->setConstant((size_t)index, lightfield_cube, 6);
				}
				else if (shader_parameter->getName() == "g_TimeParam") {
					program_params->setAutoConstantReal((size_t)index, Ogre::GpuProgramParameters::ACT_TIME, 1.0f);
				}
				else if (shader_parameter->getName() == "diffuse") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "ambient") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "specular") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "emissive") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "opacity_reflection_refraction_spectype") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 0, 1, 3));
				}
				else if (shader_parameter->getName() == "power_gloss_level") {
					program_params->setConstant((size_t)index, Ogre::Vector4(50, 0.3, 0.19, 0));
				}
				else if (shader_parameter->getName() == "g_SonicSkinFalloffParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0.15, 2, 3, 0));
				}
				else if (shader_parameter->getName() == "g_SkyParam") {
					float sky_follow_y_ratio = 1.0f;
					program_params->setConstant((size_t)index, Ogre::Vector4(1, sky_follow_y_ratio, 1, 1));
				}
				else if (shader_parameter->getName() == "g_ViewZAlphaFade") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "g_ForceAlphaColor") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "g_ChrPlayableMenuParam") {
					program_params->setConstant((size_t)index, Ogre::Vector4(1, 1, 1, 1));
				}
				else if (shader_parameter->getName() == "Fresnel") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "Luminance") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else if (shader_parameter->getName() == "Emission") {
					program_params->setConstant((size_t)index, Ogre::Vector4(0, 0, 0, 0));
				}
				else {
					MsgBox(("Unhandled constant/variable float4 " + shader_parameter->getName() + " with index " + ToString((int)index) + " on the Shader " + ToString(material->getShader()) + ". Handle it!").c_str());
				}
			}
			else if (slot == 2) {
				// Booleans
			}
			else if (slot == 3) {
				if ((size_t)index > 15) index = ((size_t) (index) / 17);
				string texture_unit = shader_parameter->getName();

				size_t texture_unit_used_count=0;
				for (size_t i=0; i<texture_units_used.size(); i++) {
					if (texture_units_used[i] == texture_unit) {
						texture_unit_used_count++;
					}
				}

				LibGens::Texture *material_texture = material->getTextureByUnit(texture_unit, texture_unit_used_count);
				if (material_texture) {
					pass->getTextureUnitState((size_t)index)->setTextureName(material_texture->getName()+LIBGENS_TEXTURE_FILE_EXTENSION);
					texture_units_used.push_back(texture_unit);
					continue;
				}


				if (shader_parameter->getName() == "TerrainDiffusemapMask") {
				}
				else if (shader_parameter->getName() == "GI") {
					pass->getTextureUnitState((size_t)index)->setTextureName("white.dds");
				}
				else if (shader_parameter->getName() == "Framebuffer") {
					//pass->getTextureUnitState((size_t)index)->setTextureName("ColorTex");
				}
				else if (shader_parameter->getName() == "Depth") {
					//pass->getTextureUnitState((size_t)index)->setTextureName("DepthTex");
					pass->getTextureUnitState((size_t)index)->setTextureName("white.dds");
				}
				else if (shader_parameter->getName() == "ShadowMap") {
					pass->getTextureUnitState((size_t)index)->setTextureName("white.dds");
				}
				else if (shader_parameter->getName() == "VerticalShadowMap") {
					pass->getTextureUnitState((size_t)index)->setTextureName("white.dds");
				}
				else if (shader_parameter->getName() == "ShadowMapJitter") {
				}
				else if (shader_parameter->getName() == "ReflectionMap") {
				}
				else if (shader_parameter->getName() == "ReflectionMap2") {
				}
				else if (shader_parameter->getName() == "INDEXEDLIGHTMAP") {
				}
				else if (shader_parameter->getName() == "PamNpcEye") {
				}
				else if (shader_parameter->getName() == "diffuse") {
				}
				else if (shader_parameter->getName() == "specular") {
				}
				else if (shader_parameter->getName() == "reflection") {
				}
				else if (shader_parameter->getName() == "normal") {
				}
				else if (shader_parameter->getName() == "displacement") {
				}
				else if (shader_parameter->getName() == "gloss") {
				}
				else if (shader_parameter->getName() == "opacity") {
				}
				else if (shader_parameter->getName() == "dlscatter") {
				}
				else {
					MsgBox(("Unhandled constant/variable sampler " + shader_parameter->getName() + " with index " + ToString((int)index) + " on the Shader " + ToString(material->getShader()) + ". Handle it!").c_str());
				}
			}
			else {
				MsgBox(("Unhandled slot " + ToString(slot) + ". Handle it!").c_str());
			}
		}
	}
}
