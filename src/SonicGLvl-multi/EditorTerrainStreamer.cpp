#include "stdafx.h"
#include <QProcess>
#include "LibGens.h"
#include "EditorTerrainStreamer.h"
#include "Terrain.h"
#include "TerrainGroup.h"

/** EditorTerrainGroupLoader */

EditorTerrainGroupLoader::EditorTerrainGroupLoader(LibGens::TerrainGroup *group, QString terrain_folder) {
	this->group = group;
	this->terrain_folder = terrain_folder;
	thread = new QThread();
}

EditorTerrainGroupLoader::~EditorTerrainGroupLoader() {
	thread->quit();
	thread->wait();
	delete thread;
}

void EditorTerrainGroupLoader::process() {
	// Verify if file exists already
	QString terrain_group_filename = QString("%1/%2%3").arg(terrain_folder).arg(group->getName().c_str()).arg(LIBGENS_TERRAIN_GROUP_FOLDER_EXTENSION);
	if (!QFileInfo(terrain_group_filename).exists()) {
		string ar_filename_cab = terrain_group_filename.toStdString() + ".cab";
		expandFileCAB(ar_filename_cab.c_str(), terrain_group_filename);
	}

	// Load group with models and instances if file exists
	if (QFileInfo(terrain_group_filename).exists()) {
		group->load();
	}

	emit loadedGroup(this);
}

LibGens::TerrainGroup *EditorTerrainGroupLoader::getGroup() {
	return group;
}

void EditorTerrainGroupLoader::expandFileCAB(QString filename, QString new_filename) {
	QStringList arguments;
	arguments << filename << new_filename;
	QProcess decompression_process;
	decompression_process.start("expand", arguments);
	decompression_process.waitForFinished();
	QFile::remove(filename);
}

QThread *EditorTerrainGroupLoader::getThread() {
	return thread;
}

/** EditorTerrainStreamer */

EditorTerrainStreamer::EditorTerrainStreamer() {
	terrain = NULL;
	max_loaders = 1;
	loading_range = 1000;
}

void EditorTerrainStreamer::setTerrain(LibGens::Terrain *v) {
	terrain = v;
	terrain_groups = terrain->getGroups();
	loaded_map.clear();
}

void EditorTerrainStreamer::setMaxLoaders(int v) {
	max_loaders = v;
}

void EditorTerrainStreamer::setLoadingRange(float v) {
	loading_range = v;
}

void EditorTerrainStreamer::setTerrainFolder(QString v) {
	terrain_folder = v;
}

bool EditorTerrainStreamer::hasPendingGroup() {
	return !pending_groups.isEmpty();
}

EditorTerrainStreamer::Group EditorTerrainStreamer::popPendingGroup() {
	EditorTerrainStreamer::Group group;
	group.first = NULL;
	group.second = false;

	if (!pending_groups.isEmpty()) {
		group = pending_groups.pop();
	}

	return group;
}

void EditorTerrainStreamer::update(Ogre::Vector3 ref) {
	if (!terrain)
		return;

	if (terrain_folder.isEmpty())
		return;

	LibGens::Vector3 reference_position(ref.x, ref.y, ref.z);

	// Only verify what groups need to be loaded if there's remaining loaders to use
	if (active_loaders.size() < max_loaders) {
		// Detect which groups need to be loaded and are closest
		QMap<float, LibGens::TerrainGroup *> terrain_group_map;
		size_t sz = terrain_groups.size();
		for (size_t i = 0; i < sz; i++) {
			if (!loaded_map[terrain_groups[i]]) {
				float distance = terrain_groups[i]->getDistance(reference_position) - terrain_groups[i]->getRadius();
				if (distance < loading_range) {
					terrain_group_map.insertMulti(distance, terrain_groups[i]);
				}
			}
		}

		// Create new loaders
		QList<LibGens::TerrainGroup *> groups_to_load = terrain_group_map.values();
		
		int loader_add_limit = max_loaders - active_loaders.size();
		QList<EditorTerrainGroupLoader *> loaders_to_start;
		while ((loaders_to_start.size() < loader_add_limit) && !groups_to_load.isEmpty()) {
			LibGens::TerrainGroup *group = groups_to_load.first();
			loaded_map[group] = true;
			groups_to_load.pop_front();

			EditorTerrainGroupLoader *loader = new EditorTerrainGroupLoader(group, terrain_folder);
			loaders_to_start.append(loader);
			active_loaders.append(loader);
		}

		// Start new loaders
		foreach(EditorTerrainGroupLoader *loader, loaders_to_start) {
			QThread *thread = loader->getThread();
			loader->moveToThread(thread);
			connect(thread, SIGNAL(started()), loader, SLOT(process()), Qt::DirectConnection);
			connect(thread, SIGNAL(finished()), loader, SLOT(deleteLater()), Qt::DirectConnection);
			connect(loader, SIGNAL(loadedGroup(EditorTerrainGroupLoader *)), this, SLOT(groupLoaded(EditorTerrainGroupLoader *)), Qt::QueuedConnection);
			thread->start();
		}
	}
}

void EditorTerrainStreamer::groupLoaded(EditorTerrainGroupLoader *loader) {
	LibGens::TerrainGroup *group = loader->getGroup();
	active_loaders.removeOne(loader);

	pending_groups.push(Group(group, true));
}