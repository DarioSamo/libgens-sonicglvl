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

#pragma once

#define LIBGENS_PATH_ERROR_MESSAGE_NULL_FILE     "Trying to read path data from unreferenced file."
#define LIBGENS_PATH_XML_EXTENSION               ".xml"
#define LIBGENS_PATH_FULL_GENERATIONS_EXTENSION  ".path.xml"

#define LIBGENS_PATH_XML_ROOT                    "SonicPath"
#define LIBGENS_PATH_XML_LIBRARY                 "library"
#define LIBGENS_PATH_XML_GEOMETRY                "geometry"
#define LIBGENS_PATH_XML_SPLINE                  "spline"
#define LIBGENS_PATH_XML_SPLINE3D                "spline3d"
#define LIBGENS_PATH_XML_COUNT                   "count"
#define LIBGENS_PATH_XML_WIDTH                   "width"
#define LIBGENS_PATH_XML_KNOT                    "knot"
#define LIBGENS_PATH_XML_POINT                   "point"
#define LIBGENS_PATH_XML_INVEC                   "invec"
#define LIBGENS_PATH_XML_OUTVEC                  "outvec"
#define LIBGENS_PATH_XML_SCENE                   "scene"
#define LIBGENS_PATH_XML_TYPE                    "type"
#define LIBGENS_PATH_XML_NODE                    "node"
#define LIBGENS_PATH_XML_ID                      "id"
#define LIBGENS_PATH_XML_NAME                    "name"
#define LIBGENS_PATH_XML_TRANSLATE               "translate"
#define LIBGENS_PATH_XML_SCALE                   "scale"
#define LIBGENS_PATH_XML_ROTATE                  "rotate"
#define LIBGENS_PATH_XML_INSTANCE                "instance"
#define LIBGENS_PATH_XML_URL                     "url"

#define LIBGENS_PATH_TAG_2D                      "@SV"
#define LIBGENS_PATH_TAG_QUICKSTEP               "@QS"
#define LIBGENS_PATH_TAG_DASH                    "@DP"
#define LIBGENS_PATH_TAG_GRIND_RAIL              "@GR"
#define LIBGENS_PATH_TAG_SUPERSONIC              "super_sonic"

namespace LibGens {
	class Knot {
		public:
			Vector3 invec;
			Vector3 outvec;
			Vector3 point;
			string type;

			Knot() {}
			void read(File *file);
			void readXML(TiXmlElement *parent);
			void writeXML(TiXmlElement *parent);


	};

	class Spline3D {
		protected:
			vector<Knot *> knots;
			unsigned int count;
		public:
			Spline3D() {}
			void read(File *file);
			void readXML(TiXmlElement *parent);
			void writeXML(TiXmlElement *parent);

			vector<Knot *> getKnots() {
				return knots;
			}

			Knot *getKnot(size_t knot_index) {
				if (knot_index < knots.size()) {
					return knots[knot_index];
				}
				else return NULL;
			}

			float getSegmentLength(size_t knot_index) {
				if (knot_index < (knots.size()-1)) {
					return (knots[knot_index + 1]->point - knots[knot_index]->point).length();
				}
				else return 0.0f;
			}

			Vector3 interpolateSegment(size_t knot_index, float t) {
				Vector3 result(0.0, 0.0, 0.0);

				if (knot_index < knots.size()-1) {
					float coeff_1 = pow(1.0-t, 3);
					float coeff_2 = 3 * pow(1.0-t, 2) * t;
					float coeff_3 = 3 * (1.0-t) * pow(t, 2);
					float coeff_4 = pow(t, 3);
					
					result = result + (knots[knot_index]->point     * coeff_1);
					result = result + (knots[knot_index]->outvec    * coeff_2);
					result = result + (knots[knot_index + 1]->invec * coeff_3);
					result = result + (knots[knot_index + 1]->point * coeff_4);
					return result;
				}
				else {
					return result;
				}
			}

