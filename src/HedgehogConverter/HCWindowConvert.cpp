//=========================================================================
//	  Copyright (c) 2016 SonicGLvl
//
//    This file is part of SonicGLvl, a community-created free level editor 
//    for the PC version of Sonic Generations.
//
//    SonicGLvl is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    SonicGLvl is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//    
//
//    Read AUTHORS.txt, LICENSE.txt and COPYRIGHT.txt for more details.
//=========================================================================

#include "HCWindow.h"
#include "LibGens.h"
#include <QFileInfo>
#include <QDir>
#include <QTemporaryDir>
#include <QProcess>
#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/config.h"
#include "Model.h"
#include "Mesh.h"
#include "Submesh.h"
#include "TerrainInstance.h"
#include "Vertex.h"
#include "VertexFormat.h"
#include "PAC.h"
#include "Material.h"
#include "Parameter.h"
#include "Texture.h"
#include "Tags.h"
#include "AR.h"
#include "Terrain.h"
#include "TerrainBlock.h"
#include "Light.h"
#include "HCMaterialDialog.h"

const int HCWindow::BaseUnassignedGroupIndex = 0x800000;

QString HCWindow::temporaryDirTemplate() {
	return converter_settings.terrain_output_path + "/HedgehogConverter-temp-XXXXXX";
}

