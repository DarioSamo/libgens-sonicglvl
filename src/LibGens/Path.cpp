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

#include "Path.h"

namespace LibGens {
	Path::Path(string filename) {
		library = NULL;
		scene = NULL;

		if (filename.find(LIBGENS_PATH_XML_EXTENSION) != string::npos) {
			readXML(filename);
		}
		else {
			File file(filename, LIBGENS_FILE_READ_BINARY);

			if (file.valid()) {
				file.readHeader();
				read(&file);
				file.close();
			}
		}
	}

	
	void Knot::read(File *file) {
		size_t header_address=file->getCurrentAddress();
		size_t address=0;

		file->goToAddress(header_address + 8);
		file->readInt32BEA(&address);
		file->goToAddress(address);
		file->readString(&type);

		file->goToAddress(header_address + 16);
		file->readInt32BEA(&address);
		file->goToAddress(address);
		file->readInt32BEA(&address);
		file->goToAddress(address+8);
		file->readInt32BEA(&address);
		file->goToAddress(address);
		invec.read(file);

		file->goToAddress(header_address + 24);
		file->readInt32BEA(&address);
		file->goToAddress(address);
		file->readInt32BEA(&address);
		file->goToAddress(address+8);
		file->readInt32BEA(&address);
		file->goToAddress(address);
		outvec.read(file);

		file->goToAddress(header_address + 32);
		file->readInt32BEA(&address);
		file->goToAddress(address);
		file->readInt32BEA(&address);
		file->goToAddress(address+8);
		file->readInt32BEA(&address);
		file->goToAddress(address);
		point.read(file);

		//printf("Found knot with type %s and the vectors:\n  Invec: %f %f %f\n  Outvec: %f %f %f\n  Point: %f %f %f\n\n", type.c_str(), invec.x, invec.y, invec.z, outvec.x, outvec.y, outvec.z, point.x, point.y, point.z);
	}


