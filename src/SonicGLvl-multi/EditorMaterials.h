#pragma once

namespace LibGens {
	class ShaderLibrary;
	class Material;
}

class EditorMaterials {
protected:
	QList<LibGens::Material *> materials;
public:
	EditorMaterials();

	void load(QString directory, LibGens::ShaderLibrary *shader_library, QWidget *parent);
};