bool HCWindow::convert() {
	QMap<string, bool> overwrite_materials;
	HCMaterialDialog dialog(&overwrite_materials, this);

	const QString EmbedTexturePrefix = "embed_";
	QStringList model_source_paths = converter_settings.model_source_paths;
	QString texture_source_path = converter_settings.texture_source_path;
	QString terrain_output_path = converter_settings.terrain_output_path;

	QTemporaryDir dir(temporaryDirTemplate());
	QTemporaryDir temp_texture_dir(temporaryDirTemplate());
    if (!dir.isValid()) {
		logProgress(ProgressFatal, "Cannot create temporal directory on System!");
		return false;
	}

	if (!temp_texture_dir.isValid()) {
		logProgress(ProgressFatal, "Cannot create temporal texture directory on System!");
		return false;
	}

	QString temp_path = dir.path();
	QString temp_texture_path = temp_texture_dir.path();
	QDir temp_dir(temp_path);
	QStringList texture_search_paths;
	texture_search_paths.append(temp_texture_path);

	//*************************
	//  Verifications
	//*************************
	logProgress(ProgressNormal, "Writing all temporary files to " + temp_path + ".");
	logProgress(ProgressNormal, "Writing all temporary textures to " + temp_texture_path + ".");

	if (model_source_paths.isEmpty()) {
		logProgress(ProgressFatal, "Source model paths can't be empty!");
		return false;
	}

	foreach(QString model_source_path, model_source_paths) {
		if (!QFileInfo(model_source_path).exists()) {
			logProgress(ProgressFatal, "Source model file " + model_source_path + " doesn't exist! Verify if it's there or if it's open by another program.");
			return false;
		}
	}

	if (texture_source_path.isEmpty()) {
		texture_source_path = QFileInfo(model_source_paths.first()).absolutePath();
		logProgress(ProgressWarning, "Source texture path is empty! Defaulting to first model's directory for searching textures.");
	}

	if (terrain_output_path.isEmpty()) {
		logProgress(ProgressFatal, "Output path can't be empty!");
		return false;
	}

	QString output_name = QDir(terrain_output_path).dirName();
	texture_search_paths.append(texture_source_path);

	//**************************************
	//  Extract and delete existing files
	//**************************************
	if (converter_settings.merge_existing) {
		logProgress(ProgressNormal, "Merging existing files...");

		if (converter_settings.game_engine == Generations) {
			QString existing_ar_file = terrain_output_path + "/" + output_name + ".ar.00";
			if (QFileInfo(existing_ar_file).exists()) {
				logProgress(ProgressNormal, "Found " + existing_ar_file + ". Extracting files into temporary directory " + temp_path);
				LibGens::ArPack ar_pack(existing_ar_file.toStdString());
				ar_pack.extract(temp_path.toStdString() + "/");
			}
			else {
				logProgress(ProgressWarning, "No existing file detected named " + existing_ar_file + " to merge with. Creating files from scratch.");
			}
		}
		else if (converter_settings.game_engine == Unleashed) {
			logProgress(ProgressFatal, "Mer-ng existing files is not implemented for Unleashed engine yet!");
			return false;
		}
		else if (converter_settings.game_engine == LostWorld) {
			QString existing_pac_file = terrain_output_path + "/" + output_name + "_trr_cmn.pac";
			if (QFileInfo(existing_pac_file).exists()) {
				logProgress(ProgressNormal, "Found " + existing_pac_file + ". Extracting files into temporary directory " + temp_path);

				LibGens::PacSet pac_set(existing_pac_file.toStdString());
				pac_set.extract(temp_path.toStdString() + "/");

				// Delete all terrain models and instances, as well as GI textures with their same base names from the temporary directory.
				logProgress(ProgressNormal, "Deleting previous terrain files from temporary directory...");

				QStringList entry_list = temp_dir.entryList(QStringList() << "*.terrain-instanceinfo" << "*.terrain-model");
				foreach(QString entry, entry_list) {
					QString remove_filename = temp_path + "/" + entry;
					bool removed_file = QFile::remove(remove_filename);
					if (removed_file) {
						logProgress(ProgressNormal, "Removed " + remove_filename + " before merging existing files.");
					}
					else {
						logProgress(ProgressWarning, "Couldn't remove " + remove_filename + " before merging existing files. Skipping.");
					}
				}

				logProgress(ProgressNormal, "Cleanup of previous files complete.");
			}
			else {
				logProgress(ProgressWarning, "No existing file detected named " + existing_pac_file + " to merge with. Creating files from scratch.");
			}
		}
		else {
			logProgress(ProgressFatal, "No valid game engine selected!");
			return false;
		}
	}

	//*************************
	//  Global Transform
	//*************************

	LibGens::Matrix4 global_transform;
	LibGens::Vector3 position(converter_settings.position_x, converter_settings.position_y, converter_settings.position_z);
	LibGens::Vector3 scale(converter_settings.scale_x * 0.01, converter_settings.scale_y * 0.01, converter_settings.scale_z * 0.01);
	LibGens::Quaternion rotation_x, rotation_y, rotation_z;
	rotation_x.fromAngleAxis(converter_settings.rotation_x * LIBGENS_MATH_RAD_TO_DEGREE, LibGens::Vector3(1.0f, 0.0f, 0.0f));
	rotation_y.fromAngleAxis(converter_settings.rotation_y * LIBGENS_MATH_RAD_TO_DEGREE, LibGens::Vector3(0.0f, 1.0f, 0.0f));
	rotation_z.fromAngleAxis(converter_settings.rotation_z * LIBGENS_MATH_RAD_TO_DEGREE, LibGens::Vector3(0.0f, 0.0f, 1.0f));
	LibGens::Quaternion orientation = rotation_x * rotation_y * rotation_z;
	global_transform.makeTransform(position, scale, orientation);

	logProgress(ProgressNormal, "Using Global Transform Matrix...");
	for (int x = 0; x < 4; x++) {
		logProgress(ProgressNormal, QString(" [%1, %2, %3, %4]").arg(global_transform[x][0]).arg(global_transform[x][1]).arg(global_transform[x][2]).arg(global_transform[x][3]));
	}

	//*************************
	//  Load Assimp Scene
	//*************************
	SceneData scene_data;
	QStringList textures_to_copy;
	QStringList textures_to_convert;

	// Load all materials from temporary directory into memory
	QStringList material_entry_list = temp_dir.entryList(QStringList() << "*.material");
	foreach(QString material_filename, material_entry_list) {
		LibGens::Material *load_material = new LibGens::Material(material_filename.toStdString());
		scene_data.materials.push_back(load_material);
	}

	foreach(QString model_source_path, model_source_paths) {
		logProgress(ProgressNormal, "Assimp importer reading model " + model_source_path + " ...");

		Assimp::Importer importer;
		importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, 0x8000);
		importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, 0x80000);

		const aiScene *scene = importer.ReadFile(model_source_path.toStdString(), aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_JoinIdenticalVertices | 
																				  aiProcess_CalcTangentSpace | aiProcess_FindInstances | aiProcess_SplitLargeMeshes);
		if (!scene) {
			logProgress(ProgressFatal, QString("Assimp failed to open %1: %2").arg(model_source_path).arg(importer.GetErrorString()));
			return false;
		}

		logProgress(ProgressNormal, "Assimp importer loaded scene sucessfully.");
		logProgress(ProgressNormal, "aiScene:");
		logProgress(ProgressNormal, QString("* Animations: %1").arg(scene->mNumAnimations));
		logProgress(ProgressNormal, QString("* Cameras: %1").arg(scene->mNumCameras));
		logProgress(ProgressNormal, QString("* Lights: %1").arg(scene->mNumLights));
		logProgress(ProgressNormal, QString("* Materials: %1").arg(scene->mNumMaterials));
		logProgress(ProgressNormal, QString("* Meshes: %1").arg(scene->mNumMeshes));
		logProgress(ProgressNormal, QString("* Textures: %1").arg(scene->mNumTextures));
		logProgress(ProgressNormal, QString("* Node Tree:"));
		logNodeTree(scene->mRootNode, "**");

		int lights_count = scene->mNumLights;
		if (lights_count && converter_settings.convert_lights) {
			LibGens::LightList light_list;

			for (int l = 0; l < lights_count; l++) {
				aiLight *scene_light = scene->mLights[l];
				aiVector3D position = scene_light->mPosition;
				aiVector3D direction = scene_light->mDirection;
				aiColor3D color = scene_light->mColorDiffuse;
				float attenuation_constant = scene_light->mAttenuationConstant;
				float attenuation_linear = scene_light->mAttenuationLinear;
				float attenuation_quadratic = scene_light->mAttenuationQuadratic;

				// Detected color in 0-100 scale.
				if ((color.r > 10.0) || (color.g > 10.0)|| (color.b > 10.0)) {
					color.r *= 0.01f; color.g *= 0.01f; color.b *= 0.01f;
					logProgress(ProgressNormal, QString("Detected Light %1 color in the 0-100 scale. Scaling down to 0-1.").arg(scene_light->mName.C_Str()));
				}

				logProgress(ProgressNormal, QString("Light %1 Position %2 %3 %4.").arg(scene_light->mName.C_Str()).arg(position.x).arg(position.y).arg(position.z));
				logProgress(ProgressNormal, QString("Light %1 Direction %2 %3 %4.").arg(scene_light->mName.C_Str()).arg(direction.x).arg(direction.y).arg(direction.z));
				logProgress(ProgressNormal, QString("Light %1 Color %2 %3 %4.").arg(scene_light->mName.C_Str()).arg(color.r).arg(color.g).arg(color.b));
				logProgress(ProgressNormal, QString("Light %1 Attenuations Constant %2 Linear %3 Quadratic %4.").arg(scene_light->mName.C_Str()).arg(attenuation_constant).arg(attenuation_linear).arg(attenuation_quadratic));

				// Invalid values from format cases
				if ((position.Length() == 0.0f) || (direction.Length() == 0.0f) || ((direction.x == direction.y == 0.0f) && direction.z == -1.0f)) {
					aiMatrix4x4 scene_global_transform;
					aiMatrix4x4 light_transform;
					for (int i = 0; i < 4; i++)
						for (int j = 0; j < 4; j++)
							scene_global_transform[i][j] = global_transform[i][j];

					if (findNodeTransform(scene->mRootNode, scene_global_transform, scene_light->mName, &light_transform)) {
						logProgress(ProgressNormal, QString("Found a light transform for %1.").arg(scene_light->mName.C_Str()));
						aiVector3D sca;
						aiQuaternion ori;
						light_transform.Decompose(sca, ori, position);
						direction = ori.GetMatrix() * aiVector3D(0.0f, 1.0f, 0.0f);

						logProgress(ProgressNormal, QString("Light %1 Transform's Position: %2 %3 %4.").arg(scene_light->mName.C_Str()).arg(position.x).arg(position.y).arg(position.z));
						logProgress(ProgressNormal, QString("Light %1 Transform's Orientation: %2 %3 %4 %5.").arg(scene_light->mName.C_Str()).arg(ori.w).arg(ori.x).arg(ori.y).arg(ori.z));
						logProgress(ProgressNormal, QString("Light %1 Transform's Direction: %2 %3 %4.").arg(scene_light->mName.C_Str()).arg(direction.x).arg(direction.y).arg(direction.z));
					}
					else {
						logProgress(ProgressError, QString("Couldn't find a valid light transform for %1.").arg(scene_light->mName.C_Str()));
						continue;
					}
				}

				if (scene_light->mType == aiLightSource_DIRECTIONAL) {
					if (direction.Length() > 0.0f) {
						LibGens::Light *light = new LibGens::Light();
						light->setType(LIBGENS_LIGHT_TYPE_DIRECTIONAL);
						light->setName(scene_light->mName.C_Str());
						light->setPosition(LibGens::Vector3(direction.x, direction.y, direction.z));
						light->setColor(LibGens::Vector3(color.r, color.g, color.b));
						light_list.addLight(light);
						light->save(temp_path.toStdString() + "/" + light->getName() + LIBGENS_LIGHT_EXTENSION);
						logProgress(ProgressNormal, QString("Saved and added diffuse directional light %1.").arg(scene_light->mName.C_Str()));
					}
					else {
						logProgress(ProgressError, QString("Direction light %1's direction is not valid.").arg(scene_light->mName.C_Str()));
					}
				}
				else if (scene_light->mType == aiLightSource_POINT) {
					logProgress(ProgressError, QString("Omni light %1 isn't supported yet").arg(scene_light->mName.C_Str()));
				}
				else {
					logProgress(ProgressError, QString("The light type %1 of %2 isn't supported.").arg(scene_light->mType).arg(scene_light->mName.C_Str()));
				}
			}

			if (light_list.getLightCount()) {
				light_list.save(temp_path.toStdString() + "/" + LIBGENS_LIGHT_LIST_FILENAME);
				logProgress(ProgressNormal, QString("Saved light list with %1 lights.").arg(light_list.getLightCount()));
			}
		}

		//**********************************************************
		//  Dump embedded textures to another temporary directory
		//**********************************************************
		int embedded_textures_count = scene->mNumTextures;
		if (embedded_textures_count) {
			for (int t = 0; t < embedded_textures_count; t++) {
				aiTexture *texture = scene->mTextures[t];
				logProgress(ProgressNormal, QString("Texture #%1: %2x%3 Pixels (%4 Hint)").arg(t).arg(texture->mWidth).arg(texture->mHeight).arg(texture->achFormatHint));

				if (texture->mWidth && !texture->mHeight) {
					QString texture_filename = QString("%1/%2%3.%4").arg(temp_texture_path).arg(EmbedTexturePrefix).arg(t).arg(texture->achFormatHint);
					logProgress(ProgressNormal, QString("Texture #%1 is compressed. Saving to %2....").arg(t).arg(texture_filename));

					LibGens::File file(texture_filename.toStdString(), LIBGENS_FILE_WRITE_BINARY);
					if (file.valid()) {
						file.write(texture->pcData, texture->mWidth);
						file.close();

						logProgress(ProgressNormal, QString("Saved texture #%1 to %2.").arg(t).arg(texture_filename));
					}
					else {
						logProgress(ProgressError, QString("Couldn't save texture #%1 to %2. Directory is write-protected or there's not enough space.").arg(t).arg(texture_filename));
					}
				}
				else if (texture->mWidth && texture->mHeight) {
					logProgress(ProgressError, QString("Texture dumping for ARGB8888 is not implemented yet! Texture #%1 can't be converted yet.").arg(t));
				}
				else {
					logProgress(ProgressWarning, QString("Invalid size for texture #%1.").arg(t));
				}
			}
		}


		//***********************************************************************************************
		//  Convert Materials
		//
		// - Convert materials first to get the real model names.
		// - Also convert materials first to know which mesh slot submeshes need to be assigned to.
		//
		//***********************************************************************************************

		int materials_count = scene->mNumMaterials;
		if (converter_settings.generate_materials) {
			overwrite_materials.clear();
			QMap<string, LibGens::Material *> existing_materials;

			// Build a list of material names and ask to override or not.
			for (int m = 0; m < materials_count; m++) {
				aiMaterial *src_material = scene->mMaterials[m];
				aiString src_material_name;
				src_material->Get(AI_MATKEY_NAME, src_material_name);
				string material_name = src_material_name.C_Str();
				LibGens::Tags tags(material_name);
				if (converter_settings.remove_material_tags) {
					material_name = tags.getName();
				}

				// Check if material with that name already exists
				LibGens::Material *existing_material = NULL;
				for (list<LibGens::Material *>::iterator it = scene_data.materials.begin(); it != scene_data.materials.end(); it++) {
					if ((*it)->getName() == material_name) {
						existing_material = *it;
						break;
					}
				}

				if (existing_material) {
					if (!existing_material->hasExtra()) {
						overwrite_materials[material_name] = false;
					}

					existing_materials[material_name] = existing_material;
				}
			}

			if (!overwrite_materials.isEmpty()) {
				dialog.setupOverrideMap();
				dialog.setFBXName(model_source_path);
				beep();
				dialog.exec();
				dialog.updateOverrideMap();
			}

			for (int m = 0; m < materials_count; m++) {
				aiMaterial *src_material = scene->mMaterials[m];
				aiString src_material_name;
				src_material->Get(AI_MATKEY_NAME, src_material_name);

				string material_name = src_material_name.C_Str();
				LibGens::Tags tags(material_name);
				if (converter_settings.remove_material_tags) {
					logProgress(ProgressNormal, QString("Detected material #%1 %2 tags with base name %3 for material %4.").arg(m).arg(tags.getTagCount()).arg(tags.getName().c_str()).arg(src_material_name.C_Str()));
					material_name = tags.getName();
				}

				LibGens::Material *existing_material = existing_materials[material_name];
				if (existing_material) {
					if (overwrite_materials[material_name]) {
						if (!existing_material->hasExtra()) {
							logProgress(ProgressWarning, "Overwriting existing material " + QString(material_name.c_str()) + ".");

							delete existing_material;
							scene_data.materials.remove(existing_material);
							existing_materials[material_name] = NULL;
						}
						else {
							logProgress(ProgressNormal, "Found existing material " + QString(material_name.c_str()) + ".");
							continue;
						}
					}
					else {
						logProgress(ProgressNormal, "Not overwriting material " + QString(material_name.c_str()) + ".");
						continue;
					}
				}

				logProgress(ProgressNormal, "Converting material " + QString(material_name.c_str()) + "...");
				LibGens::Material *material = new LibGens::Material();
				material->setExtra("From Source Models");
				material->setName(material_name);
				material->setShader(tags.getTagValue(LibGens::Tag::KeyMaterialShader, 0, "Common_d"));
				material->setNoCulling(!tags.getTagValueBool(LibGens::Tag::KeyMaterialCulling, 0, true));
				material->setColorBlend(tags.getTagValueBool(LibGens::Tag::KeyMaterialAdd));
				material->setLayer(tags.getTagValue(LibGens::Tag::KeyMaterialLayer, 0, LibGens::Material::LayerOpaq));

				// Default parameters for material
				LibGens::Parameter *param = new LibGens::Parameter();
				param->name = "diffuse";
				param->color = LibGens::Color(1, 1, 1, 0);
				material->addParameter(param);

				param = new LibGens::Parameter();
				param->name = "ambient";
				param->color = LibGens::Color(1, 1, 1, 0);
				material->addParameter(param);

				param = new LibGens::Parameter();
				param->name = "specular";
				param->color = LibGens::Color(0.9, 0.9, 0.9, 0);
				material->addParameter(param);

				param = new LibGens::Parameter();
				param->name = "emissive";
				param->color = LibGens::Color(0, 0, 0, 0);
				material->addParameter(param);

				param = new LibGens::Parameter();
				param->name = "power_gloss_level";
				param->color = LibGens::Color(50, 0.01, 0, 0);
				material->addParameter(param);

				param = new LibGens::Parameter();
				param->name = "opacity_reflection_refraction_spectype";
				param->color = LibGens::Color(1, 0, 1, 0);
				material->addParameter(param);

				// Detect textures from assimp
				for (int slot = 0; slot < 11; slot++) {
					QString texture_unit = QString();
					aiTextureType texture_type = aiTextureType_NONE;

					switch (slot) {
					case 0:
						texture_unit = "diffuse";
						texture_type = aiTextureType_DIFFUSE;
						break;
					case 1:
						texture_unit = "opacity";
						texture_type = aiTextureType_OPACITY;
						break;
					case 2:
						texture_unit = "normal";
						texture_type = aiTextureType_NORMALS;
						break;
					case 3:
						texture_unit = "reflection";
						texture_type = aiTextureType_REFLECTION;
						break;
					case 4:
						texture_unit = "specular";
						texture_type = aiTextureType_SPECULAR;
						break;
					case 5:
						texture_unit = "gloss";
						texture_type = aiTextureType_SHININESS;
						break;
					case 6:
						texture_unit = "emissive";
						texture_type = aiTextureType_EMISSIVE;
						break;
					case 7:
						texture_unit = "ambient";
						texture_type = aiTextureType_AMBIENT;
						break;
					case 8:
						texture_unit = "displacement";
						texture_type = aiTextureType_DISPLACEMENT;
						break;
					case 9:
						texture_unit = "unknown";
						texture_type = aiTextureType_HEIGHT;
						break;
					case 10:
						texture_unit = "unknown";
						texture_type = aiTextureType_UNKNOWN;
						break;
					}

					if (!texture_unit.isEmpty()) {
						int texture_count = src_material->GetTextureCount(texture_type);
						for (int t = 0; t < texture_count; t++) {
							aiString src_texture_path;
							src_material->GetTexture(texture_type, t, &src_texture_path);

							logProgress(ProgressNormal, QString("Detected texture #%1 for unit %2 (Assimp Type: %3).").arg(t).arg(src_texture_path.C_Str()).arg((int)texture_type));

							if (src_texture_path.length) {
								QString texture_path = src_texture_path.C_Str();

								// Embedded texture naming if the first symbol is an asterisk. That's how assimp imports it.
								if (!texture_path.isEmpty() && (texture_path[0] == '*')) {
									texture_path.remove(0, 1);
									unsigned int texture_index = texture_path.toInt();
									texture_path = EmbedTexturePrefix + texture_path + ".";

									logProgress(ProgressNormal, QString("Detected embedded texture with index #%1.").arg(texture_index));
									if ((texture_index >= 0) && (texture_index < scene->mNumTextures)) {
										if (scene->mTextures[texture_index]->mHeight) {
											logProgress(ProgressNormal, QString("Embedded texture #%1 is not compressed. Defaulting to dds.").arg(texture_index));
											texture_path += "dds";
										}
										else {
											QString extension = scene->mTextures[texture_index]->achFormatHint;
											logProgress(ProgressNormal, QString("Embedded texture #%1 is compressed. Extension is %2.").arg(texture_index).arg(extension));
											texture_path += extension;
										}

										logProgress(ProgressNormal, QString("Embedded texture #%1 renamed to %2.").arg(texture_index).arg(texture_path));
									}
									else {
										logProgress(ProgressError, QString("Embedded texture index #%1 is invalid. There's only %2 embedded textures on the scene.").arg(texture_index).arg(scene->mNumTextures));
									}
								}

								addTextureToMaterial(material, material_name.c_str(), texture_path, texture_unit, textures_to_copy, textures_to_convert);
							}
						}
					}
				}
			
				// After all assimp slots are detected, check for texture tags
				vector<LibGens::Tag> texture_tags = tags.getTagsByKey(LibGens::Tag::KeyMaterialTexture);
				for (size_t t = 0; t < texture_tags.size(); t++) {
					string texture_unit = texture_tags[t].getValue(0);
					string texture_name = texture_tags[t].getValue(1);
					if (texture_unit.size() && texture_name.size()) {
						addTextureToMaterial(material, material_name.c_str(), texture_name.c_str(), texture_unit.c_str(), textures_to_copy, textures_to_convert);
					}
				}

				// Add parameters from tags
				vector<LibGens::Tag> parameter_tags = tags.getTagsByKey(LibGens::Tag::KeyMaterialParameter);
				for (size_t t = 0; t < parameter_tags.size(); t++) {
					string parameter_name = parameter_tags[t].getValue(0);
					LibGens::Color color(parameter_tags[t].getValueFloat(1, 1.0f), 
										 parameter_tags[t].getValueFloat(2, 1.0f), 
										 parameter_tags[t].getValueFloat(3, 1.0f), 
										 parameter_tags[t].getValueFloat(4, 0.0f));

					if (parameter_name.size()) {
						material->setParameter(parameter_name, color);
					}
				}

				QString material_filename = QString("%1/%2.material").arg(temp_path).arg(material_name.c_str());
				material->save(material_filename.toStdString());
				logProgress(ProgressNormal, "Saved material to " + material_filename + ".");

				scene_data.materials.push_back(material);
			}
		}

		//***********************************************************************************************
		//  Convert Scene
		//
		// - Traverse node tree.
		// - Keep a list of the model indices used by each node to detect instances.
		// - The node represents either a complete model or an instance. Save to directory accordingly.
		//
		//***********************************************************************************************
		logProgress(ProgressNormal, "Converting scene node tree...");
		scene_data.model_map.clear();

		bool conversion_result = convertSceneNode(scene, scene->mRootNode, temp_path, scene_data, global_transform);
		if (!conversion_result) {
			logProgress(ProgressFatal, QString("Couldn't convert node tree properly. Check progress log for what errors to fix."));
			return false;
		}
	}

	
	if (converter_settings.copy_and_convert_textures) {
		//*************************
		//  Copy Textures
		//*************************
		textures_to_copy.removeDuplicates();

		foreach(QString texture_search_path, texture_search_paths) {
			foreach(QString texture_filename, textures_to_copy) {
				QString source_file = texture_search_path + "/" + QFileInfo(texture_filename).fileName();
				if (QFileInfo(source_file).exists()) {
					textures_to_copy.removeAll(texture_filename);

					QString output_file = temp_path + "/" + QFileInfo(texture_filename).fileName();

					QFile::remove(output_file);
					if (QFile::copy(source_file, output_file)) {
						logProgress(ProgressNormal, QString("Copied " + source_file + " to " + output_file));
					}
					else {
						logProgress(ProgressError, QString("Couldn't copy " + source_file + " to " + output_file));
					}
				}
			}
		}

		//*************************
		//  Convert Textures
		//*************************
		textures_to_convert.removeDuplicates();

		foreach(QString texture_search_path, texture_search_paths) {
			foreach(QString texture_filename, textures_to_convert) {
				QString source_file = texture_search_path + "/" + QFileInfo(texture_filename).fileName();
				QString output_file = temp_path + "/" + QFileInfo(texture_filename).baseName() + ".dds";

				if (QFileInfo(source_file).exists()) {
					textures_to_convert.removeAll(texture_filename);

					logProgress(ProgressNormal, QString("Calling NVDXT converter for " + source_file + " to " + output_file));

					const QString temp_texture = "temp.dds";
					QStringList arguments;
					arguments << "-file" << source_file << "-output" << temp_texture;

					QProcess conversion_process;
					conversion_process.start("nvdxt", arguments);
					conversion_process.waitForFinished();
					QString conversion_output = conversion_process.readAllStandardOutput();
					logProgress(ProgressNormal, QString("NVDXT Output: " + conversion_output));

					QFile::remove(output_file);
					if (QFile::copy(temp_texture, output_file)) {
						logProgress(ProgressNormal, QString("Converter NVDXT finished with " + source_file + " to " + output_file));
					}
					else {
						logProgress(ProgressWarning, QString("Couldn't copy NVDXT's result to " + output_file + ". Is this directory write-protected?"));
					}
				}
			}
		}

		foreach(QString texture_filename, textures_to_copy + textures_to_convert) {
			logProgress(ProgressError, QString("Couldn't find " + texture_filename + " in any of the texture paths."));
		}
	}

	//*************************
	//  Generate Groups
	//*************************
	QList<LibGens::TerrainGroup *> terrain_groups;
	if ((converter_settings.game_engine == Generations) || (converter_settings.game_engine == Unleashed)) {
		terrain_groups = convertTerrainGroups(scene_data);
	}


	//*************************
	//  Generate Terrain Files
	//*************************
	if ((converter_settings.game_engine == Generations) || (converter_settings.game_engine == Unleashed)) {
		if (terrain_groups.size()) {
			logProgress(ProgressNormal, QString("%1 terrain groups were generated.").arg(terrain_groups.size()));

			// Delete all terrain groups and terrain files inside.
			logProgress(ProgressNormal, "Deleting previous terrain files from temporary directory...");

			QStringList entry_list = temp_dir.entryList(QStringList() << "*.terrain" << "*.terrain-group" << "*.vt" << "*.tbst");
			foreach(QString entry, entry_list) {
				QString remove_filename = temp_path + "/" + entry;
				bool removed_file = QFile::remove(remove_filename);
				if (removed_file) {
					logProgress(ProgressNormal, "Removed " + remove_filename + " before merging existing files.");
				}
				else {
					logProgress(ProgressWarning, "Couldn't remove " + remove_filename + " before merging existing files. Skipping.");
				}
			}
			logProgress(ProgressNormal, "Cleanup of previous files complete.");

			int group_index = 0;
			LibGens::Terrain *terrain = new LibGens::Terrain();
			foreach(LibGens::TerrainGroup *group, terrain_groups) {
				group->setName(QString("tg-%1").arg(group_index, 4, 10, QChar('0')).toStdString());
				string group_filename = temp_path.toStdString() + "/" + group->getName() + ".terrain-group";
				group->save(group_filename);
				logProgress(ProgressNormal, QString("Saved terrain group to %1 and creating group info.").arg(group_filename.c_str()));

				unsigned int memory_size = 0;
				vector<LibGens::Model *> models = group->getModels();
				for (unsigned int i = 0; i < models.size(); i++) {
					memory_size += scene_data.model_size_map[models[i]->getName()];
				}
				logProgress(ProgressNormal, QString("Terrain group %1 has an estimated memory size of %2.").arg(group->getName().c_str()).arg(memory_size));

				LibGens::TerrainGroupInfo *group_info = new LibGens::TerrainGroupInfo(group);
				group_info->setFolderSize(memory_size);
				terrain->addGroupInfo(group_info);
				group_index++;
			}

			string terrain_filename = temp_path.toStdString() + "/terrain.terrain";
			terrain->save(terrain_filename);
			logProgress(ProgressNormal, QString("Saved terrain to %1.").arg(terrain_filename.c_str()));

			delete terrain;

			LibGens::TerrainBlock *terrain_block = new LibGens::TerrainBlock();
			group_index = 0;
			foreach(LibGens::TerrainGroup *group, terrain_groups) {
				vector<vector<LibGens::TerrainInstance *>> instances = group->getInstanceVectors();
				vector<LibGens::Vector3> centers = group->getInstanceCenters();
				vector<float> radiuses = group->getInstanceRadiuses();
				for (size_t i=0; i<instances.size(); i++) {
					LibGens::TerrainBlockInstance *block_instance = new LibGens::TerrainBlockInstance();
					block_instance->setCenter(centers[i]);
					block_instance->setRadius(radiuses[i]);
					block_instance->setIdentifierA(group_index);
					block_instance->setIdentifierB(i);
					block_instance->setType(1);
					terrain_block->addBlockInstance(block_instance);
				}

				group_index++;
			}

			string terrain_block_filename = temp_path.toStdString() + "/terrain-block.tbst";
			terrain_block->save(terrain_block_filename.c_str());
			logProgress(ProgressNormal, QString("Saved terrain block to %1.").arg(terrain_block_filename.c_str()));

			delete terrain_block;
		}
		else {
			logProgress(ProgressError, "No terrain groups were generated.");
		}
	}

	//***********************************
	//  Save Terrain Packs and resources
	//***********************************

	// Create output path
	QDir().mkpath(terrain_output_path);

	// Packing routines for each game engine
	if (converter_settings.game_engine == Generations) {
		logProgress(ProgressNormal, "Packing for Generations engine...");
		bool pack_result = packGenerations(terrain_groups, terrain_output_path, output_name, temp_path);
		return pack_result;
	}
	else if (converter_settings.game_engine == Unleashed) {
		logProgress(ProgressFatal, "Packing for Unleashed game engine not implemented yet!");
		return false;
	}
	else if (converter_settings.game_engine == LostWorld) {
		logProgress(ProgressNormal, "Packing for Lost World engine...");
		bool pack_result = packLostWorld(terrain_output_path, output_name, temp_path);
		return pack_result;
	}
	else {
		logProgress(ProgressFatal, "No valid game engine detected.");
	}

	//*************************
	//  Cleanup Memory
	//*************************

	foreach(LibGens::TerrainInstance *instance, scene_data.instances.values()) {
		delete instance;
	}

	foreach(LibGens::TerrainGroup *group, terrain_groups) {
		delete group;
	}

	return false;
}

