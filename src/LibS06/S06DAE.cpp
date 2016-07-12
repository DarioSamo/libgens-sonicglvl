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

#include "LibGens.h"
#include "S06XnFile.h"

namespace LibGens {
	void SonicXNFile::saveDAE(string filename, bool only_animation, float unit_scale) {
		TiXmlDocument doc;
		TiXmlDeclaration *decl = new TiXmlDeclaration( "1.0", "", "" );
		doc.LinkEndChild( decl );


		printf("Save DAE called...\n");

		SonicXNObject *object=getObject();
		if (object) printf("Found Object...\n");

		SonicXNTexture *texture=getTexture();
		if (texture) printf("Found Texture...\n");

		SonicXNBones *bones=getBones();
		if (bones) printf("Found Bones...\n");

		SonicXNMotion *motion=getMotion();
		if (motion) printf("Found Motion...\n");

		printf("Model Names Set...\n");
		TiXmlElement *colladaRoot = new TiXmlElement("COLLADA");
		colladaRoot->SetAttribute("xmlns", "http://www.collada.org/2005/11/COLLADASchema");
		colladaRoot->SetAttribute("version", "1.4.1");

		printf("Creating Collada Root...\n");

		TiXmlElement *assetRoot = new TiXmlElement("asset");
		{
			TiXmlElement *newElem = new TiXmlElement("contributor");
			TiXmlElement *authoring = new TiXmlElement("authoring_tool");
			TiXmlText *text = new TiXmlText("LibS06 - COLLADA Exporter");
			authoring->LinkEndChild(text);
			newElem->LinkEndChild(authoring);
			assetRoot->LinkEndChild(newElem);

			newElem = new TiXmlElement("unit");
			newElem->SetAttribute("name", "meters");
			newElem->SetAttribute("meter", "1.0");
			assetRoot->LinkEndChild(newElem);

			newElem = new TiXmlElement("up_axis");
			text = new TiXmlText("Y_UP");
			newElem->LinkEndChild(text);
			assetRoot->LinkEndChild(newElem);
		}
		colladaRoot->LinkEndChild(assetRoot);

		printf("Creating Texture Library...\n");

		// Texture library
		if (texture && !only_animation) {
			TiXmlElement *imagesRoot = new TiXmlElement("library_images");
			vector<string> textures = texture->getTextures();


			string target_folder = filename;
			size_t sz=target_folder.size();
			int last_slash=0;
			for (size_t i=0; i<sz; i++) {
				if ((target_folder[i] == '\\') || (target_folder[i] == '/')) last_slash=i;
			}
			if (last_slash) target_folder.erase(last_slash+1, target_folder.size()-last_slash-1);
			else target_folder="";
	

			CreateDirectory((target_folder+"textures").c_str(), NULL);
			for (size_t j=0; j<textures.size(); j++) {
				string tex_name=textures[j];
				
				size_t pos=tex_name.find(".gvr");
				if (pos == string::npos) pos=tex_name.find(".GVR");
				if (pos != string::npos) {
					std::transform(tex_name.begin(), tex_name.end(), tex_name.begin(), ::tolower);
					tex_name.replace(pos, 4, ".png");
				}

				TiXmlElement *imageNode = new TiXmlElement("image");
				imageNode->SetAttribute("id", tex_name+"-image");
				imageNode->SetAttribute("name", tex_name+"-image");

				string nm="./textures/"+tex_name;
				TiXmlElement *newElem = new TiXmlElement("init_from");
				TiXmlText *text = new TiXmlText(nm);
				newElem->LinkEndChild(text);
				imageNode->LinkEndChild(newElem);

				File texture(folder+tex_name, "rb");
				if (texture.valid()) {
					texture.clone(target_folder + nm);
					texture.close();
				}

				imagesRoot->LinkEndChild(imageNode);
			}
			colladaRoot->LinkEndChild(imagesRoot);
		}

		

		if (object) {
			if (!only_animation) {
				// Material Library
				if (texture) {
					printf("Creating Material Library...\n");
					TiXmlElement *materialsRoot = new TiXmlElement("library_materials");
					object->writeMaterialDAE(materialsRoot);
					colladaRoot->LinkEndChild(materialsRoot);
				
				
					printf("Creating Effects Library...\n");
					// Effects library
					TiXmlElement *effectsRoot = new TiXmlElement("library_effects");
					object->writeEffectsDAE(effectsRoot, texture);
					colladaRoot->LinkEndChild(effectsRoot);
				}
		
				printf("Creating Geometry Library...\n");
				// Geometry Library
				TiXmlElement *geometryRoot = new TiXmlElement("library_geometries");
				object->writeMeshesDAE(geometryRoot, unit_scale);
				colladaRoot->LinkEndChild(geometryRoot);

				printf("Creating Controller Library...\n");
				// Controller Library
				TiXmlElement *controllersRoot = new TiXmlElement("library_controllers");
				object->writeControllerDAE(controllersRoot, unit_scale);
				colladaRoot->LinkEndChild(controllersRoot);
			}
			
			if (motion) {
				printf("Creating Animation Library...\n");
				// Animation Library
				TiXmlElement *animationRoot = new TiXmlElement("library_animations");
				motion->writeDAE(animationRoot, object, bones, unit_scale);
				colladaRoot->LinkEndChild(animationRoot);
			}

			printf("Creating Visual Scene...\n");
			// Visual Scene
			TiXmlElement *visualScenesRoot = new TiXmlElement("library_visual_scenes");
			{
				TiXmlElement *visualScene = new TiXmlElement("visual_scene");
				visualScene->SetAttribute("id", "LibGensScene");
				visualScene->SetAttribute("name", "LibGensScene");

				printf("Writing main object...\n");
				object->writeDAE(visualScene, only_animation, unit_scale);

				printf("Checking for motion...\n");
				if (motion) {
					TiXmlElement *extraNode = new TiXmlElement("extra");

					TiXmlElement *techniqueMax3DNode = new TiXmlElement("technique");
					techniqueMax3DNode->SetAttribute("profile", "MAX3D");
					{
						TiXmlElement *frameRateNode = new TiXmlElement("frame_rate");
						TiXmlText *pValue=new TiXmlText("30.000000");
						frameRateNode->LinkEndChild(pValue);
						techniqueMax3DNode->LinkEndChild(frameRateNode);
					}
					extraNode->LinkEndChild(techniqueMax3DNode);

					TiXmlElement *techniqueColladaNode = new TiXmlElement("technique");
					techniqueColladaNode->SetAttribute("profile", "FCOLLADA");
					{
						TiXmlElement *startTimeNode = new TiXmlElement("start_time");
						TiXmlText *pValue=new TiXmlText("0.000000");
						startTimeNode->LinkEndChild(pValue);
						techniqueColladaNode->LinkEndChild(startTimeNode);

						TiXmlElement *endTimeNode = new TiXmlElement("end_time");
						pValue=new TiXmlText(ToString(motion->getDuration()));
						endTimeNode->LinkEndChild(pValue);
						techniqueColladaNode->LinkEndChild(endTimeNode);
					}
					extraNode->LinkEndChild(techniqueColladaNode);

					visualScene->LinkEndChild(extraNode);
				}

				visualScenesRoot->LinkEndChild(visualScene);
			}
			colladaRoot->LinkEndChild(visualScenesRoot);
		}
	
		TiXmlElement *sceneRoot = new TiXmlElement("scene");
		{
			TiXmlElement *newElem = new TiXmlElement("instance_visual_scene");
			newElem->SetAttribute("url", "#LibGensScene");
			sceneRoot->LinkEndChild(newElem);
		}
		colladaRoot->LinkEndChild(sceneRoot);


		doc.LinkEndChild(colladaRoot);
		doc.SaveFile(filename.c_str());
	}


