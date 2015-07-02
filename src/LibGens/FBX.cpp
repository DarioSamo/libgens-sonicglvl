//=========================================================================
//	  Copyright (c) 2015 SonicGLvl
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

#include "FBX.h"
#include "Material.h"
#include "Parameter.h"
#include "Texture.h"
#include "Vertex.h"
#include "VertexFormat.h"
#include "Submesh.h"
#include "Model.h"
#include "Mesh.h"
#include "MaterialLibrary.h"
#include "TerrainInstance.h"

namespace LibGens {
	FBX::FBX(FbxManager *sdk_manager, string scene_name) {
		scene = FbxScene::Create(sdk_manager, scene_name.c_str());
	}

	FBX::FBX(FbxScene* lScene) {
		material_library = new MaterialLibrary("");

		// This function show how to cycle through scene elements in a linear way.
		const int lNodeCount = lScene->GetSrcObjectCount<FbxNode>();
		FbxStatus lStatus;

		for (int lIndex=0; lIndex<lNodeCount; lIndex++) {
			FbxNode *lNode = lScene->GetSrcObject<FbxNode>(lIndex);
			printf("Node found: %s\n", lNode->GetName());

			FbxMesh *lMesh=lNode->GetMesh();
			if (lMesh) {
				TerrainInstance *terrain_instance=buildTerrainInstanceFromFBX(lNode);
				instances.push_back(terrain_instance);
				models.push_back(terrain_instance->getModel());
			}
		}

		scene = lScene;
	}


	Material *FBX::buildMaterialFromFBX(FbxSurfaceMaterial *lMaterial) {
		Material *material=new Material(lMaterial->GetName());

		material->setShader("Common_d");
		material->addParameter(new Parameter("diffuse", Color(1.0, 1.0, 1.0, 0.0)));
		material->addParameter(new Parameter("ambient", Color(1.0, 1.0, 1.0, 0.0)));
		material->addParameter(new Parameter("specular", Color(0.9, 0.9, 0.9, 0.0)));
		material->addParameter(new Parameter("emissive", Color(0.0, 0.0, 0.0, 0.0)));
		material->addParameter(new Parameter("power_gloss_level", Color(50.0, 0.1, 0.0, 0.0)));
		material->addParameter(new Parameter("opacity_reflection_refraction_spectype", Color(1.0, 0.0, 1.0, 0.0)));
		material->addParameter(new Parameter("", Color(0.0, 0.0, 0.0, 0.0)));
		material->addParameter(new Parameter("", Color(0.0, 0.0, 0.0, 0.0)));
		material->addParameter(new Parameter("", Color(0.0, 0.0, 0.0, 0.0)));
		material->addParameter(new Parameter("", Color(0.0, 0.0, 0.0, 0.0)));

		FbxProperty lProperty = lMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		if (lProperty.IsValid()) {
			const int lTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();
			if (lTextureCount) {
				const FbxFileTexture* lTexture = lProperty.GetSrcObject<FbxFileTexture>();
				if (lTexture) {
					string path=ToString(lTexture->GetFileName());

					size_t sep = path.find_last_of("\\/");
					if (sep != std::string::npos) {
						path = path.substr(sep + 1, path.size() - sep - 1);
					}
	
					size_t dot = path.find_last_of(".");
					if (dot != string::npos) {
						path = path.substr(0, dot);
					}

					Texture *texture=new Texture(ToString(lMaterial->GetName())+"-0000", "diffuse", path);
					material->addTextureUnit(texture);
				}
			}
		}
		return material;
	}