	void Spline3D::read(File *file) {
		size_t header_address=file->getCurrentAddress();
		size_t address=0;

		size_t count_address=0;
		unsigned int knots_count=0;
		size_t knots_address=0;

		file->goToAddress(header_address+16);
		file->readInt32BEA(&count_address);
		file->readInt32BE(&knots_count);
		file->readInt32BEA(&knots_address);

		file->goToAddress(count_address);
		file->readInt32BE(&count);

		//printf("New Spline3D with %d Knots\n", count);

		for (size_t i=0; i<knots_count; i++) {
			file->goToAddress(knots_address+i*4);
			file->readInt32BEA(&address);
			file->goToAddress(address);

			Knot *knot=new Knot();
			knot->read(file);
			knots.push_back(knot);
		}
	}
	
	
	void Spline::read(File *file) {
		size_t header_address=file->getCurrentAddress();
		size_t address=0;

		size_t count_address=0;
		size_t width_address=0;
		unsigned int spline3d_count=0;
		size_t spline3d_address=0;

		file->goToAddress(header_address+8);
		file->readInt32BEA(&count_address);
		file->goToAddress(header_address+16);
		file->readInt32BEA(&width_address);
		file->goToAddress(header_address+20);
		file->readInt32BE(&spline3d_count);
		file->readInt32BEA(&spline3d_address);

		file->goToAddress(count_address);
		file->readInt32BE(&count);

		file->goToAddress(width_address);
		file->readFloat32BE(&width);

		//printf("New Spline with %d Spline3Ds and %f of Width\n", spline3d_count, width);

		for (size_t i=0; i<spline3d_count; i++) {
			file->goToAddress(spline3d_address+i*4);
			file->readInt32BEA(&address);
			file->goToAddress(address);

			Spline3D *spline=new Spline3D();
			spline->read(file);
			splines.push_back(spline);
		}
	}

	
	void Geometry::read(File *file) {
		size_t header_address=file->getCurrentAddress();
		size_t address=0;

		size_t name_address=0;
		size_t id_address=0;
		size_t spline_address=0;

		file->goToAddress(header_address+8);
		file->readInt32BEA(&id_address);
		file->goToAddress(header_address+16);
		file->readInt32BEA(&name_address);
		file->goToAddress(header_address+24);
		file->readInt32BEA(&spline_address);

		file->goToAddress(name_address);
		file->readString(&name);
		file->goToAddress(id_address);
		file->readString(&id);

		//printf("Spline found with Name/ID: %s/%s\n", name.c_str(), id.c_str());

		file->goToAddress(spline_address);
		file->readInt32BEA(&address);
		file->goToAddress(address);

		spline = new Spline();
		spline->read(file);
	}

	
	void Library::read(File *file) {
		size_t header_address=file->getCurrentAddress();
		size_t address=0;

		file->goToAddress(header_address+8);
		file->readInt32BEA(&address);
		file->goToAddress(address);
		file->readString(&type);

		//printf("Library Found with type: %s\n", type.c_str());

		file->goToAddress(header_address+12);

		unsigned int geoms_count=0;
		size_t table_address=0;

		file->readInt32BE(&geoms_count);
		file->readInt32BEA(&table_address);

		for (size_t i=0; i<geoms_count; i++) {
			file->goToAddress(table_address+i*4);
			file->readInt32BEA(&address);
			file->goToAddress(address);

			Geometry *geom=new Geometry();
			geom->read(file);
			geoms.push_back(geom);
		}
	}

	
	void Node::read(File *file) {
		size_t header_address=file->getCurrentAddress();
		size_t address=0;

		size_t id_address=0;
		size_t name_address=0;
		size_t instance_url_address=0;
		size_t rotate_address=0;
		size_t scale_address=0;
		size_t translate_address=0;

		file->goToAddress(header_address+8);
		file->readInt32BEA(&id_address);
		file->goToAddress(header_address+16);
		file->readInt32BEA(&name_address);

		file->goToAddress(id_address);
		file->readString(&id);
		file->goToAddress(name_address);
		file->readString(&name);

		file->goToAddress(header_address+24);
		file->readInt32BEA(&instance_url_address);
		file->goToAddress(instance_url_address);
		file->readInt32BEA(&address);
		file->goToAddress(address+8);
		file->readInt32BEA(&address);
		file->goToAddress(address);
		file->readString(&instance_url);


		file->goToAddress(header_address+32);
		file->readInt32BEA(&rotate_address);
		file->goToAddress(rotate_address);
		file->readInt32BEA(&address);
		file->goToAddress(address+8);
		file->readInt32BEA(&address);
		file->goToAddress(address);

		rotate.read(file);


		file->goToAddress(header_address+40);
		file->readInt32BEA(&scale_address);
		file->goToAddress(scale_address);
		file->readInt32BEA(&address);
		file->goToAddress(address+8);
		file->readInt32BEA(&address);
		file->goToAddress(address);

		scale.read(file);


		file->goToAddress(header_address+48);
		file->readInt32BEA(&translate_address);
		file->goToAddress(translate_address);
		file->readInt32BEA(&address);
		file->goToAddress(address+8);
		file->readInt32BEA(&address);
		file->goToAddress(address);

		translate.read(file);

		//printf("Found node of id %s and name %s with attributes:\n  Translate: %f %f %f\n  Scale: %f %f %f\n  Rotate: %f %f %f %f\n  Instance URL: %s\n\n", id.c_str(), name.c_str(), translate.x, translate.y, translate.z, scale.x, scale.y, scale.z, rotate.x, rotate.y, rotate.z, rotate.w, instance_url.c_str());
	}

	void Scene::read(File *file) {
		size_t header_address=file->getCurrentAddress();
		size_t address=0;
		unsigned int node_count=0;
		size_t node_address=0;

		file->goToAddress(header_address+8);
		file->readInt32BEA(&address);
		file->goToAddress(address);
		file->readString(&id);

		file->goToAddress(header_address+12);
		file->readInt32BE(&node_count);
		file->readInt32BEA(&node_address);

		//printf("\n\nScene Found with id: %s\n", id.c_str());

		for (size_t i=0; i<node_count; i++) {
			file->goToAddress(node_address+i*4);
			file->readInt32BEA(&address);
			file->goToAddress(address);

			Node *node=new Node();
			node->read(file);
			nodes.push_back(node);
		}
	}
	
	void Path::read(File *file) {
		size_t header_address=file->getCurrentAddress();
		size_t address=0;

		file->goToAddress(header_address+24);
		file->readInt32BEA(&address);
		file->goToAddress(address);
		file->readInt32BEA(&address);
		file->goToAddress(address);

		library=new Library();
		library->read(file);


		file->goToAddress(header_address+32);
		file->readInt32BEA(&address);
		file->goToAddress(address);
		file->readInt32BEA(&address);
		file->goToAddress(address);

		scene=new Scene();
		scene->read(file);
	}