	void SonicXNObject::writeBonesDAE(TiXmlElement *root, size_t current, float unit_scale) {
		printf("Writing bone %d...\n", current);
		string bone_name=(bones_names ? bones_names->getName(current) : name+ToString(current));

		Error::addMessage(Error::WARNING, "Writing bone " + ToString(current) + " with name " + bone_name + " and Skinning Matrix Index " + ToString(bones[current]->matrix_index));

		printf("Writing bone with name %s...\n", bone_name.c_str());

		TiXmlElement *nodeRoot = new TiXmlElement("node");
		nodeRoot->SetAttribute("id", bone_name);
		nodeRoot->SetAttribute("sid", bone_name);
		nodeRoot->SetAttribute("name", bone_name);
		nodeRoot->SetAttribute("type", "JOINT");

		Matrix4 m=bones[current]->current_matrix;
		string nm="";
		nm += ToString(m[0][0]) + " " + ToString(m[0][1]) + " " + ToString(m[0][2]) + " " + ToString(m[0][3] * unit_scale) + " ";
		nm += ToString(m[1][0]) + " " + ToString(m[1][1]) + " " + ToString(m[1][2]) + " " + ToString(m[1][3] * unit_scale) + " ";
		nm += ToString(m[2][0]) + " " + ToString(m[2][1]) + " " + ToString(m[2][2]) + " " + ToString(m[2][3] * unit_scale) + " ";
		nm += ToString(m[3][0]) + " " + ToString(m[3][1]) + " " + ToString(m[3][2]) + " " + ToString(m[3][3] * unit_scale) + " ";

		TiXmlElement *matrixRoot = new TiXmlElement("matrix");
		TiXmlText *text = new TiXmlText(nm);
		matrixRoot->LinkEndChild(text);
		nodeRoot->LinkEndChild(matrixRoot);

		if (bones[current]->sibling_index != 0xFFFF) writeBonesDAE(root, bones[current]->sibling_index, unit_scale);
		if (bones[current]->child_index   != 0xFFFF) writeBonesDAE(nodeRoot, bones[current]->child_index, unit_scale);

		TiXmlElement *extraRoot = new TiXmlElement("extra");
		TiXmlElement *techniqueRoot = new TiXmlElement("technique");
		techniqueRoot->SetAttribute("profile", "FCOLLADA");
		TiXmlElement *visibilityRoot = new TiXmlElement("visibility");
		text = new TiXmlText("1.000000");
		visibilityRoot->LinkEndChild(text);
		techniqueRoot->LinkEndChild(visibilityRoot);
		extraRoot->LinkEndChild(techniqueRoot);
		nodeRoot->LinkEndChild(extraRoot);

		root->LinkEndChild(nodeRoot);
	}
	
