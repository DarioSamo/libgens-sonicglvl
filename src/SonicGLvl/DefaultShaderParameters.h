#pragma once

class DefaultShaderParameter {
public:
	string name;
	float r, g, b, a;

	DefaultShaderParameter();
	void read(TiXmlElement *root);
};

class DefaultShaderParameters {
public:
	list<DefaultShaderParameter *> parameters;

	DefaultShaderParameters(string filename);
};