	void Knot::readXML(TiXmlElement *root) {
		root->QueryValueAttribute(LIBGENS_PATH_XML_TYPE, &type);

		for(TiXmlElement *pElem=root->FirstChildElement(); pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();
			
			if (element_name == LIBGENS_PATH_XML_INVEC) {
				invec.readSingleXML(pElem);
			}

			if (element_name == LIBGENS_PATH_XML_OUTVEC) {
				outvec.readSingleXML(pElem);
			}

			if (element_name == LIBGENS_PATH_XML_POINT) {
				point.readSingleXML(pElem);
			}
		}

		//printf("Found knot with type %s and the vectors:\n  Invec: %f %f %f\n  Outvec: %f %f %f\n  Point: %f %f %f\n\n", type.c_str(), invec.x, invec.y, invec.z, outvec.x, outvec.y, outvec.z, point.x, point.y, point.z);
	}

	void Spline3D::readXML(TiXmlElement *root) {
		root->QueryValueAttribute(LIBGENS_PATH_XML_COUNT, &count);

		//printf("New Spline3D with %d Knots\n", count);

		for(TiXmlElement *pElem=root->FirstChildElement(); pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();
			
			if (element_name == LIBGENS_PATH_XML_KNOT) {
				Knot *knot=new Knot();
				knot->readXML(pElem);
				knots.push_back(knot);
			}
		}
	}


	void Spline::readXML(TiXmlElement *root) {
		root->QueryValueAttribute(LIBGENS_PATH_XML_COUNT, &count);
		root->QueryValueAttribute(LIBGENS_PATH_XML_WIDTH, &width);

		//printf("New Spline with %d Spline3Ds and %f of Width\n", count, width);

		for(TiXmlElement *pElem=root->FirstChildElement(); pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();
			
			if (element_name == LIBGENS_PATH_XML_SPLINE3D) {
				Spline3D *spline = new Spline3D();
				spline->readXML(pElem);
				splines.push_back(spline);
			}
		}
	}

	
	void Geometry::readXML(TiXmlElement *root) {
		root->QueryValueAttribute(LIBGENS_PATH_XML_ID, &id);
		root->QueryValueAttribute(LIBGENS_PATH_XML_NAME, &name);

		//printf("Geometry found with Name/ID: %s/%s\n", name.c_str(), id.c_str());

		for(TiXmlElement *pElem=root->FirstChildElement(); pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();
			
			if (element_name == LIBGENS_PATH_XML_SPLINE) {
				spline = new Spline();
				spline->readXML(pElem);
				break;
			}
		}
	}


	void Library::readXML(TiXmlElement *root) {
		root->QueryValueAttribute(LIBGENS_PATH_XML_TYPE, &type);

		//printf("Library Found with type: %s\n", type.c_str());

		for(TiXmlElement *pElem=root->FirstChildElement(); pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();
			
			if (element_name == LIBGENS_PATH_XML_GEOMETRY) {
				Geometry *geom=new Geometry();
				geom->readXML(pElem);
				geoms.push_back(geom);
			}
		}
	}
	
	void Node::readXML(TiXmlElement *root) {
		root->QueryValueAttribute(LIBGENS_PATH_XML_ID, &id);
		root->QueryValueAttribute(LIBGENS_PATH_XML_NAME, &name);

		for(TiXmlElement *pElem=root->FirstChildElement(); pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();
			
			if (element_name == LIBGENS_PATH_XML_TRANSLATE) {
				translate.readSingleXML(pElem);
			}

			if (element_name == LIBGENS_PATH_XML_SCALE) {
				scale.readSingleXML(pElem);
			}

			if (element_name == LIBGENS_PATH_XML_ROTATE) {
				rotate.readSingleXML(pElem);
			}

			if (element_name == LIBGENS_PATH_XML_INSTANCE) {
				pElem->QueryValueAttribute(LIBGENS_PATH_XML_URL, &instance_url);
			}
		}

		//printf("Found node of id %s and name %s with attributes:\n  Translate: %f %f %f\n  Scale: %f %f %f\n  Rotate: %f %f %f %f\n  Instance URL: %s\n\n", id.c_str(), name.c_str(), translate.x, translate.y, translate.z, scale.x, scale.y, scale.z, rotate.x, rotate.y, rotate.z, rotate.w, instance_url.c_str());
	}

	void Scene::readXML(TiXmlElement *root) {
		root->QueryValueAttribute(LIBGENS_PATH_XML_ID, &id);

		//printf("\n\nScene Found with id: %s\n", id.c_str());

		for(TiXmlElement *pElem=root->FirstChildElement(); pElem; pElem=pElem->NextSiblingElement()) {
			string element_name=pElem->ValueStr();
			
			if (element_name == LIBGENS_PATH_XML_NODE) {
				Node *node = new Node();
				node->readXML(pElem);
				nodes.push_back(node);
			}
		}

	}