	void SonicXNObject::writeControllerDAE(TiXmlElement *root, float unit_scale) {
		TiXmlElement *controllerRoot = new TiXmlElement("controller");
		controllerRoot->SetAttribute("id", name+"-controller");

		TiXmlElement *skinRoot = new TiXmlElement("skin");
		skinRoot->SetAttribute("source", "#"+name+"-geometry");
		{
			TiXmlElement *bindShapeRoot = new TiXmlElement("bind_shape_matrix");
			TiXmlText *text = new TiXmlText("1.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0 1.0");
			bindShapeRoot->LinkEndChild(text);
			skinRoot->LinkEndChild(bindShapeRoot);
		}

		vector<float> bone_weights;
		vector< vector<unsigned int> > vertex_joint_map;
		vector< vector<unsigned int> > vertex_weight_map;

		// XNO and ZNO
		for (size_t i=0; i<vertex_tables.size(); i++) {
			vector<unsigned int> blending_table=vertex_tables[i]->bone_table;

			for (size_t j=0; j<vertex_tables[i]->vertices.size(); j++) {
				vector<unsigned int> joint_map;
				vector<unsigned int> weight_map;
				vector<SonicVertex *> vertices=vertex_tables[i]->vertices;

				if (!blending_table.size()) {
					joint_map.push_back(0);
					weight_map.push_back(0);

					bool added=false;
					float w=1.0;

					for (size_t x=0; x<bone_weights.size(); x++) {
						if (w==bone_weights[x]) {
							added=true;
							weight_map.push_back(x);
							break;
						}
					}
					if (!added) {
						bone_weights.push_back(w);
						weight_map.push_back(bone_weights.size()-1);
					}

					vertex_joint_map.push_back(joint_map);
					vertex_weight_map.push_back(weight_map);
					continue;
				}
				
				for (int k=0; k<4; k++) {
					if (vertices[j]->bone_weights_f[k] > 0.0f) {
						if (vertices[j]->bone_indices[k] >= blending_table.size()) {
							printf("Index(%d) out of range: %d %d\n", k, (int)vertices[j]->bone_indices[k], blending_table.size());
						}

						size_t index=blending_table[vertices[j]->bone_indices[k]];
						SonicBone *bone=NULL;
						bool player_switch=false;

						for (size_t b=0; b<bones.size(); b++) {
							if (bones[b]->matrix_index == index) {
								joint_map.push_back(b);
								break;
							}
						}


						bool added=false;
						float w=vertices[j]->bone_weights_f[k];
						for (size_t x=0; x<bone_weights.size(); x++) {
							if (w==bone_weights[x]) {
								added=true;
								weight_map.push_back(x);
								break;
							}
						}
						if (!added) {
							bone_weights.push_back(w);
							weight_map.push_back(bone_weights.size()-1);
						}
					}
				}

				vertex_joint_map.push_back(joint_map);
				vertex_weight_map.push_back(weight_map);
			}
		}

		// GNO
		for (size_t i=0; i<vertex_resource_tables.size(); i++) {
			// Add bone data depending on the submeshes and matrix indices
			// Proper approach would be to create duplicates as needed, but this might just work

			if (!vertex_resource_tables[i]->bones.size()) {
				for (size_t x=0; x<vertex_resource_tables[i]->positions.size(); x++) {
					SonicVertexBoneData bone_data;
					bone_data.bone_1 = 0;
					bone_data.bone_2 = 0;
					bone_data.weight = 16384;
					vertex_resource_tables[i]->bones.push_back(bone_data);
				}


				for (size_t m=0; m<meshes.size(); m++) {
					for (size_t s=0; s<meshes[m]->submeshes.size(); s++) {
						unsigned int vertex_index  = meshes[m]->submeshes[s]->vertex_index;
						unsigned int polygon_index = meshes[m]->submeshes[s]->indices_index;
						unsigned int matrix_index  = meshes[m]->submeshes[s]->matrix_index;

						if (vertex_index == i) {
							SonicPolygonTable *polygon_table = polygon_tables[polygon_index];

							for (size_t x=0; x<polygon_table->faces.size(); x++) {
								for (size_t f=0; f<3; f++) {
									unsigned short position_index=polygon_table->faces[x]->points[f].position_index;
									vertex_resource_tables[i]->bones[position_index].bone_1 = matrix_index;
								}
							}
						}
					}
				}
			}

			for (size_t j=0; j<vertex_resource_tables[i]->bones.size(); j++) {
				vector<unsigned int> joint_map;
				vector<unsigned int> weight_map;

				SonicVertexBoneData *bone_data=&vertex_resource_tables[i]->bones[j];

				if (bone_data->weight > 0) {
					for (size_t b=0; b<bones.size(); b++) {
						if (bones[b]->matrix_index == bone_data->bone_1) {
							joint_map.push_back(b);
							break;
						}
					}

					bool added=false;
					float w=(bone_data->weight)/16384.0;
					for (size_t x=0; x<bone_weights.size(); x++) {
						if (w==bone_weights[x]) {
							added=true;
							weight_map.push_back(x);
							break;
						}
					}
					if (!added) {
						bone_weights.push_back(w);
						weight_map.push_back(bone_weights.size()-1);
					}
				}

				if (bone_data->weight < 16384) {
					for (size_t b=0; b<bones.size(); b++) {
						if (bones[b]->matrix_index == bone_data->bone_2) {
							joint_map.push_back(b);
							break;
						}
					}

					bool added=false;
					float w=1.0 - (bone_data->weight)/16384.0;
					for (size_t x=0; x<bone_weights.size(); x++) {
						if (w==bone_weights[x]) {
							added=true;
							weight_map.push_back(x);
							break;
						}
					}
					if (!added) {
						bone_weights.push_back(w);
						weight_map.push_back(bone_weights.size()-1);
					}
				}

				vertex_joint_map.push_back(joint_map);
				vertex_weight_map.push_back(weight_map);
			}
		}


		// Joints
		TiXmlElement *sourceRoot = new TiXmlElement("source");
		sourceRoot->SetAttribute("id", name+"-controller-Joints");
		{
			// Names
			TiXmlElement *nameArray = new TiXmlElement("Name_array");
			nameArray->SetAttribute("id", name+"-controller-Joints-array");
			nameArray->SetAttribute("count", ToString(bones.size()));

			string nm="";
			for (size_t i=0; i<bones.size(); i++) {
				string bone_name=(bones_names ? bones_names->getName(i) : name+ToString(i));
				nm += bone_name + " ";
			}
			TiXmlText *text = new TiXmlText(nm);
			nameArray->LinkEndChild(text);
			sourceRoot->LinkEndChild(nameArray);

			// Technique
			TiXmlElement *techniqueRoot = new TiXmlElement("technique_common");
			TiXmlElement *accessorRoot = new TiXmlElement("accessor");
			accessorRoot->SetAttribute("source", "#"+name+"-controller-Joints-array");
			accessorRoot->SetAttribute("count", ToString(bones.size()));

			TiXmlElement *paramRoot = new TiXmlElement("param");
			paramRoot->SetAttribute("type", "name");
			accessorRoot->LinkEndChild(paramRoot);
	
			techniqueRoot->LinkEndChild(accessorRoot);
			sourceRoot->LinkEndChild(techniqueRoot);
		}
		skinRoot->LinkEndChild(sourceRoot);


		sourceRoot = new TiXmlElement("source");
		sourceRoot->SetAttribute("id", name+"-controller-Matrices");
		{
			// Matrices
			TiXmlElement *floatArray = new TiXmlElement("float_array");
			floatArray->SetAttribute("id", name+"-controller-Matrices-array");
			floatArray->SetAttribute("count", ToString(16*bones.size()));

			string nm="";
			for (size_t i=0; i<bones.size(); i++) {
				Matrix4 m=bones[i]->matrix.transpose();

				nm+=ToString(m[0][0]) + " " + ToString(m[0][1]) + " " + ToString(m[0][2]) + " " + ToString(m[0][3]*unit_scale) + " ";
				nm+=ToString(m[1][0]) + " " + ToString(m[1][1]) + " " + ToString(m[1][2]) + " " + ToString(m[1][3]*unit_scale) + " ";
				nm+=ToString(m[2][0]) + " " + ToString(m[2][1]) + " " + ToString(m[2][2]) + " " + ToString(m[2][3]*unit_scale) + " ";
				nm+=ToString(m[3][0]) + " " + ToString(m[3][1]) + " " + ToString(m[3][2]) + " " + ToString(m[3][3]) + " ";
			}
			TiXmlText *text = new TiXmlText(nm);
			floatArray->LinkEndChild(text);
			sourceRoot->LinkEndChild(floatArray);

			// Technique
			TiXmlElement *techniqueRoot = new TiXmlElement("technique_common");
			TiXmlElement *accessorRoot = new TiXmlElement("accessor");
			accessorRoot->SetAttribute("source", "#"+name+"-controller-Matrices-array");
			accessorRoot->SetAttribute("count", ToString(bones.size()));
			accessorRoot->SetAttribute("stride", "16");

			TiXmlElement *paramRoot = new TiXmlElement("param");
			paramRoot->SetAttribute("type", "float4x4");
			accessorRoot->LinkEndChild(paramRoot);

			techniqueRoot->LinkEndChild(accessorRoot);
			sourceRoot->LinkEndChild(techniqueRoot);
		}
		skinRoot->LinkEndChild(sourceRoot);

		sourceRoot = new TiXmlElement("source");
		sourceRoot->SetAttribute("id", name+"-controller-Weights");
		{
			// Weights
			TiXmlElement *floatArray = new TiXmlElement("float_array");
			floatArray->SetAttribute("id", name+"-controller-Weights-array");
			floatArray->SetAttribute("count", ToString(bone_weights.size()));

			string nm="";
			for (size_t i=0; i<bone_weights.size(); i++) {
				nm += ToString(bone_weights[i]) + " ";
			}
			TiXmlText *text = new TiXmlText(nm);
			floatArray->LinkEndChild(text);
			sourceRoot->LinkEndChild(floatArray);

			// Technique
			TiXmlElement *techniqueRoot = new TiXmlElement("technique_common");
			TiXmlElement *accessorRoot = new TiXmlElement("accessor");
			accessorRoot->SetAttribute("source", "#"+name+"-controller-Weights-array");
			accessorRoot->SetAttribute("count", ToString(bone_weights.size()));

			TiXmlElement *paramRoot = new TiXmlElement("param");
			paramRoot->SetAttribute("type", "float");
			accessorRoot->LinkEndChild(paramRoot);

			techniqueRoot->LinkEndChild(accessorRoot);
			sourceRoot->LinkEndChild(techniqueRoot);
		}
		skinRoot->LinkEndChild(sourceRoot);


		// Joints Root
		sourceRoot = new TiXmlElement("joints");
		{
			TiXmlElement *input = new TiXmlElement("input");
			input->SetAttribute("semantic", "JOINT");
			input->SetAttribute("source", "#"+name+"-controller-Joints");
			sourceRoot->LinkEndChild(input);

			input = new TiXmlElement("input");
			input->SetAttribute("semantic", "INV_BIND_MATRIX");
			input->SetAttribute("source", "#"+name+"-controller-Matrices");
			sourceRoot->LinkEndChild(input);
		}
		skinRoot->LinkEndChild(sourceRoot);


		// Weights
		sourceRoot = new TiXmlElement("vertex_weights");
		sourceRoot->SetAttribute("count", ToString(vertex_joint_map.size()));
		{
			TiXmlElement *input = new TiXmlElement("input");
			input->SetAttribute("semantic", "JOINT");
			input->SetAttribute("offset", "0");
			input->SetAttribute("source", "#"+name+"-controller-Joints");
			sourceRoot->LinkEndChild(input);

			input = new TiXmlElement("input");
			input->SetAttribute("semantic", "WEIGHT");
			input->SetAttribute("offset", "1");
			input->SetAttribute("source", "#"+name+"-controller-Weights");
			sourceRoot->LinkEndChild(input);

			string nm="";
			for (size_t i=0; i<vertex_joint_map.size(); i++) {
				nm+=ToString(vertex_joint_map[i].size())+" ";
			}

			TiXmlElement *vcount = new TiXmlElement("vcount");
			TiXmlText *text= new TiXmlText(nm);
			vcount->LinkEndChild(text);
			sourceRoot->LinkEndChild(vcount);


			nm="";
			for (size_t i=0; i<vertex_joint_map.size(); i++) {
				for (size_t k=0; k<vertex_joint_map[i].size(); k++) {
					nm += ToString(vertex_joint_map[i][k]) + " " + ToString(vertex_weight_map[i][k]) + " ";
				}
			}
	
			TiXmlElement *v= new TiXmlElement("v");
			text= new TiXmlText(nm);
			v->LinkEndChild(text);
			sourceRoot->LinkEndChild(v);
		}
		skinRoot->LinkEndChild(sourceRoot);
	
		controllerRoot->LinkEndChild(skinRoot);
		root->LinkEndChild(controllerRoot);
	}