	Submesh *FBX::buildSubmeshFromFBX(FbxMesh *lMesh, int material_index, bool single_material) {
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
			vector<Vertex *> new_vertices;
			vector<Vector3> new_faces;

			int control_points_count=lMesh->GetControlPointsCount();
			FbxVector4 *control_points=lMesh->GetControlPoints();

			for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex) {
				if (lMaterialIndice) {
					const int lMaterialIndex = lMaterialIndice->GetAt(lPolygonIndex);
					if (lMaterialIndex != material_index) continue;
				}

				int polygon_size=lMesh->GetPolygonSize(lPolygonIndex);
				if (polygon_size == 3) {
					Vector3 face;

					for (int j=0; j<polygon_size; j++) {
						Vertex *vertex=new Vertex();

						int control_point_index=lMesh->GetPolygonVertex(lPolygonIndex, j);
						FbxVector4 control_point=control_points[control_point_index];
						vertex->setPosition(Vector3(control_point[0], control_point[2], -control_point[1]));

						FbxVector4 normal;
						lMesh->GetPolygonVertexNormal(lPolygonIndex, j, normal);
						vertex->setNormal(Vector3(normal[0], normal[2], -normal[1]));

						FbxStringList uv_sets;
						lMesh->GetUVSetNames(uv_sets);

						for (int set=0; set<uv_sets.GetCount(); set++) {
							if (set >= 4) break;
							FbxVector2 uv;
							bool no_uv;
							lMesh->GetPolygonVertexUV(lPolygonIndex, j, uv_sets[set].Buffer(), uv, no_uv);
							vertex->setUV(Vector2(uv[0], 1.0 - uv[1]), set);
						}
						
						if (j == 0) face.x=new_vertices.size();
						if (j == 1) face.y=new_vertices.size();
						if (j == 2) face.z=new_vertices.size();

						new_vertices.push_back(vertex);
					}

					new_faces.push_back(face);
				}
				else printf("Unsupported polygon size %d.\n", polygon_size);
			}

			printf("Building submesh with %d vertices and %d faces\n", new_vertices.size(), new_faces.size());

			if ((new_vertices.size() >= 3) && new_faces.size()) {
				Submesh *submesh=new Submesh();
				submesh->build(new_vertices, new_faces);
				submesh->addBone(0);

				VertexFormat *vertex_format=new VertexFormat(LIBGENS_VERTEX_FORMAT_PC);
				submesh->setVertexFormat(vertex_format);
				return submesh;
			}
		}

		return NULL;
	}


	Model *FBX::buildModelFromFBX(FbxMesh *lMesh, int material_count, vector<string> material_names) {
		Model *model=new Model();
		model->setTerrainMode(true);

		Mesh *mesh=new Mesh();
		model->addMesh(mesh);

		for (int m=0; m<material_count; m++) {
			printf("Subdiving meshes for %s\n", material_names[m].c_str());
			Submesh *terrain_submesh=buildSubmeshFromFBX(lMesh, m, (material_names.size() == 1 ? true : false));
			if (terrain_submesh) {
				mesh->addSubmesh(terrain_submesh, LIBGENS_MODEL_SUBMESH_SLOT_SOLID);
				terrain_submesh->setMaterialName(material_names[m]);
			}
		}
		return model;
	}


	TerrainInstance *FBX::buildTerrainInstanceFromFBX(FbxNode *lNode) {
		FbxVector4 node_translation=lNode->EvaluateLocalTranslation();
		FbxVector4 node_rotation=lNode->EvaluateLocalRotation();
		FbxVector4 node_scale=lNode->EvaluateLocalScaling();

		Matrix4 instance_matrix=Matrix4();
		instance_matrix.makeTransform(Vector3    (node_translation[0], node_translation[1], node_translation[2]),
									  Vector3    (node_scale[0], node_scale[1], node_scale[2]),
									  Quaternion (node_rotation[3], node_rotation[0], node_rotation[1], node_rotation[2]));
	
		// Convert and add materials to library
		vector<string> material_names;
		int material_count = lNode->GetMaterialCount();
		for (int m=0; m<material_count; m++) {
			FbxSurfaceMaterial *lMaterial=lNode->GetMaterial(m);
			string material_name = lMaterial->GetName();
			bool check_material_exists=material_library->checkMaterial(material_name);
			if (!check_material_exists) {
				Material *material=buildMaterialFromFBX(lMaterial);
				material_library->addMaterial(material);
			}
			material_names.push_back(material_name);
		}

		Model *terrain_model=buildModelFromFBX(lNode->GetMesh(), material_count, material_names);
		terrain_model->setName(lNode->GetName());
		TerrainInstance *terrain_instance=new TerrainInstance(lNode->GetName(), terrain_model, instance_matrix);
		return terrain_instance;
	}

	list<Model *> FBX::getModels() {
		return models;
	}

	list<TerrainInstance *> FBX::getInstances() {
		return instances;
	}

	void FBX::setMaterialLibrary(MaterialLibrary *v) {
		material_library = v;
	}

	MaterialLibrary *FBX::getMaterialLibrary() {
		return material_library;
	}

	FbxScene *FBX::getScene() {
		return scene;
	}
}