void HCWindow::addTextureToMaterial(LibGens::Material *material, QString material_name, QString texture_path, QString texture_unit, QStringList &textures_to_copy, QStringList &textures_to_convert) {
	QString internal_name = QString("%1-%2").arg(material_name).arg(material->getTextureUnitsSize(), 4, 10, QChar('0'));
	QString texture_name = QFileInfo(texture_path).baseName();
					
	if (!texture_name.isEmpty()) {
		if (texture_path.contains(".dds", Qt::CaseInsensitive))
			textures_to_copy.append(texture_path);
		else
			textures_to_convert.append(texture_path);

		LibGens::Texture *texture = new LibGens::Texture();
		texture->setName(texture_name.toStdString());
		texture->setUnit(texture_unit.toStdString());
		texture->setInternalName(internal_name.toStdString());
		material->addTextureUnit(texture);

		logProgress(ProgressNormal, QString("Added %1 as %2 to %3 unit.").arg(texture_name).arg(internal_name).arg(texture_unit));
	}
}


bool HCWindow::convertSceneNode(const aiScene *scene, aiNode *node, QString path, SceneData &scene_data, LibGens::Matrix4 global_transform, int parent_group_id) {
	if (scene && node) {
		aiMatrix4x4 node_matrix = node->mTransformation;
		LibGens::Matrix4 instance_matrix;
		for (int x = 0; x < 4; x++)
			for (int y = 0; y < 4; y++)
				instance_matrix[x][y] = node_matrix.m[x][y];

		instance_matrix = global_transform * instance_matrix;

		LibGens::Tags model_tags(node->mName.C_Str());

		if ((parent_group_id == -1) && (converter_settings.use_model_groups)) {
			// Search tags to find out if this is a terrain group or not
			bool is_terrain_group = model_tags.getTagsByKey(LibGens::Tag::KeyTerrainGroup).size();
			if (is_terrain_group) {
				int index = model_tags.getTagValueInt(LibGens::Tag::KeyTerrainGroup, 0, -1);

				// A group without a manual ID.
				if (index == -1)
					parent_group_id = BaseUnassignedGroupIndex + scene_data.instances.keys().size();
				// A group with a manual ID.
				else
					parent_group_id = index;
			}
		}

		unsigned int meshes = node->mNumMeshes;
		if (meshes > 0) {
			logProgress(ProgressNormal, QString("Converting %1 (%2 Meshes)...").arg(node->mName.C_Str()).arg(node->mNumMeshes));

			// Verify on existing map if this list of indices already exists
			QList<int> mesh_indices;
			for (unsigned int m = 0; m < meshes; m++) {
				mesh_indices.append(node->mMeshes[m]);
			}
			qSort(mesh_indices);

			QString model_name = node->mName.C_Str();
			if (converter_settings.remove_model_tags)
				model_name = model_tags.getName().c_str();

			QString existing_model_name = "";
			foreach(QString model_name, scene_data.model_map.keys()) {
				if (mesh_indices == scene_data.model_map[model_name].used_meshes) {
					existing_model_name = model_name;
					break;
				}
			}

			// Create LibGens model and add the name to the model map
			if (existing_model_name.isEmpty()) {
				logProgress(ProgressNormal, QString("Detected %1 as the model for this instance but there's no model converted yet. Converting model...").arg(model_name));

				// Detect and rename if duplicate files are found
				int attempt = 0;
				QString model_base_name = model_name;
				QString model_filename = QString();
				do {
					model_name = model_base_name + (attempt == 0 ? "" : QString("_%1").arg(attempt));
					model_filename = path + "/" + model_name + ".terrain-model";
					attempt++;
				} while (QFileInfo(model_filename).exists());

				if (model_base_name != model_name) {
					logProgress(ProgressWarning, QString("Detected %1 as duplicate model name. Renamed to %2.").arg(model_base_name).arg(model_name));
				}

				LibGens::Model *model = new LibGens::Model();
				model->setName(model_name.toStdString());
				model->setTerrainMode(true);

				LibGens::Mesh *mesh = new LibGens::Mesh();
				bool set_layer_string = false;

				foreach(unsigned int mesh_index, mesh_indices) {
					logProgress(ProgressNormal, QString("Converting submesh with global index %1").arg(mesh_index));

					aiMesh *src_mesh = scene->mMeshes[mesh_index];
					unsigned int material_index = src_mesh->mMaterialIndex;
					aiMaterial *src_material = scene->mMaterials[material_index];
					aiString src_material_name;
					src_material->Get(AI_MATKEY_NAME, src_material_name);

					string material_name = src_material_name.C_Str();
					LibGens::Tags material_tags(material_name);
					if (converter_settings.remove_material_tags) {
						material_name = material_tags.getName();
					}

					int num_vertices = src_mesh->mNumVertices;
					int num_faces = src_mesh->mNumFaces;

					logProgress(ProgressNormal, QString("Converting submesh with material name %1, %2 vertices and %3 faces").arg(material_name.c_str()).arg(num_vertices).arg(num_faces));

					if (material_name.size() && num_vertices && num_faces) {
						LibGens::Submesh *submesh = new LibGens::Submesh();

						vector<LibGens::Vertex *> vertices;
						vector<LibGens::Polygon> polygons;

						int uv_channels = src_mesh->GetNumUVChannels();
						if (uv_channels > 4)
							uv_channels = 4;

						for (int v=0; v < num_vertices; v++) {
							LibGens::Vertex *vertex = new LibGens::Vertex();
							vertex->setPosition(LibGens::Vector3(src_mesh->mVertices[v].x, src_mesh->mVertices[v].y, src_mesh->mVertices[v].z));
							vertex->setNormal(LibGens::Vector3(src_mesh->mNormals[v].x, src_mesh->mNormals[v].y, src_mesh->mNormals[v].z));

							if (src_mesh->mTangents)
								vertex->setTangent(LibGens::Vector3(src_mesh->mTangents[v].x, src_mesh->mTangents[v].y, src_mesh->mTangents[v].z));

							if (src_mesh->mBitangents)
								vertex->setBinormal(LibGens::Vector3(src_mesh->mBitangents[v].x, src_mesh->mBitangents[v].y, src_mesh->mBitangents[v].z));

							if (src_mesh->GetNumColorChannels() && src_mesh->mColors)
								vertex->setColor(LibGens::Color(src_mesh->mColors[0][v].r, src_mesh->mColors[0][v].g, src_mesh->mColors[0][v].b, src_mesh->mColors[0][v].a));

							
							for (int uv=0; uv < uv_channels; uv++) {
								vertex->setUV(LibGens::Vector2(src_mesh->mTextureCoords[uv][v].x, 1.0 - src_mesh->mTextureCoords[uv][v].y), uv);
							}

							vertices.push_back(vertex);
						}

						for (int p=0; p < num_faces; p++) {
							if (src_mesh->mFaces[p].mNumIndices == 3) {
								LibGens::Polygon poly = { src_mesh->mFaces[p].mIndices[2], src_mesh->mFaces[p].mIndices[1], src_mesh->mFaces[p].mIndices[0] };
								polygons.push_back(poly);
							}
						}

						submesh->build(vertices, polygons);
						submesh->setVertexFormat(new LibGens::VertexFormat(LIBGENS_VERTEX_FORMAT_PC_TERRAIN));
						submesh->setMaterialName(material_name);
						submesh->addBone(0);

						// Find matching libgens material
						LibGens::Material *material = NULL;
						for (list<LibGens::Material *>::iterator it = scene_data.materials.begin(); it != scene_data.materials.end(); it++) {
							if ((*it)->getName() == material_name) {
								material = *it;
								break;
							}
						}

						size_t submesh_slot = LIBGENS_MODEL_SUBMESH_SLOT_SOLID;

						// If material was found, add special properties to the submesh
						if (material) {
							logProgress(ProgressNormal, QString("Found material for submesh already converted. Applying special properties."));

							vector<LibGens::Texture *> textures = material->getTextureUnits();
							for (size_t t = 0; t < textures.size(); t++) {
								submesh->addTextureUnit(textures[t]->getUnit());
								submesh->addTextureID(t);
							}

							string layer = material->getLayer();
							if (converter_settings.force_tags_layers)
								layer = material_tags.getTagValue(LibGens::Tag::KeyMaterialLayer, 0, layer);

							if (layer == LibGens::Material::LayerOpaq) {
								logProgress(ProgressNormal, QString("Placing submesh on opaq layer."));
								submesh_slot = LIBGENS_MODEL_SUBMESH_SLOT_SOLID;
							}
							else if (layer == LibGens::Material::LayerTrans) {
								logProgress(ProgressNormal, QString("Placing submesh on trans layer."));
								submesh_slot = LIBGENS_MODEL_SUBMESH_SLOT_TRANSPARENT;
							}
							else if (layer == LibGens::Material::LayerPunch) {
								logProgress(ProgressNormal, QString("Placing submesh on punch layer."));
								submesh_slot = LIBGENS_MODEL_SUBMESH_SLOT_BOOLEAN;
							}
							else {
								logProgress(ProgressNormal, QString("Placing submesh on custom layer %1.").arg(layer.c_str()));

								if (set_layer_string) {
									logProgress(ProgressWarning, QString("Another custom layer is being applied on top of the current one for this mesh. Do you have materials with different layers on the same mesh? Past layer will be overwritten."));
								}

								submesh_slot = LIBGENS_MODEL_SUBMESH_SLOT_WATER;
								mesh->setWaterSlotString(layer);
								set_layer_string = true;
							}
						}
						else {
							logProgress(ProgressWarning, QString("No material was found loaded for this submesh. Can't apply any special properties to it."));
						}

						mesh->addSubmesh(submesh, submesh_slot);

						logProgress(ProgressNormal, QString("Converted submesh and added to slot %1, with %2 resulting vertices and %3 resulting indices.").arg(submesh_slot).arg(submesh->getVerticesSize()).arg(submesh->getFacesIndicesSize()));
					}
				}

				mesh->buildAABB();
				model->addMesh(mesh);

				model->buildAABB();
				model->save(model_filename.toStdString());
				logProgress(ProgressNormal, QString("Saved model to %1.").arg(model_filename));

				ModelRecord record;
				record.used_meshes = mesh_indices;
				record.aabb = model->getAABB();
				scene_data.model_map[model_name] = record;

				logProgress(ProgressNormal, QString("Model AABB: [%1, %2, %3][%4, %5, %6].").arg(record.aabb.start.x).arg(record.aabb.start.y).arg(record.aabb.start.z).arg(record.aabb.end.x).arg(record.aabb.end.y).arg(record.aabb.end.z));

				scene_data.model_size_map[model->getName()] = model->getEstimatedMemorySize();
				logProgress(ProgressNormal, QString("Model Estimated Memory Size: %1 bytes.").arg(scene_data.model_size_map[model->getName()]));

				delete model;

				existing_model_name = model_name;
			}
			else {
				logProgress(ProgressNormal, QString("Detected %1 as the model for this instance.").arg(existing_model_name));
			}

			logProgress(ProgressNormal, QString("Creating instance with model name %1...").arg(existing_model_name));

			// Detect and rename if duplicate files are found
			int attempt = 0;
			QString instance_name = model_name;
			QString instance_filename = QString();
			do {
				instance_name = model_name + (attempt == 0 ? "" : QString("_%1").arg(attempt));
				instance_filename = path + "/" + instance_name + ".terrain-instanceinfo";
				attempt++;
			} while (QFileInfo(instance_filename).exists());

			if (model_name != instance_name) {
				logProgress(ProgressWarning, QString("Detected %1 as duplicate instance name. Renamed to %2.").arg(model_name).arg(instance_name));
			}

			LibGens::TerrainInstance *instance = new LibGens::TerrainInstance();
			instance->setName(instance_name.toStdString());
			instance->setModelName(existing_model_name.toStdString());
			instance->setMatrix(instance_matrix);
			LibGens::AABB instance_aabb = scene_data.model_map[existing_model_name].aabb;
			instance_aabb.transform(instance_matrix);
			instance->setAABB(instance_aabb);
			instance->save(instance_filename.toStdString());
			scene_data.instances.insertMulti(parent_group_id, instance);

			logProgress(ProgressNormal, QString("Saved instance to %1.").arg(instance_filename));
			logProgress(ProgressNormal, QString("Instance AABB: [%1, %2, %3][%4, %5, %6].").arg(instance_aabb.start.x).arg(instance_aabb.start.y).arg(instance_aabb.start.z).arg(instance_aabb.end.x).arg(instance_aabb.end.y).arg(instance_aabb.end.z));
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			convertSceneNode(scene, node->mChildren[i], path, scene_data, instance_matrix, parent_group_id);
		}

		return true;
	}

	return false;
}