	void SonicXNObject::writeMeshesDAE(TiXmlElement *root, float unit_scale) {
		TiXmlElement *geometryRoot = new TiXmlElement("geometry");
		geometryRoot->SetAttribute("id", name+"-geometry");
		geometryRoot->SetAttribute("name", name+"-geometry");

		TiXmlElement *meshRoot = new TiXmlElement("mesh");
		geometryRoot->LinkEndChild(meshRoot);

		vector<int> pfaces;
		pfaces.clear();

		vector<Vector3> base_vertices;
		vector<Vector3> base_vert_normals;
		vector<Vector2> base_uvs;
		vector<Vector2> base_uvs_2;
		vector<Color>   base_colors;
		vector<Vector3> base_indices;

		base_vertices.clear();
		base_vert_normals.clear();
		base_uvs.clear();
		base_uvs_2.clear();
		base_colors.clear();
		base_indices.clear();

		printf("Creating Indices...\n");

		unsigned int global_index=0;
		vector<unsigned int> global_indices;
		for (size_t x=0; x<vertex_tables.size(); x++) {
			vector<SonicVertex *> vertices = vertex_tables[x]->vertices;

			for (size_t i=0; i<vertices.size(); i++) {
				// Position
				base_vertices.push_back(vertices[i]->position);
				pfaces.push_back(base_vertices.size()-1);

				// Normals
				bool added=false;
				for (size_t k=0; k<base_vert_normals.size(); k++) {
					if (base_vert_normals[k] == vertices[i]->normal) {
						pfaces.push_back(k);
						added=true;
						break;
					}
				}
				if (!added) {
					base_vert_normals.push_back(vertices[i]->normal);
					pfaces.push_back(base_vert_normals.size()-1);
				}


				// UVs
				added=false;
				for (size_t k=0; k<base_uvs.size(); k++) {
					if (base_uvs[k] == vertices[i]->uv[0]) {
						pfaces.push_back(k);
						added=true;
						break;
					}
				}
				if (!added) {
					base_uvs.push_back(vertices[i]->uv[0]);
					pfaces.push_back(base_uvs.size()-1);
				}

				// UVs 2
				added=false;
				for (size_t k=0; k<base_uvs_2.size(); k++) {
					if (base_uvs_2[k] == vertices[i]->uv[1]) {
						pfaces.push_back(k);
						added=true;
						break;
					}
				}
				if (!added) {
					base_uvs_2.push_back(vertices[i]->uv[1]);
					pfaces.push_back(base_uvs_2.size()-1);
				}

				// Color
				added=false;
				for (size_t k=0; k<base_colors.size(); k++) {
					if (base_colors[k] == Color(vertices[i]->rgba)) {
						pfaces.push_back(k);
						added=true;
						break;
					}
				}
				if (!added) {
					base_colors.push_back(Color(vertices[i]->rgba));
					pfaces.push_back(base_colors.size()-1);
				}
			}

			global_indices.push_back(global_index);
			global_index += vertices.size();
		}

		printf("Pushing Indices...\n");

		if (index_tables.size()) {
			for (size_t m=0; m<meshes.size(); m++) {
				for (size_t s=0; s<meshes[m]->submeshes.size(); s++) {
					size_t indices_index=meshes[m]->submeshes[s]->indices_index;

					vector<Vector3> indices_vector=index_tables[indices_index]->indices_vector;
					global_index = global_indices[meshes[m]->submeshes[s]->vertex_index];

					for (size_t i=0; i<indices_vector.size(); i++) {
						Vector3 face;
						face.x = indices_vector[i].x + global_index;
						face.y = indices_vector[i].y + global_index;
						face.z = indices_vector[i].z + global_index;
						base_indices.push_back(face);
					}
				}
			}
		}


		if (file_mode == MODE_GNO) {
			base_vert_normals.push_back(Vector3(1.0, 0.0, 0.0));
			base_uvs.push_back(Vector2());
			base_uvs_2.push_back(Vector2());
			base_colors.push_back(Color(1.0, 1.0, 1.0, 1.0));
		}

		for (size_t x=0; x<vertex_resource_tables.size(); x++) {
			size_t list_size=vertex_resource_tables[x]->positions.size();
			for (size_t i=0; i<list_size; i++) {
				base_vertices.push_back(vertex_resource_tables[x]->positions[i]);
			}

			list_size=vertex_resource_tables[x]->normals.size();
			for (size_t i=0; i<list_size; i++) {
				base_vert_normals.push_back(vertex_resource_tables[x]->normals[i]);
			}

			list_size=vertex_resource_tables[x]->uvs.size();
			for (size_t i=0; i<list_size; i++) {
				base_uvs.push_back(vertex_resource_tables[x]->uvs[i]);
			}

			list_size=vertex_resource_tables[x]->uvs_2.size();
			for (size_t i=0; i<list_size; i++) {
				base_uvs_2.push_back(vertex_resource_tables[x]->uvs_2[i]);
			}

			list_size=vertex_resource_tables[x]->colors.size();
			for (size_t i=0; i<list_size; i++) {
				base_colors.push_back(vertex_resource_tables[x]->colors[i]);
			}
		}

		printf("Creating Geometry Positions...\n");

		TiXmlElement *sourcePosition = new TiXmlElement("source");
		sourcePosition->SetAttribute("id", name+"-geometry-position");
		{
			// Float array with positions
			TiXmlElement *floatArray = new TiXmlElement("float_array");
			floatArray->SetAttribute("id", name+"-geometry-position-array");
			floatArray->SetAttribute("count", ToString(base_vertices.size()*3));

			string farray_string="";
			for (size_t i=0; i<base_vertices.size(); i++) {
				farray_string+=ToString(base_vertices[i].x*unit_scale)+" ";
				farray_string+=ToString(base_vertices[i].y*unit_scale)+" ";
				farray_string+=ToString(base_vertices[i].z*unit_scale)+" ";
			}

			TiXmlText *text = new TiXmlText(farray_string);
			floatArray->LinkEndChild(text);
			sourcePosition->LinkEndChild(floatArray);

			// Technique declaration
			TiXmlElement *technique = new TiXmlElement("technique_common");
			{
				TiXmlElement *accessor = new TiXmlElement("accessor");
				accessor->SetAttribute("source", "#"+name+"-geometry-position-array");
				accessor->SetAttribute("count", ToString(base_vertices.size()));
				accessor->SetAttribute("stride", "3");
				{
					TiXmlElement *xparam = new TiXmlElement("param");
					TiXmlElement *yparam = new TiXmlElement("param");
					TiXmlElement *zparam = new TiXmlElement("param");

					xparam->SetAttribute("name", "X");
					yparam->SetAttribute("name", "Y");
					zparam->SetAttribute("name", "Z");

					xparam->SetAttribute("type", "float");
					yparam->SetAttribute("type", "float");
					zparam->SetAttribute("type", "float");

					accessor->LinkEndChild(xparam);
					accessor->LinkEndChild(yparam);
					accessor->LinkEndChild(zparam);
				}
	
				technique->LinkEndChild(accessor);
			}

			sourcePosition->LinkEndChild(technique);
		}
		meshRoot->LinkEndChild(sourcePosition);

		printf("Creating Geometry Normals...\n");

		TiXmlElement *sourceNormal = new TiXmlElement("source");
		sourceNormal->SetAttribute("id", name+"-geometry-normal");
		{
			// Float array with positions
			TiXmlElement *floatArray = new TiXmlElement("float_array");
			floatArray->SetAttribute("id", name+"-geometry-normal-array");
			floatArray->SetAttribute("count", ToString(base_vert_normals.size()*3));

			string farray_string="";
			for (size_t i=0; i<base_vert_normals.size(); i++) {
				farray_string+=ToString(base_vert_normals[i].x)+" ";
				farray_string+=ToString(base_vert_normals[i].y)+" ";
				farray_string+=ToString(base_vert_normals[i].z)+" ";
			}

			TiXmlText *text = new TiXmlText(farray_string);
			floatArray->LinkEndChild(text);
			sourceNormal->LinkEndChild(floatArray);

			// Technique declaration
			TiXmlElement *technique = new TiXmlElement("technique_common");
			{
				TiXmlElement *accessor = new TiXmlElement("accessor");
				accessor->SetAttribute("source", "#"+name+"-geometry-normal-array");
				accessor->SetAttribute("count", ToString(base_vert_normals.size()));
				accessor->SetAttribute("stride", "3");
				{
					TiXmlElement *xparam = new TiXmlElement("param");
					TiXmlElement *yparam = new TiXmlElement("param");
					TiXmlElement *zparam = new TiXmlElement("param");

					xparam->SetAttribute("name", "X");
					yparam->SetAttribute("name", "Y");
					zparam->SetAttribute("name", "Z");

					xparam->SetAttribute("type", "float");
					yparam->SetAttribute("type", "float");
					zparam->SetAttribute("type", "float");

					accessor->LinkEndChild(xparam);
					accessor->LinkEndChild(yparam);
					accessor->LinkEndChild(zparam);
				}

				technique->LinkEndChild(accessor);
			}

			sourceNormal->LinkEndChild(technique);
		}
		meshRoot->LinkEndChild(sourceNormal);

		printf("Creating Geometry UVs...\n");

		TiXmlElement *sourceUV = new TiXmlElement("source");
		sourceUV->SetAttribute("id", name+"-geometry-uv");
		{
			// Float array with positions
			TiXmlElement *floatArray = new TiXmlElement("float_array");
			floatArray->SetAttribute("id", name+"-geometry-uv-array");
			floatArray->SetAttribute("count", ToString(base_uvs.size()*2));

			string farray_string="";
			for (size_t i=0; i<base_uvs.size(); i++) {
				farray_string+=ToString(base_uvs[i].x)+" ";
				farray_string+=ToString(1.0 - base_uvs[i].y)+" ";
			}

			TiXmlText *text = new TiXmlText(farray_string);
			floatArray->LinkEndChild(text);
			sourceUV->LinkEndChild(floatArray);

			// Technique declaration
			TiXmlElement *technique = new TiXmlElement("technique_common");
			{
				TiXmlElement *accessor = new TiXmlElement("accessor");
				accessor->SetAttribute("source", "#"+name+"-geometry-uv-array");
				accessor->SetAttribute("count", ToString(base_uvs.size()));
				accessor->SetAttribute("stride", "2");
				{
					TiXmlElement *xparam = new TiXmlElement("param");
					TiXmlElement *yparam = new TiXmlElement("param");

					xparam->SetAttribute("name", "S");
					yparam->SetAttribute("name", "T");

					xparam->SetAttribute("type", "float");
					yparam->SetAttribute("type", "float");

					accessor->LinkEndChild(xparam);
					accessor->LinkEndChild(yparam);
				}

				technique->LinkEndChild(accessor);
			}

			sourceUV->LinkEndChild(technique);
		}
		meshRoot->LinkEndChild(sourceUV);

		printf("Creating Geometry UVs 2...\n");
		
		TiXmlElement *sourceUV2 = new TiXmlElement("source");
		sourceUV2->SetAttribute("id", name+"-geometry-uv2");
		{
			// Float array with positions
			TiXmlElement *floatArray = new TiXmlElement("float_array");
			floatArray->SetAttribute("id", name+"-geometry-uv2-array");
			floatArray->SetAttribute("count", ToString(base_uvs_2.size()*2));

			string farray_string="";
			for (size_t i=0; i<base_uvs_2.size(); i++) {
				farray_string+=ToString(base_uvs_2[i].x)+" ";
				farray_string+=ToString(1.0 - base_uvs_2[i].y)+" ";
			}

			TiXmlText *text = new TiXmlText(farray_string);
			floatArray->LinkEndChild(text);
			sourceUV2->LinkEndChild(floatArray);

			// Technique declaration
			TiXmlElement *technique = new TiXmlElement("technique_common");
			{
				TiXmlElement *accessor = new TiXmlElement("accessor");
				accessor->SetAttribute("source", "#"+name+"-geometry-uv2-array");
				accessor->SetAttribute("count", ToString(base_uvs_2.size()));
				accessor->SetAttribute("stride", "2");
				{
					TiXmlElement *xparam = new TiXmlElement("param");
					TiXmlElement *yparam = new TiXmlElement("param");

					xparam->SetAttribute("name", "S");
					yparam->SetAttribute("name", "T");

					xparam->SetAttribute("type", "float");
					yparam->SetAttribute("type", "float");

					accessor->LinkEndChild(xparam);
					accessor->LinkEndChild(yparam);
				}

				technique->LinkEndChild(accessor);
			}

			sourceUV2->LinkEndChild(technique);
		}
		meshRoot->LinkEndChild(sourceUV2);

		printf("Creating Geometry Colors...\n");
		
		TiXmlElement *sourceColor = new TiXmlElement("source");
		sourceColor->SetAttribute("id", name+"-geometry-color");
		{
			// Float array with positions
			TiXmlElement *floatArray = new TiXmlElement("float_array");
			floatArray->SetAttribute("id", name+"-geometry-color-array");
			floatArray->SetAttribute("count", ToString(base_colors.size()*4));

			string farray_string="";
			for (size_t i=0; i<base_colors.size(); i++) {
				farray_string+=ToString(base_colors[i].r)+" ";
				farray_string+=ToString(base_colors[i].g)+" ";
				farray_string+=ToString(base_colors[i].b)+" ";
				farray_string+=ToString(base_colors[i].a)+" ";
			}

			TiXmlText *text = new TiXmlText(farray_string);
			floatArray->LinkEndChild(text);
			sourceColor->LinkEndChild(floatArray);

			// Technique declaration
			TiXmlElement *technique = new TiXmlElement("technique_common");
			{
				TiXmlElement *accessor = new TiXmlElement("accessor");
				accessor->SetAttribute("source", "#"+name+"-geometry-color-array");
				accessor->SetAttribute("count", ToString(base_colors.size()));
				accessor->SetAttribute("stride", "4");
				{
					TiXmlElement *rparam = new TiXmlElement("param");
					TiXmlElement *gparam = new TiXmlElement("param");
					TiXmlElement *bparam = new TiXmlElement("param");
					TiXmlElement *aparam = new TiXmlElement("param");

					rparam->SetAttribute("name", "R");
					rparam->SetAttribute("type", "float");

					gparam->SetAttribute("name", "G");
					gparam->SetAttribute("type", "float");

					bparam->SetAttribute("name", "B");
					bparam->SetAttribute("type", "float");

					aparam->SetAttribute("name", "A");
					aparam->SetAttribute("type", "float");

					accessor->LinkEndChild(rparam);
					accessor->LinkEndChild(gparam);
					accessor->LinkEndChild(bparam);
					accessor->LinkEndChild(aparam);
				}

				technique->LinkEndChild(accessor);
			}

			sourceColor->LinkEndChild(technique);
		}
		meshRoot->LinkEndChild(sourceColor);


		printf("Creating Vertex Declaration...\n");

		// Vertex Decl
		TiXmlElement *sourceVertex = new TiXmlElement("vertices");
		sourceVertex->SetAttribute("id", name+"-geometry-vertex");

		TiXmlElement *inputVertex = new TiXmlElement("input");
		inputVertex->SetAttribute("semantic", "POSITION");
		inputVertex->SetAttribute("source", "#"+name+"-geometry-position");
		sourceVertex->LinkEndChild(inputVertex);
		meshRoot->LinkEndChild(sourceVertex);

		printf("Creating Faces...\n");
		global_index=0;
		for (size_t m=0; m<meshes.size(); m++) {
			for (size_t s=0; s<meshes[m]->submeshes.size(); s++) {
				TiXmlElement *trianglesRoot = new TiXmlElement("triangles");
				unsigned int mat_index = meshes[m]->submeshes[s]->material_index;
				string mat_name = name + ToString(mat_index);
				trianglesRoot->SetAttribute("material", mat_name);
				{
					inputVertex = new TiXmlElement("input");
					inputVertex->SetAttribute("semantic", "VERTEX");
					inputVertex->SetAttribute("source", "#"+name+"-geometry-vertex");
					inputVertex->SetAttribute("offset", "0");
					trianglesRoot->LinkEndChild(inputVertex);

					inputVertex = new TiXmlElement("input");
					inputVertex->SetAttribute("semantic", "NORMAL");
					inputVertex->SetAttribute("source", "#"+name+"-geometry-normal");
					inputVertex->SetAttribute("offset", "1");
					trianglesRoot->LinkEndChild(inputVertex);

					inputVertex = new TiXmlElement("input");
					inputVertex->SetAttribute("semantic", "TEXCOORD");
					inputVertex->SetAttribute("source", "#"+name+"-geometry-uv");
					inputVertex->SetAttribute("offset", "2");
					inputVertex->SetAttribute("set", "0");
					trianglesRoot->LinkEndChild(inputVertex);

					inputVertex = new TiXmlElement("input");
					inputVertex->SetAttribute("semantic", "TEXCOORD");
					inputVertex->SetAttribute("source", "#"+name+"-geometry-uv2");
					inputVertex->SetAttribute("offset", "3");
					inputVertex->SetAttribute("set", "1");
					trianglesRoot->LinkEndChild(inputVertex);

					inputVertex = new TiXmlElement("input");
					inputVertex->SetAttribute("semantic", "COLOR");
					inputVertex->SetAttribute("source", "#"+name+"-geometry-color");
					inputVertex->SetAttribute("offset", "4");
					inputVertex->SetAttribute("set", "0");
					trianglesRoot->LinkEndChild(inputVertex);

					string pfaces_str="";
					if (file_mode == MODE_GNO) {
						unsigned int polygon_index=meshes[m]->submeshes[s]->indices_index;
						size_t sz=polygon_tables[polygon_index]->faces.size();

						trianglesRoot->SetAttribute("count", ToString(sz));

						unsigned int positions_offset=0;
						unsigned int normals_offset=1;
						unsigned int uvs_offset=1;
						unsigned int uvs_2_offset=1;
						unsigned int colors_offset=1;

						size_t v_index=meshes[m]->submeshes[s]->vertex_index;
						for (size_t x=0; x<v_index; x++) {
							positions_offset += vertex_resource_tables[x]->positions.size();
							normals_offset   += vertex_resource_tables[x]->normals.size();
							uvs_offset       += vertex_resource_tables[x]->uvs.size();
							uvs_2_offset     += vertex_resource_tables[x]->uvs_2.size();
							colors_offset    += vertex_resource_tables[x]->colors.size();
						}

						

						for (size_t i=0; i<sz; i++) {
							for (size_t p=0; p<3; p++) {
								pfaces_str += ToString(polygon_tables[polygon_index]->faces[i]->points[p].position_index + positions_offset) + " ";
								
								if (vertex_resource_tables[v_index]->normals.size()) pfaces_str += ToString(polygon_tables[polygon_index]->faces[i]->points[p].normal_index + normals_offset)   + " ";
								else pfaces_str += "0 ";

								if (vertex_resource_tables[v_index]->uvs.size())     pfaces_str += ToString(polygon_tables[polygon_index]->faces[i]->points[p].uv_index     + uvs_offset)       + " ";
								else pfaces_str += "0 ";

								if (vertex_resource_tables[v_index]->uvs_2.size())   pfaces_str += ToString(polygon_tables[polygon_index]->faces[i]->points[p].uv2_index    + uvs_2_offset)     + " ";
								else pfaces_str += "0 ";

								if (vertex_resource_tables[v_index]->colors.size())  pfaces_str += ToString(polygon_tables[polygon_index]->faces[i]->points[p].color_index  + colors_offset)    + " ";
								else pfaces_str += "0 ";
							}
						}
					}
					else {
						int sz=index_tables[meshes[m]->submeshes[s]->indices_index]->indices_vector.size();

						trianglesRoot->SetAttribute("count", ToString(sz));
						for (size_t i=global_index; i<global_index+sz; i++) {
							pfaces_str += ToString(pfaces[base_indices[i].x*5]) + " ";
							pfaces_str += ToString(pfaces[base_indices[i].x*5 + 1]) + " ";
							pfaces_str += ToString(pfaces[base_indices[i].x*5 + 2]) + " ";
							pfaces_str += ToString(pfaces[base_indices[i].x*5 + 3]) + " ";
							pfaces_str += ToString(pfaces[base_indices[i].x*5 + 4]) + " ";
							pfaces_str += ToString(pfaces[base_indices[i].y*5]) + " ";
							pfaces_str += ToString(pfaces[base_indices[i].y*5 + 1]) + " ";
							pfaces_str += ToString(pfaces[base_indices[i].y*5 + 2]) + " ";
							pfaces_str += ToString(pfaces[base_indices[i].y*5 + 3]) + " ";
							pfaces_str += ToString(pfaces[base_indices[i].y*5 + 4]) + " ";
							pfaces_str += ToString(pfaces[base_indices[i].z*5]) + " ";
							pfaces_str += ToString(pfaces[base_indices[i].z*5 + 1]) + " ";
							pfaces_str += ToString(pfaces[base_indices[i].z*5 + 2]) + " ";
							pfaces_str += ToString(pfaces[base_indices[i].z*5 + 3]) + " ";
							pfaces_str += ToString(pfaces[base_indices[i].z*5 + 4]) + " ";
						}

						global_index+=sz;
					}

					TiXmlElement *pFaces = new TiXmlElement("p");
					TiXmlText *text = new TiXmlText(pfaces_str);
					pFaces->LinkEndChild(text);
					trianglesRoot->LinkEndChild(pFaces);
				}

				meshRoot->LinkEndChild(trianglesRoot);
			}
		}
		
		root->LinkEndChild(geometryRoot);

		printf("Finished with geometry...\n");
	}

