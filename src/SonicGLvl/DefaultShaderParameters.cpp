#include "DefaultShaderParameters.h"

DefaultShaderParameter::DefaultShaderParameter() {
	name = "";
	r=g=b=a=0.0f;
}


void DefaultShaderParameter::read(TiXmlElement *root) {
	root->QueryValueAttribute("name", &name);
	root->QueryFloatAttribute("r", &r);
	root->QueryFloatAttribute("g", &g);
	root->QueryFloatAttribute("b", &b);
	root->QueryFloatAttribute("a", &a);
}


DefaultShaderParameters::DefaultShaderParameters(string filename) {
	TiXmlDocument doc(filename);
	if (!doc.LoadFile()) {
		return;
	}

	TiXmlHandle hDoc(&doc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	pElem=hDoc.FirstChildElement().Element();
	if (!pElem) {
		return;
	}

	pElem=pElem->FirstChildElement();
	for(pElem; pElem; pElem=pElem->NextSiblingElement()) {
		string entry_name = pElem->ValueStr();

		if (entry_name=="Parameter") {
			DefaultShaderParameter *parameter = new DefaultShaderParameter();
			parameter->read(pElem);
			parameters.push_back(parameter);
		}
	}
}