#include "stdafx.h"
#include "EditorMaterials.h"
#include "EditorMaterialConverter.h"

EditorMaterials::EditorMaterials() {

}

void EditorMaterials::load(QString directory, LibGens::ShaderLibrary *shader_library, QWidget *parent) {
	QDir dir(directory);
	dir.setFilter(QDir::Files);
	dir.setNameFilters(QStringList() << "*.material");
	QStringList material_names = dir.entryList();

	int progress_count = 0;
	int progress_max_count = material_names.size();
	QProgressDialog progress(QString(), QString(), 0, progress_max_count, parent);
	progress.setWindowTitle("Loading materials...");
    progress.setWindowModality(Qt::WindowModal);

	foreach(QString material_name, material_names) {
		progress.setLabelText("Loading material " + material_name);
		progress.setValue(progress_count++);

		LibGens::Material *material = new LibGens::Material((directory + "/" + material_name).toStdString());
		Ogre::Material *ogre_material = EditorMaterialConverter::convertMaterial(material, shader_library);
		materials.append(material);
	}

	progress.setValue(progress_max_count);
}