	void SonicXNObject::writeDAE(TiXmlElement *root, bool only_bones, float unit_scale) {
		printf("Writing bones...\n");
		
		// Create bone nodes via recursive method
		for (size_t i=0; i<bones.size(); i++) {
			if (bones[i]->parent_index > bones.size()) {
				writeBonesDAE(root, i, unit_scale);
			}
		}

		if (only_bones) return;

		printf("Writing node name...\n");

		TiXmlElement *nodeRoot = new TiXmlElement("node");
		nodeRoot->SetAttribute("id", name);
		nodeRoot->SetAttribute("sid", name);
		nodeRoot->SetAttribute("name", name);

		if (bones.size()) {
			TiXmlElement *instance = new TiXmlElement("instance_controller");
			instance->SetAttribute("url", "#"+name+"-controller");
			writeMaterialBindDAE(instance);
			nodeRoot->LinkEndChild(instance);
			root->LinkEndChild(nodeRoot);
			return;
		}
		
		printf("Writing instanced geometry with material binds...\n");
		TiXmlElement *instanceGeometry = new TiXmlElement("instance_geometry");
		instanceGeometry->SetAttribute("url", "#"+name+"-geometry");
		writeMaterialBindDAE(instanceGeometry);
		nodeRoot->LinkEndChild(instanceGeometry);

		printf("Writing translation matrix...\n");
		TiXmlElement *matrixNode = new TiXmlElement("matrix");
		string matrix_str="1.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0 1.0";
		TiXmlText *text = new TiXmlText(matrix_str);
		matrixNode->LinkEndChild(text);
		nodeRoot->LinkEndChild(matrixNode);

		root->LinkEndChild(nodeRoot);
	}