			Vector3 interpolateSegmentTangent(size_t knot_index, float t) {
				Vector3 result(0.0, 0.0, 0.0);

				if (knot_index < knots.size()-1) {
					float coeff_1 = -3*pow(1.0-t, 2);
					float coeff_2 = 3*pow(1.0-t, 2) + 6*pow(t, 2) - 6*t ;
					float coeff_3 = -9*pow(t, 2) + 6*t;
					float coeff_4 = 3*pow(t, 2);
					
					result = result + (knots[knot_index]->point     * coeff_1);
					result = result + (knots[knot_index]->outvec    * coeff_2);
					result = result + (knots[knot_index + 1]->invec * coeff_3);
					result = result + (knots[knot_index + 1]->point * coeff_4);

					result.normalise();
					return result;
				}
				else {
					return result;
				}
			}
	};

	class Spline {
		protected:
			list<Spline3D *> splines;
			float width;
			unsigned int count;
		public:
			Spline() {}
			void read(File *file);
			void readXML(TiXmlElement *parent);
			void writeXML(TiXmlElement *parent);

			list<Spline3D *> getSplines() {
				return splines;
			}

			size_t getKnotsSize() {
				if (!splines.size()) return 0;

				Spline3D *first_spline = (*splines.begin());

				if (first_spline) {
					return first_spline->getKnots().size();
				}
				else return 0;
			}

			Vector3 getKnotPoint(size_t knot_index) {
				Vector3 result(0,0,0);
				if (!splines.size()) return result;

				for (list<Spline3D *>::iterator it=splines.begin(); it!=splines.end(); it++) {
					Knot *knot = (*it)->getKnot(knot_index);

					if (knot) {
						result = result + knot->point;
					}
				}

				result = result / (float)splines.size();
				return result;
			}

			Vector3 interpolateSegment(size_t knot_index, float t) {
				Vector3 result(0,0,0);
				if (!splines.size()) return result;

				for (list<Spline3D *>::iterator it=splines.begin(); it!=splines.end(); it++) {
					result = result + (*it)->interpolateSegment(knot_index, t);
				}

				result = result / (float)splines.size();
				return result;
			}

			Vector3 interpolateSegmentTangent(size_t knot_index, float t) {
				Vector3 result(0,0,0);
				if (!splines.size()) return result;

				for (list<Spline3D *>::iterator it=splines.begin(); it!=splines.end(); it++) {
					result = result + (*it)->interpolateSegmentTangent(knot_index, t);
				}

				result = result / (float)splines.size();
				return result;
			}
	};

	class Geometry {
		protected:
			Spline *spline;
			string name;
			string id;
		public:
			Geometry() {}
			void read(File *file);
			void readXML(TiXmlElement *parent);
			void writeXML(TiXmlElement *parent);

			string getID() {
				return id;
			}

			string getName() {
				return name;
			}

			Spline *getSpline() {
				return spline;
			}
	};

	class Library {
		protected:
			list<Geometry *> geoms;
			string type;
		public:
			Library() {}
			void read(File *file);
			void readXML(TiXmlElement *parent);
			void writeXML(TiXmlElement *parent);

			Spline *getSpline(string instance_name);
	};

	class Node {
		protected:
			string id;
			string name;
			string instance_url;

			size_t stage_id;

			Vector3 translate;
			Vector3 scale;
			Quaternion rotate;
		public:
			Node() {
				stage_id = 0;
			}
			void read(File *file);
			void readXML(TiXmlElement *parent);
			void writeXML(TiXmlElement *parent);

			void setStageID(size_t v) {
				stage_id = v;
			}

			size_t getStageID() {
				return stage_id;
			}

			Vector3 getTranslate() {
				return translate;
			}

			Vector3 getScale() {
				return scale;
			}

			Quaternion getRotation() {
				return rotate;
			}

			string getInstanceName() {
				return instance_url;
			}

			Vector3 findClosestPoint(Spline *spline, Vector3 target_position, float *target_distance=NULL, Vector3 *result_tangent=NULL, bool ignore_vertical_tangents=false);
	};

	class Scene {
		protected:
			list<Node *> nodes;
			string id;
		public:
			Scene() {}
			void read(File *file);
			void readXML(TiXmlElement *parent);
			void writeXML(TiXmlElement *parent);

			list<Node *> getNodes() {
				return nodes;
			}
	};

	class Path {
		protected:
			Library *library;
			Scene *scene;
		public:
			Path(string filename);
			void read(File *file);
			void readXML(string filename);
			void save(string filename);

			Library *getLibrary() {
				return library;
			}

			Scene *getScene() {
				return scene;
			}
	};
};
