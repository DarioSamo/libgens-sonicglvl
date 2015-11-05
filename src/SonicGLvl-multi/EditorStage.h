#pragma once

class EditorStage {
protected:
	QString stage_name;
	QString stage_filename;
#ifdef SONICGLVL_LOST_WORLD
	QString sets_directory;
#endif
public:
	EditorStage();
	QString stageName();
	bool load(QString filename, QString &error);
	static QString extension();
	QString filename();
#ifdef SONICGLVL_LOST_WORLD
	QString setsDirectory();
#endif
};