	void SonicXNObject::writeMaterialDAE(TiXmlElement *root) {
		for (size_t i=0; i<material_tables.size(); i++) {
			string mat_name = name+ToString(i);

			TiXmlElement *materialNode = new TiXmlElement("material");
			materialNode->SetAttribute("id", mat_name+"ID");
			materialNode->SetAttribute("name", mat_name);


			TiXmlElement *newElem = new TiXmlElement("instance_effect");
			newElem->SetAttribute("url", "#"+mat_name+"-effect");
			materialNode->LinkEndChild(newElem);

			root->LinkEndChild(materialNode);
		}

		for (size_t i=0; i<old_material_tables.size(); i++) {
			string mat_name = name+ToString(i);

			TiXmlElement *materialNode = new TiXmlElement("material");
			materialNode->SetAttribute("id", mat_name+"ID");
			materialNode->SetAttribute("name", mat_name);


			TiXmlElement *newElem = new TiXmlElement("instance_effect");
			newElem->SetAttribute("url", "#"+mat_name+"-effect");
			materialNode->LinkEndChild(newElem);

			root->LinkEndChild(materialNode);
		}
	}


	void SonicXNObject::writeMaterialBindDAE(TiXmlElement *root) {
		TiXmlElement *bindMaterialRoot = new TiXmlElement("bind_material");
		TiXmlElement *techniqueRoot = new TiXmlElement("technique_common");
	
		for (size_t i=0; i<material_tables.size(); i++) {
			string mat_name=name+ToString(i);

			TiXmlElement *instanceMaterial = new TiXmlElement("instance_material");
			instanceMaterial->SetAttribute("symbol", mat_name);
			instanceMaterial->SetAttribute("target", "#"+mat_name+"ID");
			techniqueRoot->LinkEndChild(instanceMaterial);
		}

		for (size_t i=0; i<old_material_tables.size(); i++) {
			string mat_name=name+ToString(i);

			TiXmlElement *instanceMaterial = new TiXmlElement("instance_material");
			instanceMaterial->SetAttribute("symbol", mat_name);
			instanceMaterial->SetAttribute("target", "#"+mat_name+"ID");
			techniqueRoot->LinkEndChild(instanceMaterial);
		}
	
		bindMaterialRoot->LinkEndChild(techniqueRoot);
		root->LinkEndChild(bindMaterialRoot);
	}


	void SonicXNObject::writeEffectTextureDAE(TiXmlElement *root, string tex_name) {
		TiXmlElement *newParam = new TiXmlElement("newparam");
		newParam->SetAttribute("sid", tex_name+"-image-surface");

		{
			TiXmlElement *surface = new TiXmlElement("surface");
			surface->SetAttribute("type", "2D");
			{
				TiXmlElement *newElem = new TiXmlElement("init_from");
				TiXmlText *text = new TiXmlText(tex_name+"-image");
				newElem->LinkEndChild(text);
				surface->LinkEndChild(newElem);
			}

			newParam->LinkEndChild(surface);
		}
		root->LinkEndChild(newParam);


		newParam = new TiXmlElement("newparam");
		newParam->SetAttribute("sid", tex_name+"-image-sampler");
		{
			TiXmlElement *sampler = new TiXmlElement("sampler2D");
			{
				TiXmlElement *newElem = new TiXmlElement("source");
				TiXmlText *text = new TiXmlText(tex_name+"-image-surface");
				newElem->LinkEndChild(text);
				sampler->LinkEndChild(newElem);
			}

			newParam->LinkEndChild(sampler);
		}
		root->LinkEndChild(newParam);
	}

