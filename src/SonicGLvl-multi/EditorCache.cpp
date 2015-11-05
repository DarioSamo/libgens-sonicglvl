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

#include "stdafx.h"
#include "EditorCache.h"
#include "LibGens.h"
#include "PAC.h"

/** EditorHash */

EditorHash::EditorHash() {
	for (int i=0; i<5; i++) {
		hash_data[i] = 0;
	}
}

EditorHash::EditorHash(int *v) {
	for (int i=0; i<5; i++) {
		hash_data[i] = v[i];
	}
}

EditorHash::EditorHash(QString hash_string) {
	QStringList hash_string_list = hash_string.split(".");
	if (hash_string_list.size() == 5) {
		for (int i=0; i<5; i++) {
			hash_data[i] = hash_string_list[i].toInt();
		}
	}
}

QString EditorHash::toString() {
	return QString("%1.%2.%3.%4.%5").arg(hash_data[0]).arg(hash_data[1]).arg(hash_data[2]).arg(hash_data[3]).arg(hash_data[4]);
}

bool EditorHash::operator==(const EditorHash &h) const {
	for (int i=0; i<5; i++) {
		if (h.hash_data[i] != hash_data[i]) {
			return false;
		}
	}

	return true;
}

/** EditorCacheHash */
const QString EditorCacheHash::DocumentRoot = "CacheHash";
const QString EditorCacheHash::DocumentHashRoot = "Hash";

EditorCacheHash::EditorCacheHash() {
	file_hashes.clear();
}

void EditorCacheHash::addFileHash(QString name, EditorHash hash) {
	file_hashes[name] = hash;
}

void EditorCacheHash::setStageName(const QString &name) {
	stage_name = name;
}

const QString &EditorCacheHash::getStageName() {
	return stage_name;
}

bool EditorCacheHash::compareFileHash(QString name, EditorHash hash) {
	if (file_hashes.contains(name)) {
		return file_hashes[name] == hash;
	}

	return false;
}

void EditorCacheHash::readDocument(QDomElement &element) {
	stage_name = element.attribute("stageName");

	QDomElement child_element = element.firstChildElement();
	while (!child_element.isNull()) {
		if (child_element.tagName() == DocumentHashRoot) {
			QString hash_name = child_element.attribute("name");
			QString hash_value = child_element.attribute("value");
			if (!hash_name.isEmpty() && !hash_value.isEmpty()) {
				EditorHash hash(hash_value);
				file_hashes[hash_name] = hash_value;
			}
		}
		child_element = child_element.nextSiblingElement();
	}
}

void EditorCacheHash::writeDocument(QDomDocument &document, QDomElement &element) {
	element.setAttribute("stageName", stage_name);

	QMap<QString, EditorHash>::iterator it;
	for (it = file_hashes.begin(); it != file_hashes.end(); it++) {
		QDomElement child_element = document.createElement(DocumentHashRoot);
		child_element.setAttribute("name", it.key());
		child_element.setAttribute("value", it.value().toString());
		element.appendChild(child_element);
	}
}

/** EditorCache */
const QString EditorCache::DocumentRoot = "EditorCache";
const QString EditorCache::CachePath = "/cache";
const QString EditorCache::CacheHashPath = "hashes.xml";

#ifdef SONICGLVL_LOST_WORLD
const QString EditorCache::ConfigPath = "config.lua";
const QString EditorCache::FarPath = "far";
const QString EditorCache::MiscPath = "misc";
const QString EditorCache::ObjPath = "obj";
const QString EditorCache::SkyPath = "sky";
const QString EditorCache::TrrCmnPath = "trr_cmn";
const QString EditorCache::PacExtension = ".pac";
#endif

EditorCache::EditorCache(QString program_path) {
	path = program_path + CachePath;
	QDir().mkpath(path);
}

bool EditorCache::loadHashes() {
	QFile file(hashFilename());
	if (file.open(QFile::ReadOnly | QFile::Text)) {
		QDomDocument document(DocumentRoot);
		if (document.setContent(&file)) {
			QDomElement document_element = document.documentElement();
			if (document_element.tagName() == DocumentRoot) {
				// Read all hashes
				QDomElement child_element = document_element.firstChildElement();
				while (!child_element.isNull()) {
					if (child_element.tagName() == EditorCacheHash::DocumentRoot) {
						EditorCacheHash cache_hash;
						cache_hash.readDocument(child_element);
						hashes.append(cache_hash);
					}
					child_element = child_element.nextSiblingElement();
				}
				return true;
			}
		}
		else {
			// FIXME: LOG XML FAILED
		}
		
		file.close();
	}
	else {
		// FIXME: LOG FILE FAILED
	}

	return false;
}