void HCWindow::logNodeTree(aiNode *node, QString prefix) {
	if (node) {
		logProgress(ProgressNormal, QString("%1%2 (%3 Children, %4 Meshes)").arg(prefix).arg(node->mName.C_Str()).arg(node->mNumChildren).arg(node->mNumMeshes));

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			logNodeTree(node->mChildren[i], prefix + "*");
		}
	}
}

bool HCWindow::findNodeTransform(aiNode *node, aiMatrix4x4 parent_transform, aiString name, aiMatrix4x4 *return_transform) {
	if (node) {
		aiMatrix4x4 transform = parent_transform * node->mTransformation;
		if (node->mName == name) {
			*return_transform = transform;
			return true;
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			if (findNodeTransform(node->mChildren[i], transform, name, return_transform)) {
				return true;
			}
		}
	}

	return false;
}

bool HCWindow::packLostWorld(QString output_path, QString output_name, QString path) {
	// Remove previous pac files that could be in directory
	QDir dir(output_path);
	QStringList entry_list = dir.entryList(QStringList() << "*_trr_cmn.pac*");
	foreach(QString entry, entry_list) {
		QString remove_filename = output_path + "/" + entry;
		if (QFile::remove(remove_filename)) {
			logProgress(ProgressNormal, "Removed " + remove_filename + " for cleaning up before packing.");
		}
		else {
			logProgress(ProgressWarning, "Couldn't remove " + remove_filename + " for cleaning up before packing.");
		}
	}

	// Save pac files
	LibGens::PacSet *pac_set = new LibGens::PacSet();
	pac_set->addFolder((path + "/").toStdString());
	pac_set->splitPacks();
	pac_set->save((output_path + "/" + output_name + "_trr_cmn.pac").toStdString());
	delete pac_set;

	return true;
}