	void SonicXNObject::writeEffectTechniqueDAE(TiXmlElement *root, string tex_name) {
		TiXmlElement *techniqueNode = new TiXmlElement("technique");
		techniqueNode->SetAttribute("sid", "COMMON");

		TiXmlElement *phongNode = new TiXmlElement("phong");
		{
			// Diffuse parameter for the first texture
			TiXmlElement *newElem = new TiXmlElement("diffuse");
			TiXmlElement *textureElem = new TiXmlElement("texture");
			textureElem->SetAttribute("texture", tex_name+"-image-sampler");
			textureElem->SetAttribute("texcoord", "UVSET0");

			TiXmlElement *colorElem = new TiXmlElement("color");
			TiXmlText *text = new TiXmlText("1.000000 1.000000 1.000000 1");
			colorElem->LinkEndChild(text);

			newElem->LinkEndChild(textureElem);
			newElem->LinkEndChild(colorElem);
			phongNode->LinkEndChild(newElem);
		}
		techniqueNode->LinkEndChild(phongNode);

		root->LinkEndChild(techniqueNode);
	}

	void SonicXNObject::writeEffectsDAE(TiXmlElement *root, SonicXNTexture *texture) {
		for (size_t m=0; m<material_tables.size(); m++) {
			string mat_name = name+ToString(m);

			TiXmlElement *effectNode = new TiXmlElement("effect");
			effectNode->SetAttribute("id", mat_name+"-effect");
			effectNode->SetAttribute("name", mat_name+"-effect");

			TiXmlElement *profileNode = new TiXmlElement("profile_COMMON");
			{
				if (file_mode == MODE_ZNO) {
					vector<SonicTextureUnitZNO *> texture_units_zno=material_tables[m]->texture_units_zno;

					for (size_t i=0; i<texture_units_zno.size(); i++) {
						string tex_name=texture->getTexture(texture_units_zno[i]->index);
						writeEffectTextureDAE(profileNode, tex_name);
					}
					
					if (texture_units_zno.size()) {
						string tex_name=texture->getTexture(texture_units_zno[0]->index);
						writeEffectTechniqueDAE(profileNode, tex_name);
					}
				}
				else {
					vector<SonicTextureUnit *> texture_units=material_tables[m]->texture_units;

					for (size_t i=0; i<texture_units.size(); i++) {
						string tex_name=texture->getTexture(texture_units[i]->index);
						writeEffectTextureDAE(profileNode, tex_name);
					}

					if (texture_units.size()) {
						string tex_name=texture->getTexture(texture_units[0]->index);
						writeEffectTechniqueDAE(profileNode, tex_name);
					}
				}
			}
			effectNode->LinkEndChild(profileNode);
			root->LinkEndChild(effectNode);
		}

		for (size_t m=0; m<old_material_tables.size(); m++) {
			string mat_name = name+ToString(m);

			TiXmlElement *effectNode = new TiXmlElement("effect");
			effectNode->SetAttribute("id", mat_name+"-effect");
			effectNode->SetAttribute("name", mat_name+"-effect");

			TiXmlElement *profileNode = new TiXmlElement("profile_COMMON");
			{
				unsigned int texture_unit=old_material_tables[m]->texture_unit;
				printf("Effect's texture unit is %d...\n", texture_unit);

				string tex_name=texture->getTexture(texture_unit);

				size_t pos=tex_name.find(".gvr");
				if (pos == string::npos) pos=tex_name.find(".GVR");
				if (pos != string::npos) {
					std::transform(tex_name.begin(), tex_name.end(), tex_name.begin(), ::tolower);
					tex_name.replace(pos, 4, ".png");
				}

				printf("Writing effect's texture with texture name %s...\n", tex_name.c_str());
				writeEffectTextureDAE(profileNode, tex_name);

				printf("Writing effect's technique with texture name %s...\n", tex_name.c_str());
				writeEffectTechniqueDAE(profileNode, tex_name);
			}
			effectNode->LinkEndChild(profileNode);
			root->LinkEndChild(effectNode);
		}
	}