bool EditorCache::saveHashes() {
	QFile file(hashFilename());
	if (file.open(QFile::WriteOnly | QFile::Text)) {
		QTextStream out(&file);
		QDomDocument document(DocumentRoot);
		QDomElement root = document.createElement(DocumentRoot);

		foreach(EditorCacheHash hash, hashes) {
			QDomElement element = document.createElement(EditorCacheHash::DocumentRoot);
			hash.writeDocument(document, element);
			root.appendChild(element);
		}

		document.appendChild(root);
		document.save(out, SONICGLVL_INDENT);
		return true;
	}
	else {
		// FIXME: LOG FILE FAILED
	}

	return false;
}

QString EditorCache::hashFilename() {
	return absolutePath() + "/" + CacheHashPath;
}

QString EditorCache::absolutePath() {
	return path;
}

void EditorCache::unpackFileSafe(QString stage_name, QString filename, QString logic_name, QProgressDialog &progress) {
	progress.setLabelText(QString("Verifying %1").arg(logic_name));

	// Check if it already exists in cache
	int sha1_hash[5];
#ifdef SONICGLVL_LOST_WORLD
	LibGens::PacSet set(filename.toStdString());
	for (int i=0; i<5; i++) sha1_hash[i] = set.getSHA1Hash(i);
#endif

	EditorHash file_hash(sha1_hash);
	EditorCacheHash &hash = getEditorCacheHash(stage_name);
	bool hash_found = hash.compareFileHash(logic_name, file_hash);

	// Only extract and make the directory if the hash wasn't found already
	if (!hash_found) {
		progress.setLabelText(QString("Extracting %1").arg(logic_name));

		// Make directory
		QString file_directory = stagePath(stage_name) + "/" + logic_name;
		QDir().mkpath(file_directory);

		// Remove all files from directory
		QDir dir(file_directory);
		dir.setNameFilters(QStringList() << "*.*");
		dir.setFilter(QDir::Files);

		QStringList file_list = dir.entryList();
		foreach(QString file, file_list) {
			dir.remove(file);
		}

		// Extract files to directory
#ifdef SONICGLVL_LOST_WORLD
		set.extract(file_directory.toStdString() + "/");
#endif

		hash.addFileHash(logic_name, file_hash);
	}
}

EditorCacheHash &EditorCache::getEditorCacheHash(QString stage_name) {
	for (QList<EditorCacheHash>::iterator it = hashes.begin(); it != hashes.end(); it++) {
		if ((*it).getStageName() == stage_name) {
			return *it;
		}
	}

	EditorCacheHash hash;
	hash.setStageName(stage_name);
	hashes.append(hash);
	return hashes.last();
}

bool EditorCache::unpackStage(QString stage_name, QString path, QWidget *parent) {
	QFileInfo info(path);
	if (info.exists()) {
		QDir dir = info.dir();
		QString dir_base_name = dir.absolutePath();

		int current_progress = 0;
#ifdef SONICGLVL_LOST_WORLD
		int max_progress_count = 5;
#endif
		QProgressDialog progress(QString(), QString(), 0, max_progress_count, parent);
		progress.setWindowTitle("Unpacking Stage to Cache...");
		progress.setWindowModality(Qt::WindowModal);
		progress.setMinimumDuration(0);

#ifdef SONICGLVL_LOST_WORLD
		unpackFileSafe(stage_name, dir_base_name + "/" + stage_name + "_" + FarPath + PacExtension, FarPath, progress);
		progress.setValue(current_progress++);
		unpackFileSafe(stage_name, dir_base_name + "/" + stage_name + "_" + MiscPath + PacExtension, MiscPath, progress);
		progress.setValue(current_progress++);
		unpackFileSafe(stage_name, dir_base_name + "/" + stage_name + "_" + ObjPath + PacExtension, ObjPath, progress);
		progress.setValue(current_progress++);
		unpackFileSafe(stage_name, dir_base_name + "/" + stage_name + "_" + SkyPath + PacExtension, SkyPath, progress);
		progress.setValue(current_progress++);
		unpackFileSafe(stage_name, dir_base_name + "/" + stage_name + "_" + TrrCmnPath + PacExtension, TrrCmnPath, progress);
		progress.setValue(current_progress++);
#endif

		progress.setValue(max_progress_count);

		saveHashes();
	}
	return false;
}

bool EditorCache::packStage(QString stage_name, QString path) {
	return false;
}

QString EditorCache::stagePath(QString stage_name) {
	return absolutePath() + "/" + stage_name;
}


#ifdef SONICGLVL_LOST_WORLD
QString EditorCache::skyPath(QString stage_name) {
	return stagePath(stage_name) + "/" + SkyPath;
}

QString EditorCache::farPath(QString stage_name) {
	return stagePath(stage_name) + "/" + FarPath;
}

QString EditorCache::terrainCommonPath(QString stage_name) {
	return stagePath(stage_name) + "/" + TrrCmnPath;
}
#endif