bool HCWindow::packGenerations(QList<LibGens::TerrainGroup *> &terrain_groups, QString output_path, QString output_name, QString path) {
	if (terrain_groups.size()) {
		logProgress(ProgressNormal, "Saving terrain groups for Stage.pfd...");

		QTemporaryDir stage_pfd_dir(temporaryDirTemplate());
		QString stage_pfd_path = stage_pfd_dir.path();
		LibGens::ArPack stage_pfd_pack;
		string output_stage_pfd_path = output_path.toStdString() + "/Stage.pfd";
		if (converter_settings.merge_existing && QFileInfo(output_stage_pfd_path.c_str()).exists()) {
			LibGens::ArPack existing_stage_pfd_pack(output_stage_pfd_path);
			logProgress(ProgressNormal, QString("Extracting %1.").arg(output_stage_pfd_path.c_str()));
			existing_stage_pfd_pack.extract(stage_pfd_path.toStdString() + "/");
		
			logProgress(ProgressNormal, QString("Removing existing terrain groups from %1.").arg(stage_pfd_path));
			QStringList entry_list = QDir(stage_pfd_path).entryList(QStringList() << "tg-*.ar");
			foreach(QString entry, entry_list) {
				QString remove_filename = stage_pfd_path + "/" + entry;
				bool removed_file = QFile::remove(remove_filename);
				if (removed_file) {
					logProgress(ProgressNormal, "Removed " + remove_filename + " before packing PFD.");
				}
				else {
					logProgress(ProgressError, "Couldn't remove " + remove_filename + " before packing PFD.");
				}
			}

			logProgress(ProgressNormal, QString("Adding existing GI groups from %1.").arg(stage_pfd_path));
			entry_list = QDir(stage_pfd_path).entryList(QStringList() << "gia-*.ar");
			foreach(QString entry, entry_list) {
				QString add_filename = stage_pfd_path + "/" + entry;
				stage_pfd_pack.addFile(add_filename.toStdString());
				logProgress(ProgressNormal, "Added " + add_filename + " before packing PFD.");
			}
		}

		// Save Terrain Group AR Files
		foreach(LibGens::TerrainGroup *group, terrain_groups) {
			QString group_filename = QString("%1/%2.ar").arg(stage_pfd_path).arg(group->getName().c_str());
			LibGens::ArPack tg_ar;

			vector<LibGens::Model *> models = group->getModels();
			list<LibGens::TerrainInstance *> instances = group->getInstances();
			for (vector<LibGens::Model *>::iterator it = models.begin(); it != models.end(); it++) {
				string model_name = (*it)->getName() + ".terrain-model";
				string model_filename = path.toStdString() + "/" + model_name;
				LibGens::File model_file(model_filename, LIBGENS_FILE_READ_BINARY);
				if (model_file.valid()) {
					model_file.close();

					tg_ar.addFile(model_filename);
					logProgress(ProgressNormal, QString("Added %1 to %2 AR Pack.").arg(model_filename.c_str()).arg(group->getName().c_str()));
				}
			}

			for (list<LibGens::TerrainInstance *>::iterator it = instances.begin(); it != instances.end(); it++) {
				string instance_name = (*it)->getName() + ".terrain-instanceinfo";
				string instance_filename = path.toStdString() + "/" + instance_name;
				LibGens::File instance_file(instance_filename, LIBGENS_FILE_READ_BINARY);
				if (instance_file.valid()) {
					instance_file.close();

					tg_ar.addFile(instance_filename);
					logProgress(ProgressNormal, QString("Added %1 to %2 AR Pack.").arg(instance_filename.c_str()).arg(group->getName().c_str()));
				}
			}

			tg_ar.save(group_filename.toStdString());

			logProgress(ProgressNormal, QString("Compressing %1 with CAB Compression.").arg(group_filename));

			QStringList arguments;
			arguments << group_filename << group_filename;
			QProcess conversion_process;
			conversion_process.start("makecab", arguments);
			conversion_process.waitForFinished();
			QString conversion_output = conversion_process.readAllStandardOutput();
			logProgress(ProgressNormal, QString("Cabinet Maker Output: " + conversion_output));

			stage_pfd_pack.addFile(group_filename.toStdString());
			logProgress(ProgressNormal, "Saving terrain group to " + group_filename + "...");
		}

		// Save PFD files
		stage_pfd_pack.save(output_stage_pfd_path);
		logProgress(ProgressNormal, "Packed Stage.pfd");
		stage_pfd_pack.savePFI(path.toStdString() + "/Stage.pfi");
		logProgress(ProgressNormal, "Saved Stage.pfi");
	}

	// Delete all terrain model and instances from the resources directory
	QDir temp_dir(path);
	QStringList entry_list = temp_dir.entryList(QStringList() << "*.terrain-instanceinfo" << "*.terrain-model");
	foreach(QString entry, entry_list) {
		QString remove_filename = path + "/" + entry;
		bool removed_file = QFile::remove(remove_filename);
		if (removed_file) {
			logProgress(ProgressNormal, "Removed " + remove_filename + " before packing resource files.");
		}
		else {
			logProgress(ProgressError, "Couldn't remove " + remove_filename + " before packing resource files.");
		}
	}

	// Finally, pack the resources directory
	LibGens::ArPack resources_pack;
	entry_list = temp_dir.entryList(QStringList() << "*");
	foreach(QString entry, entry_list) {
		resources_pack.addFile((path + "/" + entry).toStdString());
	}

	QString resources_pack_filename = QString("%1/%2.ar.00").arg(output_path).arg(output_name);
	resources_pack.save(resources_pack_filename.toStdString());
	logProgress(ProgressNormal, "Saved " + resources_pack_filename + ".");
	return true;
}