	void SonicXNMotion::writeDAE(TiXmlElement *root, SonicXNObject *object, SonicXNBones *bones, float unit_scale) {
		unsigned int frame_length_i=(int)end_frame;

		for (size_t b=0; b<object->bones.size(); b++) {
			SonicMotionControl *pos_motion_control = getPositionMotionControl(b);
			SonicMotionControl *rot_motion_control = getAnglesMotionControl(b);
			SonicMotionControl *pos_x_motion_control = getPositionXMotionControl(b);
			SonicMotionControl *pos_y_motion_control = getPositionYMotionControl(b);
			SonicMotionControl *pos_z_motion_control = getPositionZMotionControl(b);
			SonicMotionControl *rot_x_motion_control = getAngleXMotionControl(b);
			SonicMotionControl *rot_y_motion_control = getAngleYMotionControl(b);
			SonicMotionControl *rot_z_motion_control = getAngleZMotionControl(b);
			SonicMotionControl *rot_beta_x_motion_control = getAngleBetaXMotionControl(b);
			SonicMotionControl *rot_beta_y_motion_control = getAngleBetaYMotionControl(b);
			SonicMotionControl *rot_beta_z_motion_control = getAngleBetaZMotionControl(b);
			SonicMotionControl *sca_x_motion_control = getScaleXMotionControl(b);
			SonicMotionControl *sca_y_motion_control = getScaleYMotionControl(b);
			SonicMotionControl *sca_z_motion_control = getScaleZMotionControl(b);

			TiXmlElement *animationNode = new TiXmlElement("animation");
			string bone_name=object->name+ToString(b);
			if (bones) bone_name=bones->getName(b);

			animationNode->SetAttribute("id", bone_name+"-anim");
			animationNode->SetAttribute("name", bone_name);

			TiXmlElement *animationSubNode = new TiXmlElement("animation");
			{
				TiXmlElement *matrixAnimationInputNode = new TiXmlElement("source");
				matrixAnimationInputNode->SetAttribute("id", bone_name+"-Matrix-animation-input");
				{
					TiXmlElement *matrixAnimationInputArrayNode = new TiXmlElement("float_array");
					matrixAnimationInputArrayNode->SetAttribute("id", bone_name+"-Matrix-animation-input-array");
					matrixAnimationInputArrayNode->SetAttribute("count", frame_length_i);
					string text="";
					for (size_t i=0; i<frame_length_i; i++) {
						text += ToString((float)i/30.0) + " ";
					}
					TiXmlText *pValue=new TiXmlText(text);
					matrixAnimationInputArrayNode->LinkEndChild(pValue);
					matrixAnimationInputNode->LinkEndChild(matrixAnimationInputArrayNode);


					TiXmlElement *techniqueCommonNode = new TiXmlElement("technique_common");
					TiXmlElement *accessorNode = new TiXmlElement("accessor");
					accessorNode->SetAttribute("source", "#"+bone_name+"-Matrix-animation-input-array");
					accessorNode->SetAttribute("count", frame_length_i);
					TiXmlElement *paramNode = new TiXmlElement("param");
					paramNode->SetAttribute("name", "TIME");
					paramNode->SetAttribute("type", "float");
					accessorNode->LinkEndChild(paramNode);
					techniqueCommonNode->LinkEndChild(accessorNode);
					matrixAnimationInputNode->LinkEndChild(techniqueCommonNode);
				}
				animationSubNode->LinkEndChild(matrixAnimationInputNode);

				TiXmlElement *matrixAnimationOutputTransformNode = new TiXmlElement("source");
				matrixAnimationOutputTransformNode->SetAttribute("id", bone_name+"-Matrix-animation-output-transform");
				{
					TiXmlElement *matrixAnimationOutputArrayNode = new TiXmlElement("float_array");
					matrixAnimationOutputArrayNode->SetAttribute("id", bone_name+"-Matrix-animation-output-transform-array");
					matrixAnimationOutputArrayNode->SetAttribute("count", frame_length_i*16);

					SonicBone *bone=object->bones[b];

					// Set frame initial values
					Vector3 position = bone->translation;
					Vector3 scale = bone->scale;
					Quaternion orientation = bone->orientation;

					Matrix3 mr;
					float bone_rot_x, bone_rot_y, bone_rot_z;
					unsigned short rot_x_ref, rot_y_ref, rot_z_ref;
					unsigned int rotation_flag=object->bones[b]->flag & 3840u;

					bone_rot_x = bone->rotation_x * LIBGENS_MATH_INT32_TO_RAD;
					bone_rot_y = bone->rotation_y * LIBGENS_MATH_INT32_TO_RAD;
					bone_rot_z = bone->rotation_z * LIBGENS_MATH_INT32_TO_RAD;

					rot_x_ref = (bone_rot_x / LIBGENS_MATH_PI * 32767.5f);
					rot_y_ref = (bone_rot_y / LIBGENS_MATH_PI * 32767.5f);
					rot_z_ref = (bone_rot_z / LIBGENS_MATH_PI * 32767.5f);

					string text="";
					for (size_t i=0; i<frame_length_i; i++) {
						float rot_x=bone_rot_x;
						float rot_y=bone_rot_y;
						float rot_z=bone_rot_z;
						
						if (pos_x_motion_control) position.x = pos_x_motion_control->getFrameValue(i, position.x);
						if (pos_y_motion_control) position.y = pos_y_motion_control->getFrameValue(i, position.y);
						if (pos_z_motion_control) position.z = pos_z_motion_control->getFrameValue(i, position.z);

						if (pos_motion_control) {
							position = pos_motion_control->getFrameVector(i, position);
						}

						bool update_quaternion=false;
						if (rot_x_motion_control) {
							rot_x = rot_x_motion_control->getFrameValue(i, rot_x_ref) / 65535.0f * (LIBGENS_MATH_PI*2);
							update_quaternion = true;
						}

						if (rot_y_motion_control) {
							rot_y = rot_y_motion_control->getFrameValue(i, rot_y_ref) / 65535.0f * (LIBGENS_MATH_PI*2);
							update_quaternion = true;
						}

						if (rot_z_motion_control) {
							rot_z = rot_z_motion_control->getFrameValue(i, rot_z_ref) / 65535.0f * (LIBGENS_MATH_PI*2);
							update_quaternion = true;
						}

						if (rot_beta_x_motion_control) {
							rot_x = rot_beta_x_motion_control->getFrameValue(i, rot_x_ref) / 65535.0f * (LIBGENS_MATH_PI*2);
							update_quaternion = true;
						}

						if (rot_beta_y_motion_control) {
							rot_y = rot_beta_y_motion_control->getFrameValue(i, rot_y_ref) / 65535.0f * (LIBGENS_MATH_PI*2);
							update_quaternion = true;
						}

						if (rot_beta_z_motion_control) {
							rot_z = rot_beta_z_motion_control->getFrameValue(i, rot_z_ref) / 65535.0f * (LIBGENS_MATH_PI*2);
							update_quaternion = true;
						}

						if (rot_motion_control) {
							Vector3 rotation_vector = rot_motion_control->getFrameVector(i, Vector3(rot_x, rot_y, rot_z));
							rot_x = rotation_vector.x  / 65535.0f * (LIBGENS_MATH_PI*2);
							rot_y = rotation_vector.y  / 65535.0f * (LIBGENS_MATH_PI*2);
							rot_z = rotation_vector.z  / 65535.0f * (LIBGENS_MATH_PI*2);
							update_quaternion = true;
						}
						
						if (sca_x_motion_control) scale.x    = sca_x_motion_control->getFrameValue(i, scale.x);
						if (sca_y_motion_control) scale.y    = sca_y_motion_control->getFrameValue(i, scale.y);
						if (sca_z_motion_control) scale.z    = sca_z_motion_control->getFrameValue(i, scale.z);
						
						if (update_quaternion) {
							if (rotation_flag != 0u) {
								if (rotation_flag != 256u) {
									if (rotation_flag != 1024u) mr.fromEulerAnglesZYX(rot_z, rot_y, rot_x);
									else mr.fromEulerAnglesYXZ(rot_z, rot_y, rot_x);
								}
								else mr.fromEulerAnglesYZX(rot_z, rot_y, rot_x);
							}
							else mr.fromEulerAnglesZYX(rot_z, rot_y, rot_x);

							orientation.fromRotationMatrix(mr);
						}
						

						Matrix4 m;
						m.makeTransform(position*unit_scale, scale, orientation);
						
						for (size_t x=0; x<4; x++) {
							for (size_t y=0; y<4; y++) {
								text += ToString(m[x][y]) + " ";
							}
						}
					}

					TiXmlText *pValue=new TiXmlText(text);
					matrixAnimationOutputArrayNode->LinkEndChild(pValue);
					matrixAnimationOutputTransformNode->LinkEndChild(matrixAnimationOutputArrayNode);


					TiXmlElement *techniqueCommonNode = new TiXmlElement("technique_common");
					TiXmlElement *accessorNode = new TiXmlElement("accessor");
					accessorNode->SetAttribute("source", "#"+bone_name+"-Matrix-animation-output-transform-array");
					accessorNode->SetAttribute("count", frame_length_i);
					accessorNode->SetAttribute("stride", 16);
					TiXmlElement *paramNode = new TiXmlElement("param");
					paramNode->SetAttribute("type", "float4x4");
					accessorNode->LinkEndChild(paramNode);
					techniqueCommonNode->LinkEndChild(accessorNode);
					matrixAnimationOutputTransformNode->LinkEndChild(techniqueCommonNode);
				}
				animationSubNode->LinkEndChild(matrixAnimationOutputTransformNode);

				TiXmlElement *interpolationsNode = new TiXmlElement("source");
				interpolationsNode->SetAttribute("id", bone_name+"-Interpolations");
				{
					TiXmlElement *interpolationsArrayNode = new TiXmlElement("Name_array");
					interpolationsArrayNode->SetAttribute("id", bone_name+"-Interpolations-array");
					interpolationsArrayNode->SetAttribute("count", frame_length_i);

					string text="";
					for (size_t i=0; i<frame_length_i; i++) {
						text += "LINEAR ";
					}
					TiXmlText *pValue=new TiXmlText(text);
					interpolationsArrayNode->LinkEndChild(pValue);
					interpolationsNode->LinkEndChild(interpolationsArrayNode);


					TiXmlElement *techniqueCommonNode = new TiXmlElement("technique_common");
					TiXmlElement *accessorNode = new TiXmlElement("accessor");
					accessorNode->SetAttribute("source", "#"+bone_name+"-Interpolations-array");
					accessorNode->SetAttribute("count", frame_length_i);
					TiXmlElement *paramNode = new TiXmlElement("param");
					paramNode->SetAttribute("type", "name");
					accessorNode->LinkEndChild(paramNode);
					techniqueCommonNode->LinkEndChild(accessorNode);
					interpolationsNode->LinkEndChild(techniqueCommonNode);
				}
				animationSubNode->LinkEndChild(interpolationsNode);

				TiXmlElement *matrixAnimationTransformNode = new TiXmlElement("sampler");
				matrixAnimationTransformNode->SetAttribute("id", bone_name+"-Matrix-animation-transform");
				{
					TiXmlElement *input = new TiXmlElement("input");
					TiXmlElement *output = new TiXmlElement("input");
					TiXmlElement *interpolation = new TiXmlElement("input");

					input->SetAttribute("semantic", "INPUT");
					output->SetAttribute("semantic", "OUTPUT");
					interpolation->SetAttribute("semantic", "INTERPOLATION");

					input->SetAttribute("source", "#"+bone_name+"-Matrix-animation-input");
					output->SetAttribute("source", "#"+bone_name+"-Matrix-animation-output-transform");
					interpolation->SetAttribute("source", "#"+bone_name+"-Interpolations");

					matrixAnimationTransformNode->LinkEndChild(input);
					matrixAnimationTransformNode->LinkEndChild(output);
					matrixAnimationTransformNode->LinkEndChild(interpolation);
				}

				animationSubNode->LinkEndChild(matrixAnimationTransformNode);

				TiXmlElement *channelNode = new TiXmlElement("channel");
				channelNode->SetAttribute("source", "#"+bone_name+"-Matrix-animation-transform");
				channelNode->SetAttribute("target", bone_name+"/matrix");
				animationSubNode->LinkEndChild(channelNode);
			}
			animationNode->LinkEndChild(animationSubNode);

			root->LinkEndChild(animationNode);
		}
	}
}
