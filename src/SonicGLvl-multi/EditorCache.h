#pragma once

class EditorHash {
friend class EditorHash;
protected:
	int hash_data[5];
public:
	EditorHash();
	EditorHash(int *v);
	EditorHash(QString hash_string);
	QString toString();
	bool operator==(const EditorHash &h) const;
};

class EditorCacheHash {
protected:
	QString stage_name;
	QMap<QString, EditorHash> file_hashes;
public:
	static const QString DocumentRoot;
	static const QString DocumentHashRoot;

	EditorCacheHash();
	bool compareFileHash(QString name, EditorHash hash);
	void addFileHash(QString name, EditorHash hash);
	void readDocument(QDomElement &element);
	void writeDocument(QDomDocument &document, QDomElement &element);
	void setStageName(const QString &name);
	const QString &getStageName();
};

class EditorCache {
protected:
	QString path;
	QList<EditorCacheHash> hashes;
public:
	static const QString DocumentRoot;
	static const QString CachePath;
	static const QString CacheHashPath;

#ifdef SONICGLVL_LOST_WORLD
	static const QString ConfigPath;
	static const QString FarPath;
	static const QString MiscPath;
	static const QString ObjPath;
	static const QString SkyPath;
	static const QString TrrCmnPath;
	static const QString PacExtension;
#elif SONICGLVL_GENERATIONS
	static const QString PackedPath;
	static const QString DataPath;
	static const QString ResourcesPath;
	static const QString TerrainPath;
	static const QString TerrainAddPath;
	static const QString StagePath;
	static const QString StageAddPath;
	static const QString ArExtension;
	static const QString PfdExtension;
#endif

	EditorCache(QString program_path);
	bool loadHashes();
	bool saveHashes();

	/** Verifies if the file to be unpacked is already on cache by comparing the hashes. If it isn't, clean the contents 
		of the directory and unpacked the new contents.

		@param stage_name Current stage being unpacked.
		@param filename Full filename to be unpacked.
		@param logic_name Logical name of the file to be stored inside the hashes.
	*/
	void unpackFileSafe(QString stage_name, QString filename, QString logic_name, QProgressDialog &progress, QString suffix = QString());
	bool unpackStage(QString stage_name, QString path, QWidget *parent);
	bool packStage(QString stage_name, QString path);
	QString hashFilename();
	QString absolutePath();
	QString stagePath(QString stage_name);
	EditorCacheHash &getEditorCacheHash(QString stage_name);

#ifdef SONICGLVL_LOST_WORLD
	QString skyPath(QString stage_name);
	QString farPath(QString stage_name);
	QString terrainCommonPath(QString stage_name);
#elif SONICGLVL_GENERATIONS
	QString dataPath(QString stage_name);
	QString resourcesPath(QString stage_name);
	QString terrainPath(QString stage_name);
	QString terrainAddPath(QString stage_name);
#endif
};