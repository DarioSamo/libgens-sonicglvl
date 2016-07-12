#pragma once

#include <QStack>
#include <QThread>

using namespace std;

namespace LibGens {
	class ArPack;
	class Terrain;
	class TerrainGroup;
}

class EditorTerrainGroupLoader : public QObject {
	Q_OBJECT
protected:
	LibGens::TerrainGroup *group;
	QThread *thread;
	QString terrain_folder;
public:
	EditorTerrainGroupLoader(LibGens::TerrainGroup *group, QString terrain_folder);
	~EditorTerrainGroupLoader();
	QThread *getThread();
	LibGens::TerrainGroup *getGroup();
	void expandFileCAB(QString filename, QString new_filename);
private slots:
	void process();
signals:
	void loadedGroup(EditorTerrainGroupLoader *);
};

class EditorTerrainStreamer : public QObject {
	Q_OBJECT
public:
	typedef QPair<LibGens::TerrainGroup *, bool> Group;
protected:
	QStack<Group> pending_groups;
	QList<EditorTerrainGroupLoader *> active_loaders;
	LibGens::Terrain *terrain;
	vector<LibGens::TerrainGroup *> terrain_groups;
	QString terrain_folder;
	int max_loaders;
	float loading_range;
	QMap<LibGens::TerrainGroup *, bool> loaded_map;
public:
	EditorTerrainStreamer();

	void setTerrain(LibGens::Terrain *v);
	void setMaxLoaders(int v);
	void setLoadingRange(float v);
	void setTerrainFolder(QString v);

	bool hasPendingGroup();
	Group popPendingGroup();

	void update(Ogre::Vector3 reference_position);
private slots:
	void groupLoaded(EditorTerrainGroupLoader *loader);
};