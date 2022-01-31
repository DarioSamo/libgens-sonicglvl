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
	const unsigned char xno_constant_floats[80] = { 
										   0x00 ,0x00 ,0x80 ,0x3F ,0x00 ,0x00 ,0x80 ,0x3F ,0x00 ,0x00 ,0x80 ,0x3F ,0x00 ,0x00 ,0x80 ,0x3F 
										  ,0x00 ,0x00 ,0x80 ,0x3F ,0x00 ,0x00 ,0x80 ,0x3F ,0x00 ,0x00 ,0x80 ,0x3F ,0x00 ,0x00 ,0x80 ,0x3F 
										  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x80 ,0x3F 
										  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x80 ,0x3F 
										  ,0xFF ,0xFF ,0xFF ,0x40 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 
										 };

	const unsigned char xno_constant_ints[64] =   { 
										   0x01 ,0x00 ,0x00 ,0x00 ,0x02 ,0x03 ,0x00 ,0x00 ,0x03 ,0x03 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 
										  ,0x06 ,0x80 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,0x00 ,0x00 ,0x04 ,0x02 ,0x00 ,0x00 
										  ,0x00 ,0x00 ,0x00 ,0x00 ,0x01 ,0x00 ,0x00 ,0x00 ,0x03 ,0x02 ,0x00 ,0x00 ,0x01 ,0x00 ,0x00 ,0x00 
										  ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 
										 };


	void SonicXNFile::importFBX(FBX *fbx) {
		FbxScene *lScene = fbx->getScene();
		if (!lScene) return;

		// Add Bone to Object
		SonicXNObject *object = getObject();
		if (object) {
			SonicBone *sonic_bone=new SonicBone();
			sonic_bone->zero();
			object->bones.push_back(sonic_bone);

			object->aabb.reset();
		}

		// This function show how to cycle through scene elements in a linear way.
		const int lNodeCount = lScene->GetSrcObjectCount<FbxNode>();
		FbxStatus lStatus;

		for (int lIndex=0; lIndex<lNodeCount; lIndex++) {
			FbxNode *lNode = lScene->GetSrcObject<FbxNode>(lIndex);
			printf("Node found: %s\n", lNode->GetName());
			addFBXNode(lNode);
		}

		// Build Center and Radius
		if (object) {
			object->center = object->aabb.center();
			object->radius = object->aabb.radius();
		}
	}


	void SonicXNFile::addFBXNode(FbxNode *lNode) {
		if (!lNode) return;

		FbxMesh *lMesh = lNode->GetMesh();
		if (!lMesh) return;

		SonicXNObject  *object=getObject();
		if (!object) return;

		SonicXNTexture *texture=getTexture();

		FbxAMatrix transform_matrix=lNode->EvaluateGlobalTransform();

		// Convert and add materials
		int material_base_index = object->material_tables.size();
		int material_count = lNode->GetMaterialCount();
		for (int m=0; m<material_count; m++) {
			FbxSurfaceMaterial *lMaterial=lNode->GetMaterial(m);
			addFBXMaterial(lMaterial);
		}

		// Build Sonic Mesh
		SonicMesh *solid_sonic_mesh = new SonicMesh();
		solid_sonic_mesh->flag = 513;

		// Add Submeshes to Sonic Mesh
		for (int m=0; m<material_count; m++) {
			addFBXSubmesh(lNode, lMesh, solid_sonic_mesh, m, material_base_index, material_count == 1 ? true : false, transform_matrix);
		}

		// Add extras to Sonic Mesh
		if (texture) {
			for (size_t i=0; i<texture->getTextureUnitsSize(); i++) {
				solid_sonic_mesh->extras.push_back(i);
			}
		}
		
		if (solid_sonic_mesh->submeshes.size()) object->meshes.push_back(solid_sonic_mesh);
	}

	void SonicXNFile::addFBXSubmesh(FbxNode *lNode, FbxMesh *lMesh, SonicMesh *sonic_mesh, int material_index, int material_base_index, bool single_material, FbxAMatrix transform_matrix) {
		SonicXNObject  *object=getObject();
		if (!object) return;

		SonicXNEffect *effect = getEffect();

		FbxAMatrix rotation_matrix = transform_matrix;
		rotation_matrix.SetT(FbxVector4(0.0, 0.0, 0.0, 0.0));
		rotation_matrix.SetS(FbxVector4(1.0, 1.0, 1.0, 1.0));

		// Set up Object
		unsigned int sonic_material_index = material_index;
		vector<SonicVertex *> new_vertices;
		vector<unsigned int> new_indices;

		// Scan FBX Mesh for vertices on the current material index
		int lPolygonCount=lMesh->GetPolygonCount();
		FbxLayerElementArrayTemplate<int>* lMaterialIndice = NULL;

		bool add=false;
		if (!single_material) {
			FbxGeometryElement::EMappingMode lMaterialMappingMode = FbxGeometryElement::eNone;
			if (lMesh->GetElementMaterial()) {
				lMaterialIndice = &lMesh->GetElementMaterial()->GetIndexArray();
				lMaterialMappingMode = lMesh->GetElementMaterial()->GetMappingMode();


				if (!lMaterialIndice) printf("Mesh has no material index element.\n");

				if (lMaterialIndice && lMaterialMappingMode == FbxGeometryElement::eByPolygon) {
					if (lMaterialIndice->GetCount() == lPolygonCount) {
						add = true;
					}
				}
			}
		}
		else add = true;
		
		if (add) {
			int control_points_count=lMesh->GetControlPointsCount();
			FbxVector4 *control_points=lMesh->GetControlPoints();
			int vertex_color_count = lMesh->GetElementVertexColorCount();

			for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex) {
				if (lMaterialIndice) {
					const int lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
					if (lMaterialIndex != material_index) continue;
				}

				int polygon_size=lMesh->GetPolygonSize(lPolygonIndex);
				if (polygon_size == 3) {
					Vector3 face;

					for (int j=0; j<polygon_size; j++) {
						int control_point_index=lMesh->GetPolygonVertex(lPolygonIndex, j);

						FbxVector4 control_point=transform_matrix.MultT(control_points[control_point_index]);
						FbxVector4 normal;
						lMesh->GetPolygonVertexNormal(lPolygonIndex, j, normal);
						normal = rotation_matrix.MultT(normal);

						// Create Vertex
						SonicVertex *v=new SonicVertex();
						v->zero();
						v->position = Vector3(control_point[0], control_point[2], -control_point[1]);
						v->normal   = Vector3(normal[0], normal[2], -normal[1]);

						// Add to AABB
						object->aabb.addPoint(v->position);

						FbxStringList uv_sets;
						lMesh->GetUVSetNames(uv_sets);

						for (int set=0; set<uv_sets.GetCount(); set++) {
							if (set >= 4) break;
							FbxVector2 uv;
							bool no_uv;
							lMesh->GetPolygonVertexUV(lPolygonIndex, j, uv_sets[set].Buffer(), uv, no_uv);
							v->uv[set] = Vector2(uv[0], 1.0-uv[1]);
						}

						for (size_t c=0; c<vertex_color_count; c++) {
							FbxGeometryElementVertexColor* lVertexColor = lMesh->GetElementVertexColor(c);
							
							if ((lVertexColor->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) && (lVertexColor->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)) {
								FbxColor lColor = lVertexColor->GetDirectArray().GetAt(control_point_index);

								if (c == 0) {
									v->rgba[0] = (lColor.mRed * 255.0);
									v->rgba[1] = (lColor.mGreen * 255.0);
									v->rgba[2] = (lColor.mBlue * 255.0);
									v->rgba[3] = (lColor.mAlpha * 255.0);
								}

								if (c == 1) {
									v->rgba_2[0] = (lColor.mRed * 255.0);
									v->rgba_2[1] = (lColor.mGreen * 255.0);
									v->rgba_2[2] = (lColor.mBlue * 255.0);
									v->rgba_2[3] = (lColor.mAlpha * 255.0);
								}
							}
							else {
								printf("Vertex color mapping type unsupported for this FBX! Report!\n");
								getchar();
							}
						}
						
						// Calculate Tangent and Binormal
						Vector3 c1 = v->normal.crossProduct(Vector3(0.0, 0.0, 1.0));
						Vector3 c2 = v->normal.crossProduct(Vector3(0.0, 1.0, 0.0));
						if(c1.length() > c2.length()) v->tangent = c1;
						else v->tangent = c2;
						v->tangent.normalise();
						v->binormal = v->tangent.crossProduct(v->normal);

						// Detect for clones on current vertex list
						bool clone=false;
						for (unsigned int y=0; y<new_vertices.size(); y++) {
							if ((*new_vertices[y])==(*v)) {
								clone = true;
								new_indices.push_back(y);
								delete v;
								break;
							}
						}

						if (!clone) {
							new_vertices.push_back(v);
							new_indices.push_back(new_vertices.size()-1);
						}
					}
				}
				else printf("Unsupported polygon size.\n");
			}
		}
		

		vector< vector<SonicVertex *> > vertices_output;
		vector< vector<unsigned int> > indices_output;
		unsigned int max_skinning_bones=32;
		// Temporary Workaround for submesh splitting, just add existing tables
		vertices_output.push_back(new_vertices);
		indices_output.push_back(new_indices);

		for (size_t split=0; split<vertices_output.size(); split++){
			// Per submesh add a reference to the effect file based on the material names
			// Also add any new effect names
			if (effect) {
				FbxSurfaceMaterial *lMaterial=lNode->GetMaterial(material_index);

				if (lMaterial) {
					string material_name = ToString(lMaterial->GetName());

					string shader_name = "Billboard03.fx";
					string sub_shader_name = "Billboard03";

					size_t shader_name_pos = material_name.find_first_of("@");
					size_t sub_shader_name_pos = material_name.find_last_of("@");

					// One @ Symbol was found at least
					if (shader_name_pos != string::npos) {
						shader_name_pos += 1;
						sub_shader_name_pos += 1;

						// Verify if there's more than one @ symbol
						if (shader_name_pos != sub_shader_name_pos) {
							shader_name = material_name.substr(shader_name_pos, sub_shader_name_pos-shader_name_pos-1);
							sub_shader_name = material_name.substr(sub_shader_name_pos, material_name.size()-sub_shader_name_pos);
						}
						// There's only one @ symbol, auto-generate .fx name
						else {
							sub_shader_name = material_name.substr(shader_name_pos, material_name.size()-shader_name_pos);
							shader_name = sub_shader_name + ".fx";
						}
					}

					size_t material_effect_shader_index = effect->addMaterialShader(shader_name);
					size_t material_effect_index = effect->addMaterialName(sub_shader_name, material_effect_shader_index);
					effect->addExtra(material_effect_index);
				}
			}

			// Create submesh per split
			unsigned int sonic_vertex_index=object->vertex_tables.size();
			SonicVertexTable *sonic_vertex_table=new SonicVertexTable();
			sonic_vertex_table->vertices = vertices_output[split];
			sonic_vertex_table->vertex_size = 52;
			sonic_vertex_table->flag_1 = 0x01740B;
			sonic_vertex_table->flag_2 = 0x115A;
			sonic_vertex_table->bone_table.push_back(0); // FIXME: Fake Skinning
			object->vertex_tables.push_back(sonic_vertex_table);

			unsigned int sonic_index_index=object->index_tables.size();
			SonicIndexTable *sonic_index_table=new SonicIndexTable();
			sonic_index_table->flag = 0x4810;
			object->index_tables.push_back(sonic_index_table);

			triangle_stripper::indices tri_indices;
			for (size_t i=0; i<indices_output[split].size(); i+=3) {
				tri_indices.push_back(indices_output[split][i]);
				tri_indices.push_back(indices_output[split][i+1]);
				tri_indices.push_back(indices_output[split][i+2]);
			}

			triangle_stripper::tri_stripper stripper(tri_indices);
			stripper.SetCacheSize(0);
			stripper.SetBackwardSearch(false);
			triangle_stripper::primitive_vector out_vector;
			stripper.Strip(&out_vector);

			for (size_t i=0; i<out_vector.size(); i+=1) {
				if (out_vector[i].Type == triangle_stripper::TRIANGLE_STRIP) {
					for (size_t j=0; j<out_vector[i].Indices.size(); j++) {
						sonic_index_table->indices.push_back(out_vector[i].Indices[j]);
					}
					sonic_index_table->strip_sizes.push_back(out_vector[i].Indices.size());
				}
				else {
					for (size_t j=0; j<out_vector[i].Indices.size(); j+=3) {
						sonic_index_table->indices.push_back(out_vector[i].Indices[j]);
						sonic_index_table->indices.push_back(out_vector[i].Indices[j+1]);
						sonic_index_table->indices.push_back(out_vector[i].Indices[j+2]);
						sonic_index_table->strip_sizes.push_back(3);
					}
				}
			}

			SonicSubmesh *sonic_submesh=new SonicSubmesh();
			sonic_submesh->node_index   = 0x36;
			sonic_submesh->matrix_index = 0xFFFFFFFF;
			sonic_submesh->center.x = 0.0f;
			sonic_submesh->center.y = 0.0f;
			sonic_submesh->center.z = 0.0f;
			sonic_submesh->radius   = 0.0f;
			sonic_submesh->material_index  = sonic_material_index + material_base_index;
			sonic_submesh->vertex_index    = sonic_vertex_index;
			sonic_submesh->indices_index   = sonic_index_index;
			sonic_submesh->indices_index_2 = sonic_index_index;
			sonic_mesh->submeshes.push_back(sonic_submesh);
		}
	}


	void SonicXNFile::addFBXMaterialProperty(FbxProperty *lProperty, SonicMaterialTable *sonic_material_table) {
		SonicXNTexture *texture=getTexture();
		if (!texture) return;

		const int lTextureCount = lProperty->GetSrcObjectCount<FbxFileTexture>();
		if (lTextureCount) {
			const FbxFileTexture* lTexture = lProperty->GetSrcObject<FbxFileTexture>();
			if (lTexture) {
				string texture_name = File::nameFromFilename(ToString(lTexture->GetFileName()));

				size_t pos=texture_name.find(".png");
				if (pos == string::npos) pos=texture_name.find(".PNG");
				if (pos != string::npos) {
					texture_name.replace(pos, 4, ".dds");
				}

				unsigned int texture_index = texture->addTexture(texture_name);

				SonicTextureUnit *texture_unit = new SonicTextureUnit();
				texture_unit->flag_f = 2.187562f;
				texture_unit->index = texture_index;
				texture_unit->flag = 0x80000000;
				texture_unit->flag_2_f = 1.0f;
				texture_unit->flag_2 = 0x010004;
				texture_unit->flag_3_f = 0.0f;
				texture_unit->flag_3 = 0;
				sonic_material_table->texture_units.push_back(texture_unit);
			}
		}
	}


	void SonicXNFile::addFBXMaterial(FbxSurfaceMaterial *lMaterial) {
		SonicXNTexture *texture=getTexture();
		SonicXNObject  *object=getObject();
		
		if (texture && object) {
			// Set up Material Table
			SonicMaterialTable *sonic_material_table = new SonicMaterialTable();

			sonic_material_table->data_block_1_length = 20;
			sonic_material_table->data_block_2_length = 16;

			memcpy(sonic_material_table->first_floats, xno_constant_floats, 80);
			memcpy(sonic_material_table->first_ints, xno_constant_ints, 64);
			sonic_material_table->flag_table = 0x1000030;
			sonic_material_table->user_flag = 0;

			// Scan for textures
			for (size_t i=0; i<10; i++) {
				const char *property_name = NULL;

				if (i == 0) property_name = FbxSurfaceMaterial::sAmbient;
				if (i == 1) property_name = FbxSurfaceMaterial::sDiffuse;
				if (i == 2) property_name = FbxSurfaceMaterial::sEmissive;
				if (i == 3) property_name = FbxSurfaceMaterial::sSpecular;
				if (i == 4) property_name = FbxSurfaceMaterial::sBump;
				if (i == 5) property_name = FbxSurfaceMaterial::sShininess;
				if (i == 6) property_name = FbxSurfaceMaterial::sNormalMap;
				if (i == 7) property_name = FbxSurfaceMaterial::sTransparentColor;
				if (i == 8) property_name = FbxSurfaceMaterial::sReflection;
				if (i == 9) property_name = FbxSurfaceMaterial::sDisplacementColor;

				if (property_name) {
					FbxProperty lProperty = lMaterial->FindProperty(property_name);
					if (lProperty.IsValid()) addFBXMaterialProperty(&lProperty, sonic_material_table);
				}
			}

			// Push to object material tables
			object->material_tables.push_back(sonic_material_table);
		}
	}

	// FIXME: Add to mesh the used texture units in the extras
}