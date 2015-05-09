#include "ObjectExtra.h"
#include "Object.h"

namespace LibGens {
	ObjectExtra::ObjectExtra(ObjectExtra *extra) {
		type = extra->getType();
		name = extra->getName();
		parameters = extra->getParameters();
		parameter_names = extra->getParameterNames();
	}

	ObjectExtra::ObjectExtra() {
	}

	bool ObjectExtra::compare(ObjectExtra *extra) {
		if (!extra) return false;
		if (type != extra->getType()) return false;
		if (name != extra->getName()) return false;
		if (parameters.size() != extra->getParameterSize()) return false;
		if (parameter_names.size() != extra->getParameterNameSize()) return false;

		for (size_t i=0; i<parameters.size(); i++) {
			if (parameters[i] != extra->getParameter(i)) {
				return false;
			}
		}

		for (size_t i=0; i<parameter_names.size(); i++) {
			if (parameter_names[i] != extra->getParameterName(i)) {
				return false;
			}
		}

		return true;
	}

	void ObjectExtra::setName(string nm) {
		name=nm;
	}

	string ObjectExtra::getName() {
		return name;
	}

	void ObjectExtra::setType(string nm) {
		type=nm;
	}

	string ObjectExtra::getType() {
		return type;
	}

	void ObjectExtra::addParameter(string p, string v) {
		parameter_names.push_back(p);
		parameters.push_back(v);
	}

	void ObjectExtra::setParameterNames(vector<string> v) {
		parameter_names = v;
	}

	vector<string> ObjectExtra::getParameterNames() {
		return parameter_names;
	}

	void ObjectExtra::setParameters(vector<string> v) {
		parameters = v;
	}

	vector<string> ObjectExtra::getParameters() {
		return parameters;
	}

	size_t ObjectExtra::getParameterSize() {
		return parameters.size();
	}

	size_t ObjectExtra::getParameterNameSize() {
		return parameter_names.size();
	}

	string ObjectExtra::getParameter(size_t index) {
		if (index >= parameters.size()) return "";
		return parameters[index];
	}

	string ObjectExtra::getParameterName(size_t index) {
		if (index >= parameter_names.size()) return "";
		return parameter_names[index];
	}

	void ObjectExtra::writeXMLTemplate(TiXmlElement *root) {
		TiXmlElement* extraRoot=new TiXmlElement(LIBGENS_OBJECT_ELEMENT_EXTRA);
		extraRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_TYPE, type);
		extraRoot->SetAttribute(LIBGENS_OBJECT_ELEMENT_ATTRIBUTE_NAME, name);

		for (size_t i=0; i<parameter_names.size(); i++) {
			extraRoot->SetAttribute(parameter_names[i], parameters[i]);
		}

		root->LinkEndChild(extraRoot);
	}
};