	void Path::readXML(string filename) {
		TiXmlDocument doc(filename);
		if (!doc.LoadFile()) {
			Error::addMessage(Error::FILE_NOT_FOUND, LIBGENS_PATH_ERROR_MESSAGE_NULL_FILE + filename);
			return;
		}

		TiXmlHandle hDoc(&doc);
		TiXmlElement* pElem;
		TiXmlHandle hRoot(0);

		pElem=hDoc.FirstChildElement().Element();
		if (!pElem) {
			Error::addMessage(Error::EXCEPTION, LIBGENS_PATH_ERROR_MESSAGE_NULL_FILE);
			return;
		}

		pElem=pElem->FirstChildElement();
		for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
			string entry_name=pElem->ValueStr();

			if (entry_name == LIBGENS_PATH_XML_LIBRARY) {
				library=new Library();
				library->readXML(pElem);
			}

			if (entry_name == LIBGENS_PATH_XML_SCENE) {
				scene=new Scene();
				scene->readXML(pElem);
			}
		}
	}
	
	void Knot::writeXML(TiXmlElement *parent) {
		TiXmlElement *root = new TiXmlElement(LIBGENS_PATH_XML_KNOT);
		TiXmlText *text;
		root->SetAttribute(LIBGENS_PATH_XML_TYPE, type);

		TiXmlElement *invecElem = new TiXmlElement(LIBGENS_PATH_XML_INVEC);
		text = new TiXmlText(ToString(invec.x) + " " + ToString(invec.y) + " " + ToString(invec.z));
		invecElem->LinkEndChild(text);
		root->LinkEndChild(invecElem);

		TiXmlElement *outvecElem = new TiXmlElement(LIBGENS_PATH_XML_OUTVEC);
		text = new TiXmlText(ToString(outvec.x) + " " + ToString(outvec.y) + " " + ToString(outvec.z));
		outvecElem->LinkEndChild(text);
		root->LinkEndChild(outvecElem);

		TiXmlElement *pointElem = new TiXmlElement(LIBGENS_PATH_XML_POINT);
		text = new TiXmlText(ToString(point.x) + " " + ToString(point.y) + " " + ToString(point.z));
		pointElem->LinkEndChild(text);
		root->LinkEndChild(pointElem);

		parent->LinkEndChild(root);
	}

	void Spline3D::writeXML(TiXmlElement *parent) {
		TiXmlElement *root = new TiXmlElement(LIBGENS_PATH_XML_SPLINE3D);
		root->SetAttribute(LIBGENS_PATH_XML_COUNT, count);

		for (vector<Knot *>::iterator it=knots.begin(); it!=knots.end(); it++) {
			(*it)->writeXML(root);
		}
		
		parent->LinkEndChild(root);
	}


	void Spline::writeXML(TiXmlElement *parent) {
		TiXmlElement *root = new TiXmlElement(LIBGENS_PATH_XML_SPLINE);
		root->SetAttribute(LIBGENS_PATH_XML_COUNT, count);
		root->SetAttribute(LIBGENS_PATH_XML_WIDTH, ToString(width));

		for (list<Spline3D *>::iterator it=splines.begin(); it!=splines.end(); it++) {
			(*it)->writeXML(root);
		}
		
		parent->LinkEndChild(root);
	}

	
	void Geometry::writeXML(TiXmlElement *parent) {
		TiXmlElement *root = new TiXmlElement(LIBGENS_PATH_XML_GEOMETRY);
		root->SetAttribute(LIBGENS_PATH_XML_ID, id);
		root->SetAttribute(LIBGENS_PATH_XML_NAME, name);

		if (spline) spline->writeXML(root);
		
		parent->LinkEndChild(root);
	}


	void Library::writeXML(TiXmlElement *parent) {
		TiXmlElement *root = new TiXmlElement(LIBGENS_PATH_XML_LIBRARY);
		root->SetAttribute(LIBGENS_PATH_XML_TYPE, type);

		for (list<Geometry *>::iterator it=geoms.begin(); it!=geoms.end(); it++) {
			(*it)->writeXML(root);
		}

		parent->LinkEndChild(root);
	}


	void Node::writeXML(TiXmlElement *parent) {
		TiXmlElement *root = new TiXmlElement(LIBGENS_PATH_XML_NODE);
		TiXmlText *text;

		root->SetAttribute(LIBGENS_PATH_XML_ID, id);
		root->SetAttribute(LIBGENS_PATH_XML_NAME, name);

		TiXmlElement *translateElem = new TiXmlElement(LIBGENS_PATH_XML_TRANSLATE);
		text = new TiXmlText(ToString(translate.x) + " " + ToString(translate.y) + " " + ToString(translate.z));
		translateElem->LinkEndChild(text);
		root->LinkEndChild(translateElem);

		TiXmlElement *scaleElem = new TiXmlElement(LIBGENS_PATH_XML_SCALE);
		text = new TiXmlText(ToString(scale.x) + " " + ToString(scale.y) + " " + ToString(scale.z));
		scaleElem->LinkEndChild(text);
		root->LinkEndChild(scaleElem);

		TiXmlElement *rotateElem = new TiXmlElement(LIBGENS_PATH_XML_ROTATE);
		text = new TiXmlText(ToString(rotate.x) + " " + ToString(rotate.y) + " " + ToString(rotate.z) + " " + ToString(rotate.w));
		rotateElem->LinkEndChild(text);
		root->LinkEndChild(rotateElem);

		TiXmlElement *instanceElem = new TiXmlElement(LIBGENS_PATH_XML_INSTANCE);
		instanceElem->SetAttribute(LIBGENS_PATH_XML_URL, instance_url);
		root->LinkEndChild(instanceElem);

		parent->LinkEndChild(root);
	}

	void Scene::writeXML(TiXmlElement *parent) {
		TiXmlElement *root = new TiXmlElement(LIBGENS_PATH_XML_SCENE);
		root->SetAttribute(LIBGENS_PATH_XML_ID, id);

		for (list<Node *>::iterator it=nodes.begin(); it!=nodes.end(); it++) {
			(*it)->writeXML(root);
		}

		parent->LinkEndChild(root);
	}
	
	void Path::save(string filename) {
		TiXmlDocument doc;
		TiXmlDeclaration *decl = new TiXmlDeclaration( "1.0", "", "" );
		doc.LinkEndChild( decl );
		TiXmlElement *currentElem = new TiXmlElement(LIBGENS_PATH_XML_ROOT);

		if (library) {
			library->writeXML(currentElem);
		}

		if (scene) {
			scene->writeXML(currentElem);
		}

		doc.LinkEndChild(currentElem);
		doc.SaveFile(filename);
	}


	Spline *Library::getSpline(string instance_name) {
		for (list<Geometry *>::iterator it=geoms.begin(); it!=geoms.end(); it++) {
			if ((*it)->getID() == instance_name) {
				return (*it)->getSpline();
			}
		}

		return NULL;
	}


	Vector3 Node::findClosestPoint(Spline *spline, Vector3 target_position, float *target_distance, Vector3 *result_tangent, bool ignore_vertical_tangents) {
		size_t knots_size = spline->getKnotsSize();

		float closest_distance = LIBGENS_AABB_MAX_START;
		Vector3 closest_point(LIBGENS_AABB_MAX_START, LIBGENS_AABB_MAX_START, LIBGENS_AABB_MAX_START);

		Matrix4 node_matrix;
		node_matrix.makeTransform(translate, scale, rotate);

		for (size_t knot_index=0; knot_index < knots_size-1; knot_index++) {
			// Retrieve the start and end point of the segment
			// Multiply them by the transformation matrix to get their world position
			Vector3 first_position = spline->getKnotPoint(knot_index);
			first_position = node_matrix * first_position;

			Vector3 second_position = spline->getKnotPoint(knot_index + 1);
			second_position = node_matrix * second_position;

			// Calculate direction and length of segment
			Vector3 direction = second_position - first_position;
			float segment_length = direction.normalise();

			// Calculate distance between target position and start of the segment
			Vector3 distance = target_position - first_position;

			// Find out the matching point of the target position to the spline with dot product
			// against segment's direction
			float target_length = distance.dotProduct(direction);
			if (target_length < 0) target_length = 0;
			if (target_length > segment_length) target_length = segment_length;

			Vector3 spline_point = spline->interpolateSegment(knot_index, target_length / segment_length);
			Vector3 node_spline_point = node_matrix * spline_point;

			// Check if the distance to the matching point is lower than the current winning distance
			float spline_point_distance = (node_spline_point - target_position).length();
			if (spline_point_distance < closest_distance) {
				Vector3 spline_tangent = spline_point + spline->interpolateSegmentTangent(knot_index, target_length / segment_length);
				spline_tangent = node_matrix * spline_tangent;
				spline_tangent = spline_tangent - node_spline_point;
				spline_tangent.normalise();

				if (ignore_vertical_tangents) {
					if (abs(spline_tangent.y) >= 0.5) {
						continue;
					}
				}

				if (result_tangent) {
					*result_tangent = spline_tangent;
				}

				closest_distance = spline_point_distance;
				closest_point = node_spline_point;

				if (target_distance) {
					*target_distance = closest_distance;
				}
			}
		}

		return closest_point;
	}
};