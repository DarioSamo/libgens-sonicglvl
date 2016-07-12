//=========================================================================
//	  Copyright (c) 2016 SonicGLvl
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

#include "GIWindow.h"
#include <QDir>
#include <QTemporaryDir>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include "AR.h"
#include "Terrain.h"
#include "TerrainGroup.h"
#include "Model.h"
#include "TerrainInstance.h"
#include "FreeImage.h"
#include "Path.h"

bool GIWindow::convert() {
	if (converter_settings.game_engine == Generations) {
		logProgress(ProgressNormal, "Starting conversion for Generations engine...");
	}
	else if (converter_settings.game_engine == Unleashed) {
		logProgress(ProgressFatal, "No conversion algorithm implemented for Unleashed yet!");
		return false;
	}
	else if (converter_settings.game_engine == LostWorld) {
		logProgress(ProgressFatal, "No conversion algorithm implemented for Lost World yet!");
		return false;
	}

	if (converter_settings.mode == PreRender) {
		logProgress(ProgressNormal, "Converter is operating on Pre-Render mode...");
	}
	else if (converter_settings.mode == PostRender) {
		logProgress(ProgressNormal, "Converter is operating on Post-Render mode...");
	}
	else {
		logProgress(ProgressFatal, "Invalid render mode selected.");
		return false;
	}

	if (converter_settings.terrain_directory.isEmpty()) {
		logProgress(ProgressFatal, "Terrain directory cannot be empty!");
		return false;
	}

	if (converter_settings.mode == PostRender) {
		if (converter_settings.source_gi_directory.isEmpty()) {
			logProgress(ProgressFatal, "Source GI directory cannot be empty!");
			return false;
		}
	}

	QTemporaryDir temp_dir;
	QString temp_path = temp_dir.path();
	logProgress(ProgressNormal, "Storing temporary terrain files in " + temp_path + ".");

	QDir terrain_dir(converter_settings.terrain_directory);
	QString slot_name = terrain_dir.dirName();
	QString terrain_resources_filename = converter_settings.terrain_directory + "/" + slot_name + ".ar.00";
	if (QFileInfo(terrain_resources_filename).exists()) {
		LibGens::ArPack terrain_ar(terrain_resources_filename.toStdString());
		terrain_ar.extract(temp_path.toStdString() + "/");
		logProgress(ProgressNormal, "Extracted " + terrain_resources_filename + " to " + temp_path + ".");
	}
	else {
		logProgress(ProgressFatal, "Couldn't find " + terrain_resources_filename + " for existing terrain. Cannot create GI for an empty stage.");
		return false;
	}

	vector<LibGens::GITextureGroup *> gi_groups;
	QTemporaryDir stage_temp_dir;
	QTemporaryDir stage_add_temp_dir;
	QString stage_temp_path = stage_temp_dir.path();
	QString stage_add_temp_path = stage_add_temp_dir.path();

	QString stage_pfd_filename = converter_settings.terrain_directory + "/Stage.pfd";
	if (!QFileInfo(stage_pfd_filename).exists()) {
		logProgress(ProgressFatal, "Couldn't find " + stage_pfd_filename + " for existing terrain geometry and terrain GI. Cannot create GI for an empty stage.");
		return false;
	}

	QString stage_add_pfd_filename = converter_settings.terrain_directory + "/Stage-Add.pfd";
	if (!QFileInfo(stage_add_pfd_filename).exists()) {
		logProgress(ProgressFatal, "Couldn't find " + stage_add_pfd_filename + " for existing terrain GI. Cannot create GI for an empty stage.");
		return false;
	}

	// Extract existing Stage.pfd into a temporary directory and remove all the gia- files if on Pre-Render mode.
	{
		LibGens::ArPack stage_ar_pack(stage_pfd_filename.toStdString());
		stage_ar_pack.extract(stage_temp_path.toStdString() + "/", ".cab");
		logProgress(ProgressNormal, "Extracted " + stage_pfd_filename + " to " + stage_temp_path + ".");

		QStringList entry_list = QDir(stage_temp_path).entryList(QStringList() << "*.cab");
		foreach(QString entry, entry_list) {
			QString entry_filename = stage_temp_path + "/" + entry;
			expandFileCAB(entry_filename, entry_filename.left(entry_filename.size() - 4));
		}
	}

	// Extract existing Stage-Add.pfd into a temporary directory and  if on Pre-Render mode.
	{
		LibGens::ArPack stage_add_ar_pack(stage_add_pfd_filename.toStdString());
		stage_add_ar_pack.extract(stage_add_temp_path.toStdString() + "/", ".cab");
		logProgress(ProgressNormal, "Extracted " + stage_add_pfd_filename + " to " + stage_add_temp_path + ".");

		QStringList entry_list = QDir(stage_add_temp_path).entryList(QStringList() << "*.cab");
		foreach(QString entry, entry_list) {
			QString entry_filename = stage_add_temp_path + "/" + entry;
			expandFileCAB(entry_filename, entry_filename.left(entry_filename.size() - 4));
		}
	}

	unsigned int total_pre_render_size = 0;
	if (converter_settings.mode == PreRender) {
		LibGens::Path *reference_path = NULL;
		list<pair<LibGens::Node *, LibGens::Spline *>> reference_nodes;
		if (!converter_settings.reference_path_file.isEmpty() && QFileInfo(converter_settings.reference_path_file).exists()) {
			reference_path = new LibGens::Path(converter_settings.reference_path_file.toStdString());
			reference_nodes = reference_path->getNodes();
			logProgress(ProgressNormal, QString("Loaded reference path from %1. Using distance-based generation based on reference path variables. Found %2 nodes.").arg(converter_settings.reference_path_file).arg(reference_nodes.size()));
		}
		else {
			logProgress(ProgressWarning, "Reference path file is either empty or doesn't exist. Defaulting to uniform generation, reference path variables will be ignored.");
		}

		// Remove all the gia- files from the stage folders.
		QStringList entry_list = QDir(stage_temp_path).entryList(QStringList() << "gia-*");
		foreach(QString entry, entry_list) {
			QString entry_filename = stage_temp_path + "/" + entry;
			QFile::remove(entry_filename);
			logProgress(ProgressNormal, QString("Removed %1.").arg(entry_filename));
		}

		entry_list = QDir(stage_add_temp_path).entryList(QStringList() << "gia-*");
		foreach(QString entry, entry_list) {
			QString entry_filename = stage_add_temp_path + "/" + entry;
			QFile::remove(entry_filename);
			logProgress(ProgressNormal, QString("Removed %1.").arg(entry_filename));
		}

		QString terrain_filename = temp_path + "/terrain.terrain";
		if (!QFileInfo(terrain_filename).exists()) {
			logProgress(ProgressFatal, "Couldn't find " + terrain_filename + " for existing terrain configuration file. Cannot create GI for an empty stage.");
			return false;
		}

		logProgress(ProgressNormal, "Loading terrain file " + terrain_filename + "...");
		LibGens::Terrain terrain(terrain_filename.toStdString(), temp_path.toStdString() + "/", temp_path.toStdString() + "/", "");

		vector<LibGens::TerrainGroup *> groups = terrain.getGroups();
		logProgress(ProgressNormal, QString("Loaded %1 terrain groups.").arg(groups.size()));

		// Create Model AABB Map.
		QMap<string, LibGens::AABB> instance_aabbs;
		LibGens::AABB world_aabb;
		world_aabb.reset();

		LibGens::GITextureGroupInfo *gi_group_info = new LibGens::GITextureGroupInfo();
		for (size_t i=0; i<groups.size(); i++) {
			string terrain_group_filename = groups[i]->getName() + LIBGENS_TERRAIN_GROUP_FOLDER_EXTENSION;
			QString terrain_group_path = QString("%1/%2").arg(stage_temp_path).arg(terrain_group_filename.c_str());

			// Read model AABBs
			QMap<string, LibGens::AABB> model_aabbs;
			LibGens::ArPack group_ar_pack(terrain_group_path.toStdString(), false);
			LibGens::File *ar_pack_file = new LibGens::File(terrain_group_path.toStdString(), LIBGENS_FILE_READ_BINARY);

			vector<LibGens::Model *> models = groups[i]->getModels();
			for (size_t m=0; m<models.size(); m++) {
				string model_name = models[m]->getName();
				LibGens::ArFile *ar_file = group_ar_pack.getFile(model_name + LIBGENS_TERRAIN_MODEL_EXTENSION);
				if (ar_file) {
					ar_pack_file->setGlobalOffset(ar_file->getAbsoluteDataAddress());

					LibGens::Model *model = new LibGens::Model(ar_pack_file, true);
					LibGens::AABB aabb = model->getAABB();
					model_aabbs[model_name] = aabb;
					logProgress(ProgressNormal, QString("Loaded %1.terrain-model from group's AR Pack. AABB: [%2, %3, %4][%5, %6, %7]").arg(model_name.c_str()).arg(aabb.start.x).arg(aabb.start.y).arg(aabb.start.z).arg(aabb.end.x).arg(aabb.end.y).arg(aabb.end.z));
					delete model;
				}
				else {
					logProgress(ProgressError, QString("Couldn't find %1.terrain-model in group's AR Pack.").arg(model_name.c_str()));
				}
			}

			list<LibGens::TerrainInstance *> instances = groups[i]->getInstances();
			for (list<LibGens::TerrainInstance *>::iterator it = instances.begin(); it != instances.end(); it++) {
				string instance_name = (*it)->getName();
				if (converter_settings.no_gi_nodes) {
					if (instance_name.find("NoGI") != string::npos) {
						logProgress(ProgressNormal, QString("NoGI detected in %1's name. Skipping this node.").arg(instance_name.c_str()));
						continue;
					}
				}

				LibGens::ArFile *ar_file = group_ar_pack.getFile(instance_name + LIBGENS_TERRAIN_INSTANCE_EXTENSION);
				if (ar_file) {
					ar_pack_file->setGlobalOffset(ar_file->getAbsoluteDataAddress());

					LibGens::TerrainInstance *instance = new LibGens::TerrainInstance(ar_pack_file);
					string model_name = instance->getModelName();
					if (model_aabbs.contains(model_name)) {
						LibGens::Matrix4 instance_matrix = instance->getMatrix();
						LibGens::AABB aabb = model_aabbs[model_name];
						aabb.transform(instance_matrix);
						instance_aabbs[instance_name] = aabb;
						world_aabb.merge(aabb);
						logProgress(ProgressNormal, QString("Loaded %1.terrain-instanceinfo using model %2 from group's AR Pack. AABB: [%3, %4, %5][%6, %7, %8]").arg(instance_name.c_str()).arg(model_name.c_str()).arg(aabb.start.x).arg(aabb.start.y).arg(aabb.start.z).arg(aabb.end.x).arg(aabb.end.y).arg(aabb.end.z));

						gi_group_info->addInstance(instance_name, aabb.center(), aabb.sizeMax() / 2.0f);
					}
					else {
						logProgress(ProgressError, QString("Couldn't find %1 in calculated AABBs for models.").arg(model_name.c_str()));
					}
					delete instance;
				}
				else {
					logProgress(ProgressError, QString("Couldn't find %1.terrain-instanceinfo in group's AR Pack.").arg(instance_name.c_str()));
				}
			}

			ar_pack_file->close();
			delete ar_pack_file;
		}

		QList<string> instance_names = instance_aabbs.keys();
		logProgress(ProgressNormal, QString("Calculated AABBs for %1 instances.").arg(instance_names.size()));

		float world_size_max = world_aabb.sizeMax();
		if (world_aabb.sizeMax() <= 0.0f) {
			logProgress(ProgressFatal, "Empty AABB. Can't generate GI groups for an empty stage.");
			return false;
		}

		QList<RenderItem> render_items;

		// Find next power of two.
		int cc = max(ceil(world_size_max / converter_settings.octree_cell_size), 4.0);
		int old_cc = cc;
		cc = nextPowerOfTwo(cc);

		logProgress(ProgressNormal, QString("Increased max cell count from %1 to %2.").arg(old_cc).arg(cc));

		world_size_max = cc * converter_settings.octree_cell_size;
		LibGens::AABB old_world_aabb = world_aabb;
		world_aabb.addPoint(world_aabb.start + LibGens::Vector3(world_size_max, world_size_max, world_size_max));

		logProgress(ProgressNormal, QString("Old World AABB: [%1, %2, %3][%4, %5, %6].").arg(old_world_aabb.start.x).arg(old_world_aabb.start.y).arg(old_world_aabb.start.z).arg(old_world_aabb.end.x).arg(old_world_aabb.end.y).arg(old_world_aabb.end.z));
		logProgress(ProgressNormal, QString("New World AABB: [%1, %2, %3][%4, %5, %6].").arg(world_aabb.start.x).arg(world_aabb.start.y).arg(world_aabb.start.z).arg(world_aabb.end.x).arg(world_aabb.end.y).arg(world_aabb.end.z));
		createGroups(world_aabb, instance_names, gi_group_info, reference_nodes, render_items, instance_aabbs);

		gi_group_info->sortGroupsByQualityLevel();

		gi_groups = gi_group_info->getGroups();
		size_t gi_groups_size = gi_groups.size();
		logProgress(ProgressNormal, QString("Generated %1 GI Groups.").arg(gi_groups_size));

		for (size_t g = 0; g < gi_groups_size; g++) {
			QTemporaryDir gi_temp_dir;
			QString gi_temp_path = gi_temp_dir.path();
			QDir gi_group_dir(gi_temp_path);

			logProgress(ProgressNormal, QString("Organizing subtextures for group %1 with texture size %2.").arg(g).arg(converter_settings.max_texture_size));
			gi_groups[g]->organizeSubtextures(max(converter_settings.max_atlas_texture_size, converter_settings.max_texture_size));
			logProgress(ProgressNormal, QString("Done organizing subtextures for group %1.").arg(g));

			// Create the atlas textures and save it to the directory
			unsigned int folder_size = 0;
			int texture_index = 0;
			list<LibGens::GITexture *> textures = gi_groups[g]->getTextures();
			for (list<LibGens::GITexture *>::iterator it = textures.begin(); it != textures.end(); it++) {
				unsigned int w = (*it)->getWidth();
				unsigned int h = (*it)->getHeight();

				// Create the bitmap with the debugging colors
				FIBITMAP *atlas_bitmap = FreeImage_Allocate(w, h, 32);

				list<LibGens::GISubtexture *> subtextures = (*it)->getSubtextures();
				for (list<LibGens::GISubtexture *>::iterator it2 = subtextures.begin(); it2 != subtextures.end(); it2++) {
					int start_x = (*it2)->getX() * w;
					int start_y = (*it2)->getY() * h;
					int sub_w = (*it2)->getPixelWidth();
					int sub_h = (*it2)->getPixelHeight();
					QColor color = debugColor(gi_groups[g]->getQualityLevel(), max(sub_w, sub_h));

					RGBQUAD rgbquad;
					rgbquad.rgbRed = color.red();
					rgbquad.rgbGreen = color.green();
					rgbquad.rgbBlue = color.blue();
					rgbquad.rgbReserved = 0;

					for (int x = 0; x < sub_w; x++) {
						for (int y = 0; y < sub_h; y++) {
							FreeImage_SetPixelColor(atlas_bitmap, start_x + x, h - 1 - (start_y + y), &rgbquad);
						}
					}
				}
				
				// Save the bitmap
				QString bitmap_name = QString("%1/%2").arg(gi_temp_path).arg((*it)->getName().c_str());
				QString bitmap_filename = bitmap_name + ".png";
				QString texture_filename = bitmap_name + ".dds";

				FreeImage_Save(FIF_PNG, atlas_bitmap, bitmap_filename.toUtf8().constData());
				FreeImage_Unload(atlas_bitmap);

				// Convert the bitmap to dds
				logProgress(ProgressNormal, QString("Calling NVDXT converter for %1 to %2").arg(bitmap_filename).arg(texture_filename));

				const QString temp_texture = "temp.dds";
				QStringList arguments;
				arguments << "-file" << bitmap_filename << "-output" << temp_texture;

				QProcess conversion_process;
				conversion_process.start("nvdxt", arguments);
				conversion_process.waitForFinished();
				QString conversion_output = conversion_process.readAllStandardOutput();
				logProgress(ProgressNormal, QString("NVDXT Output: " + conversion_output));

				QFile::remove(bitmap_filename);
				QFile::remove(texture_filename);
				if (QFile::copy(temp_texture, texture_filename)) {
					logProgress(ProgressNormal, QString("Converter NVDXT finished with %1 to %2").arg(bitmap_filename).arg(texture_filename));
					folder_size += QFileInfo(texture_filename).size();
				}
				else {
					logProgress(ProgressError, QString("Couldn't copy NVDXT's result to %1. Is this directory write-protected?").arg(texture_filename));
				}

				texture_index++;
			}

			logProgress(ProgressNormal, QString("Saving atlasinfo for group %1.").arg(g));
			gi_groups[g]->setFolderSize(folder_size);
			gi_groups[g]->saveAtlasinfo(gi_temp_path.toStdString() + "/atlasinfo");
			total_pre_render_size += folder_size;

			// Pack GI Group AR
			LibGens::ArPack gi_group_ar_pack;
			QStringList entry_list = gi_group_dir.entryList(QStringList() << "*.dds" << "atlasinfo");
			foreach(QString entry, entry_list) {
				gi_group_ar_pack.addFile((gi_temp_path + "/" + entry).toStdString());
			}

			QString gi_group_name = QString("gia-%1.ar").arg(g);
			if (gi_groups[g]->getQualityLevel() == LIBGENS_GI_TEXTURE_GROUP_LOWEST_QUALITY) {
				gi_group_ar_pack.save((stage_temp_path + "/" + gi_group_name).toStdString());
			}
			else {
				gi_group_ar_pack.save((stage_add_temp_path + "/" + gi_group_name).toStdString());
			}
		}

		string gi_group_info_filename = temp_path.toStdString() + "/gi-texture.gi-texture-group-info";
		gi_group_info->save(gi_group_info_filename);
		logProgress(ProgressNormal, QString("Saved %1.").arg(gi_group_info_filename.c_str()));
		delete gi_group_info;

		//***********************************
		//  Save Render List
		//***********************************
		if (!converter_settings.render_output_file.isEmpty()) {
			QFile render_list_file(converter_settings.render_output_file);
			render_list_file.open(QIODevice::WriteOnly);
			if(render_list_file.isOpen()){
				QTextStream stream(&render_list_file);
				foreach(RenderItem item, render_items) {
					stream << QString("%1 %2\r\n").arg(item.size).arg(item.instance_name);
				}
				render_list_file.close();

				logProgress(ProgressNormal, QString("Wrote render list file to %1 successfully with %2 render items.").arg(converter_settings.render_output_file).arg(render_items.size()));
			}
			else {
				logProgress(ProgressError, QString("Couldn't write render list file to %1. Is this directory write-protected?").arg(converter_settings.render_output_file));
			}
		}
		else {
			logProgress(ProgressWarning, QString("Render-list output name is empty. Didn't sve render list.").arg(converter_settings.render_output_file));
		}

		delete reference_path;
		reference_path = NULL;
	}

	if (converter_settings.mode == PostRender) {
		string gi_group_info_filename = temp_path.toStdString() + "/gi-texture.gi-texture-group-info";

		LibGens::GITextureGroupInfo *gi_group_info = new LibGens::GITextureGroupInfo(gi_group_info_filename);
		gi_groups = gi_group_info->getGroups();
		size_t gi_groups_size = gi_groups.size();
		logProgress(ProgressNormal, QString("Found %1 existing GI Groups.").arg(gi_groups_size));

		for (size_t g = 0; g < gi_groups_size; g++) {
			//******************************************************
			// Read the AR Pack and atlasinfo files for the group.
			//******************************************************
			LibGens::GITextureGroup *group = gi_groups[g];
			unsigned int quality_level = group->getQualityLevel();
			QString quality_level_string = QString("-level%1").arg(quality_level);

			LibGens::ArPack *group_ar_pack = NULL;
			LibGens::File *group_ar_file = NULL;
			string group_ar_filename = "";
			if (quality_level == LIBGENS_GI_TEXTURE_GROUP_LOWEST_QUALITY)
				group_ar_filename = stage_temp_path.toStdString() + "/gia-" + ToString(g) + ".ar";
			else
				group_ar_filename = stage_add_temp_path.toStdString() + "/gia-" + ToString(g) + ".ar";

			logProgress(ProgressNormal, QString("Loading Group's AR Pack from %1.").arg(group_ar_filename.c_str()));
			group_ar_pack = new LibGens::ArPack(group_ar_filename);
			group_ar_file = new LibGens::File(group_ar_filename, LIBGENS_FILE_READ_BINARY);
			
			logProgress(ProgressNormal, QString("Reading atlasinfo file from group AR Pack."));
			LibGens::ArFile *atlasinfo_file = group_ar_pack->getFile(LIBGENS_GI_TEXTURE_GROUP_ATLASINFO_FILE);
			if (atlasinfo_file) {
				group_ar_file->setGlobalOffset(atlasinfo_file->getAbsoluteDataAddress());
				group->readAtlasinfo(group_ar_file);
			}

			group_ar_file->close();
			delete group_ar_file;

			//********************************************************************************
			// Clone all the subtextures, delete the textures, and organize the subtextures.
			//********************************************************************************
			QTemporaryDir group_temp_dir;
			QString group_temp_path = group_temp_dir.path();
			group_ar_pack->extract(group_temp_path.toStdString() + "/");

			// Detect if any of the textures has a sub-texture that can be replaced by one of the ones in the GI import folder.
			// Fill the filename map for both lightmaps and shadowmaps.
			QMap<QString, QString> import_subtexture_lightmaps;
			QMap<QString, QString> import_subtexture_shadowmaps;
			list<LibGens::GITexture *> gi_textures = group->getTextures();

			// Scan for any independent textures and create textures for them in the group since the atlasinfo file does not load them.
			list<LibGens::GITexture *> gi_textures_to_delete;
			QStringList independent_textures = QDir(group_temp_path).entryList(QStringList() << QString("*%1.dds").arg(quality_level_string));
			foreach(QString texture_name, independent_textures) {
				QFileInfo info(texture_name);
				texture_name = info.baseName();

				LibGens::GITexture *texture = new LibGens::GITexture();
				texture->setName(texture_name.toStdString());

				LibGens::GISubtexture *subtexture = new LibGens::GISubtexture();
				subtexture->setName(texture_name.toStdString());
				subtexture->setX(0.0f);
				subtexture->setY(0.0f);
				subtexture->setWidth(1.0f);
				subtexture->setHeight(1.0f);
				texture->addSubtexture(subtexture);
				gi_textures.push_back(texture);
				gi_textures_to_delete.push_back(texture);
				logProgress(ProgressNormal, QString("Found independent texture %1.dds for this group. Added it to processing list.").arg(texture_name));
			}

			for (list<LibGens::GITexture *>::iterator it = gi_textures.begin(); it != gi_textures.end(); it++) {
				list<LibGens::GISubtexture *> subtextures = (*it)->getSubtextures();
				for (list<LibGens::GISubtexture *>::iterator it2 = subtextures.begin(); it2 != subtextures.end(); it2++) {
					QString subtexture_name = (*it2)->getName().c_str();
					if (subtexture_name.contains(quality_level_string)) {
						subtexture_name.remove(quality_level_string);
					}

					QString lightmap_filename = converter_settings.source_gi_directory + "/" + subtexture_name + "_lightmap.png";
					QString shadowmap_filename = converter_settings.source_gi_directory + "/" + subtexture_name + "_shadowmap.png";
					if (QFileInfo(lightmap_filename).exists())
						import_subtexture_lightmaps[subtexture_name] = lightmap_filename;

					if (QFileInfo(shadowmap_filename).exists())
						import_subtexture_shadowmaps[subtexture_name] = shadowmap_filename;
				}
			}

			// If any of both maps aren't empty, we start re-generating the atlasfile by merging it with the existing content.
			if (!import_subtexture_lightmaps.isEmpty() || !import_subtexture_shadowmaps.isEmpty() || converter_settings.override_lightmap || converter_settings.override_shadowmap) {
				unsigned int folder_size = 0;
				unsigned int max_atlas_texture_size = converter_settings.max_atlas_texture_size;

				for (list<LibGens::GITexture *>::iterator it = gi_textures.begin(); it != gi_textures.end(); it++) {
					string texture_name = (*it)->getName();
					QString texture_filename = QString("%1/%2.dds").arg(group_temp_path).arg(texture_name.c_str());
					const char *texture_filename_c = texture_filename.toUtf8().constData();

					// Use FreeImage to convert the DDS to a png properly, since Qt doesn't load the alpha channels like it should.
					FIBITMAP *bitmap = NULL;
					FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(texture_filename_c);
					if (fif == FIF_UNKNOWN)
						fif = FreeImage_GetFIFFromFilename(texture_filename_c);

					if (fif != FIF_UNKNOWN) {
						bitmap = FreeImage_Load(fif, texture_filename_c);
					}

					// Load the converter texture and use it to extract information from there.
					if (bitmap) {
						unsigned int texture_width = FreeImage_GetWidth(bitmap);
						unsigned int texture_height = FreeImage_GetHeight(bitmap);
						logProgress(ProgressNormal, QString("Loaded %1.dds for this group. Texture size is %2x%3.").arg(texture_name.c_str()).arg(texture_width).arg(texture_height));
						(*it)->setWidth(texture_width);
						(*it)->setHeight(texture_height);

						// Extract all subtextures from the existing image into their own textures and store them in the folder.
						list<LibGens::GISubtexture *> subtextures = (*it)->getSubtextures();
						for (list<LibGens::GISubtexture *>::iterator it2 = subtextures.begin(); it2 != subtextures.end(); it2++) {
							QString subtexture_name = (*it2)->getName().c_str();
							if (subtexture_name.contains(quality_level_string)) {
								subtexture_name.remove(quality_level_string);
							}

							QString target_lightmap_filename = group_temp_path + "/" + subtexture_name + "_lightmap.png";
							QString target_shadowmap_filename = group_temp_path + "/" + subtexture_name + "_shadowmap.png";

							// If we have a lightmap to import, copy that instead.
							unsigned int subtexture_width = texture_width * (*it2)->getWidth();
							unsigned int subtexture_height = texture_height * (*it2)->getHeight();
							(*it2)->setPixelWidth(subtexture_width);
							(*it2)->setPixelHeight(subtexture_height);
							if (subtexture_width && subtexture_height) {
								unsigned int scaled_width = subtexture_width;
								unsigned int scaled_height = subtexture_height;
								logProgress(ProgressNormal, QString("Processing %1.").arg(subtexture_name));

								for (int step = 0; step < 2; step++) {
									// Do shadowmap first so lightmap gets scaled to that.
									bool do_shadowmap = converter_settings.scale_to_shadowmap ? (step == 0) : (step == 1);
								
									if (do_shadowmap) {
										// Import and scale existing image from import folder.
										if (converter_settings.override_shadowmap) {
											QImage override_image(scaled_width, scaled_height, QImage::Format_RGBA8888);
											int alpha = converter_settings.inverted_shadowmaps ? (255 - converter_settings.override_shadowmap_a) : converter_settings.override_shadowmap_a;
											override_image.fill(QColor(alpha, alpha, alpha));
											override_image.save(target_shadowmap_filename);
										}
										else if (import_subtexture_shadowmaps.contains(subtexture_name)) {
											logProgress(ProgressNormal, QString("Importing shadowmap texture from %1 into %2...").arg(import_subtexture_shadowmaps[subtexture_name]).arg(target_shadowmap_filename));
											QFile::copy(import_subtexture_shadowmaps[subtexture_name], target_shadowmap_filename);

											QImage import_image;
											if (import_image.load(target_shadowmap_filename)) {
												int import_image_width = import_image.width();
												int import_image_height = import_image.height();

												if (converter_settings.scale_to_shadowmap) {
													int downscale_factor = (1 << quality_level);
													scaled_width = import_image_width / downscale_factor;
													scaled_height = import_image_height / downscale_factor;
												}
											
												if ((scaled_width != import_image_width) || (scaled_height != import_image_height)) {
													QImage scaled_image = import_image.scaled(scaled_width, scaled_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
													scaled_image.save(target_shadowmap_filename);
												}
											}
										}
										// Otherwise, extract the existing content of the image.
										else {
											logProgress(ProgressNormal, QString("Extracting shadowmap texture into %1...").arg(target_shadowmap_filename));
											QImage extract_image(subtexture_width, subtexture_height, QImage::Format_RGBA8888);
											int start_x = texture_width * (*it2)->getX();
											int start_y = texture_height * (*it2)->getY();

											// Turn the alpha into the RGB channel. (And also take inverted shadowmaps into account)
											int image_width = extract_image.width();
											int image_height = extract_image.height();
											for (int x = 0; x < image_width; x++) {
												for (int y = 0; y < image_height; y++) {
													// FreeImage works on bottom-left as 0, 0
													RGBQUAD quad;
													FreeImage_GetPixelColor(bitmap, start_x + x, texture_height - 1 - (start_y + y), &quad);
												
													int alpha = converter_settings.inverted_shadowmaps ? (255 - quad.rgbReserved) : quad.rgbReserved;
													extract_image.setPixel(x, y, QColor(alpha, alpha, alpha).rgb());
												}
											}

											// Scale the extracted image if it's different than the desired width or height.
											if ((scaled_width != extract_image.width()) || (scaled_height != extract_image.height()))
												extract_image = extract_image.scaled(scaled_width, scaled_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

											extract_image.save(target_shadowmap_filename);
										}
									}
									else {
										// Import and scale existing image from import folder.
										if (converter_settings.override_lightmap) {
											QImage override_image(scaled_width, scaled_height, QImage::Format_RGBA8888);
											override_image.fill(QColor(converter_settings.override_lightmap_r, converter_settings.override_lightmap_g, converter_settings.override_lightmap_b));
											override_image.save(target_lightmap_filename);
										}
										else if (import_subtexture_lightmaps.contains(subtexture_name)) {
											logProgress(ProgressNormal, QString("Importing lightmap texture from %1 into %2...").arg(import_subtexture_lightmaps[subtexture_name]).arg(target_lightmap_filename));
											QFile::copy(import_subtexture_lightmaps[subtexture_name], target_lightmap_filename);

											QImage import_image;
											if (import_image.load(target_lightmap_filename)) {
												int import_image_width = import_image.width();
												int import_image_height = import_image.height();

												if (converter_settings.scale_to_lightmap) {
													int downscale_factor = (1 << quality_level);
													scaled_width = import_image_width / downscale_factor;
													scaled_height = import_image_height / downscale_factor;
												}
											
												if ((scaled_width != import_image_width) || (scaled_height != import_image_height)) {
													QImage scaled_image = import_image.scaled(scaled_width, scaled_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
													scaled_image.save(target_lightmap_filename);
												}
											}
										}
										// Otherwise, extract the existing content of the image.
										else {
											logProgress(ProgressNormal, QString("Extracting lightmap texture into %1...").arg(target_lightmap_filename));
											QImage extract_image(subtexture_width, subtexture_height, QImage::Format_RGBA8888);
											int start_x = texture_width * (*it2)->getX();
											int start_y = texture_height * (*it2)->getY();
											int image_width = extract_image.width();
											int image_height = extract_image.height();
											for (int x = 0; x < image_width; x++) {
												for (int y = 0; y < image_height; y++) {
													// FreeImage works on bottom-left as 0, 0
													RGBQUAD quad;
													FreeImage_GetPixelColor(bitmap, start_x + x, texture_height - 1 - (start_y + y), &quad);
													extract_image.setPixel(x, y, QColor(quad.rgbRed, quad.rgbGreen, quad.rgbBlue).rgb());
												}
											}

											// Scale the extracted image if it's different than the desired width or height.
											if ((scaled_width != extract_image.width()) || (scaled_height != extract_image.height()))
												extract_image = extract_image.scaled(scaled_width, scaled_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

											extract_image.save(target_lightmap_filename);
										}
									}
								}
							
								// Once we have the final width and height, we clone the subtexture with the desired size.
								LibGens::GISubtexture *clone_subtexture = new LibGens::GISubtexture();
								clone_subtexture->setPixelWidth(scaled_width);
								clone_subtexture->setPixelHeight(scaled_height);
								clone_subtexture->setName(subtexture_name.toStdString());
								max_atlas_texture_size = max(max_atlas_texture_size, scaled_width);
								max_atlas_texture_size = max(max_atlas_texture_size, scaled_height);
								group->addSubtextureToOrganize(clone_subtexture);
								logProgress(ProgressNormal, QString("Created subtexture with size %1x%2 for %3.").arg(scaled_width).arg(scaled_height).arg(subtexture_name));
							}
							else {
								logProgress(ProgressError, QString("Subtexture %1 has an invalid width and height %2x%3.").arg(subtexture_name).arg(subtexture_width).arg(subtexture_height));
							}
						}

						FreeImage_Unload(bitmap);
					}
					else {
						logProgress(ProgressError, QString("Couldn't load %1.dds for this group. Skipping texture.").arg(texture_name.c_str()));
					}
				}

				logProgress(ProgressNormal, QString("Organizing subtextures for group #%1 with size %2.").arg(g).arg(max_atlas_texture_size));
				group->deleteTextures();
				group->organizeSubtextures(max_atlas_texture_size);

				QTemporaryDir new_group_dir;
				QString new_group_temp_path = new_group_dir.path();

				logProgress(ProgressNormal, QString("Saving textures and atlasinfo file..."));
				gi_textures = group->getTextures();
				for (list<LibGens::GITexture *>::iterator it = gi_textures.begin(); it != gi_textures.end(); it++) {
					QString atlas_image_filename = QString("%1/%2.png").arg(new_group_temp_path).arg((*it)->getName().c_str());
					QString atlas_texture_filename = QString("%1/%2.dds").arg(new_group_temp_path).arg((*it)->getName().c_str());

					int atlas_width = (*it)->getWidth();
					int atlas_height = (*it)->getHeight();

					FIBITMAP *atlas_bitmap = FreeImage_Allocate(atlas_width, atlas_height, 32);

					list<LibGens::GISubtexture *> subtextures = (*it)->getSubtextures();
					for (list<LibGens::GISubtexture *>::iterator it2 = subtextures.begin(); it2 != subtextures.end(); it2++) {
						QString subtexture_name = (*it2)->getName().c_str();
						if (subtexture_name.contains(quality_level_string)) {
							subtexture_name.remove(quality_level_string);
						}

						QString target_lightmap_filename = group_temp_path + "/" + subtexture_name + "_lightmap.png";
						QString target_shadowmap_filename = group_temp_path + "/" + subtexture_name + "_shadowmap.png";
						QImage lightmap_image;
						QImage shadowmap_image;
						if (lightmap_image.load(target_lightmap_filename) && shadowmap_image.load(target_shadowmap_filename)) {
							if ((lightmap_image.width() == shadowmap_image.width()) && (lightmap_image.height() == shadowmap_image.height())) {
								int target_x = (*it2)->getX() * atlas_width;
								int target_y = (*it2)->getY() * atlas_height;
								int width = lightmap_image.width();
								int height = lightmap_image.height();
								logProgress(ProgressNormal, QString("Merging lightmap and shadowmap textures for %1 at [%2, %3][%4, %5]...").arg(subtexture_name).arg(target_x).arg(target_y).arg(width).arg(height));

								for (int x = 0; x < width; x++) {
									for (int y = 0; y < height; y++) {
										QColor lightmap_color = lightmap_image.pixel(x, y);
										QColor shadowmap_color = shadowmap_image.pixel(x, y);
										int alpha = min((shadowmap_color.red() + shadowmap_color.green() + shadowmap_color.blue()) / 3, 255);
										if (converter_settings.inverted_shadowmaps)
											alpha = 255 - alpha;

										RGBQUAD rgbquad;
										rgbquad.rgbRed = lightmap_color.red();
										rgbquad.rgbGreen = lightmap_color.green();
										rgbquad.rgbBlue = lightmap_color.blue();
										rgbquad.rgbReserved = alpha;
										FreeImage_SetPixelColor(atlas_bitmap, target_x + x, atlas_height - 1 - (target_y + y), &rgbquad);
									}
								}
							}
							else {
								logProgress(ProgressError, QString("Lightmap and Shadowmap sizes are somehow different! %1x%2 vs %3x%4").arg(lightmap_image.width()).arg(lightmap_image.height()).arg(shadowmap_image.width()).arg(shadowmap_image.height()));
							}
						}
						else {
							logProgress(ProgressError, QString("Couldn't load %1 and/or %2. Cannot merge into GI Atlas texture.").arg(target_lightmap_filename).arg(target_shadowmap_filename));
						}
					}

					FreeImage_Save(FIF_PNG, atlas_bitmap, atlas_image_filename.toUtf8().constData());
					FreeImage_Unload(atlas_bitmap);

					const QString temp_texture = "temp.dds";
					QStringList arguments;
					arguments << "-file" << atlas_image_filename << "-output" << temp_texture;

					logProgress(ProgressNormal, QString("Converting with NVDXT " + atlas_image_filename));
					QProcess conversion_process;
					conversion_process.start("nvdxt", arguments);
					conversion_process.waitForFinished();
					QString conversion_output = conversion_process.readAllStandardOutput();
					logProgress(ProgressNormal, QString("NVDXT Output: " + conversion_output));
					QFile::remove(atlas_texture_filename);
					if (QFile::copy(temp_texture, atlas_texture_filename)) {
						logProgress(ProgressNormal, QString("Converter NVDXT finished with %1 to %2").arg(atlas_image_filename).arg(atlas_texture_filename));
						folder_size += QFileInfo(atlas_texture_filename).size();
					}
					else {
						logProgress(ProgressError, QString("Couldn't copy NVDXT's result to %1. Is this directory write-protected?").arg(atlas_texture_filename));
					}
				}
				
				logProgress(ProgressNormal, QString("Updating folder size from %1 to %2.").arg(group->getFolderSize()).arg(folder_size));
				group->setFolderSize(folder_size);
				group->saveAtlasinfo(new_group_temp_path.toStdString() + "/atlasinfo");

				// Pack GI Group AR
				LibGens::ArPack gi_group_ar_pack;
				QStringList entry_list = QDir(new_group_temp_path).entryList(QStringList() << "*.dds" << "atlasinfo");
				foreach(QString entry, entry_list) {
					gi_group_ar_pack.addFile((new_group_temp_path + "/" + entry).toStdString());
				}

				logProgress(ProgressNormal, QString("Packing Group #%1's AR Pack.").arg(g));
				QString gi_group_name = QString("gia-%1.ar").arg(g);
				if (quality_level == LIBGENS_GI_TEXTURE_GROUP_LOWEST_QUALITY) {
					gi_group_ar_pack.save((stage_temp_path + "/" + gi_group_name).toStdString());
				}
				else {
					gi_group_ar_pack.save((stage_add_temp_path + "/" + gi_group_name).toStdString());
				}
			}

			for (list<LibGens::GITexture *>::iterator it = gi_textures_to_delete.begin(); it != gi_textures_to_delete.end(); it++) {
				delete *it;
			}
			gi_textures_to_delete.clear();

			delete group_ar_pack;
		}

		gi_group_info->save(gi_group_info_filename);
		logProgress(ProgressNormal, QString("Saved %1.").arg(gi_group_info_filename.c_str()));
		delete gi_group_info;
	}

	//***********************************
	//  Save Terrain Packs and resources
	//***********************************

	// Create output path
	QDir().mkpath(converter_settings.terrain_directory);

	// Packing routines for each game engine
	if (converter_settings.game_engine == Generations) {
		logProgress(ProgressNormal, "Packing for Generations engine...");
		bool pack_result = packGenerations(converter_settings.terrain_directory, slot_name, temp_path, stage_temp_path, stage_add_temp_path);
		return pack_result;
	}
	else if (converter_settings.game_engine == Unleashed) {
		logProgress(ProgressFatal, "Packing for Unleashed game engine not implemented yet!");
		return false;
	}
	else if (converter_settings.game_engine == LostWorld) {
		logProgress(ProgressFatal, "Packing for Lost World game engine not implemented yet!");
		return false;
	}
	else {
		logProgress(ProgressFatal, "No valid game engine detected.");
	}

	if (converter_settings.mode == PreRender) {
		float mb_size = total_pre_render_size / 1024.0 / 1024.0;
		logProgress(ProgressNormal, QString("Generated %1 MB of GI in total.").arg(mb_size));
	}

	return true;
}

QList<GIWindow::GIGroup> GIWindow::createGroups(LibGens::AABB current_aabb, QList<string> instances, LibGens::GITextureGroupInfo *gi_group_info, LibGens::PathNodeList &reference_nodes, QList<RenderItem> &render_items, const QMap<string, LibGens::AABB> &instance_aabbs) {
	QList<GIWindow::GIGroup> return_groups;
	int instances_size = instances.size();
	if (instances_size <= 0)
		return return_groups;

	float size_max = current_aabb.sizeMax();
	logProgress(ProgressNormal, QString("Size of group: %1 with %2 instances.").arg(size_max).arg(instances_size));

	// Find the closest distance to the AABB's center.
	float distance_path = reference_nodes.size() ? FLT_MAX : 0.0f;
	for (LibGens::PathNodeList::iterator it = reference_nodes.begin(); it != reference_nodes.end(); it++) {
		float node_distance = FLT_MAX;
		(*it).first->findClosestPoint((*it).second, current_aabb.center(), &node_distance);
		distance_path = min(distance_path, node_distance);
	}

	// Quality scale factor is based on the distance to the path divided by the distance interval, truncated to an integer plus 1.
	// Scale factor 1 is the default cell size and no downscaling of the texture size multiplier.
	int quality_scale_factor = max(distance_path / converter_settings.reference_distance_interval, 0.0);
	quality_scale_factor = min(quality_scale_factor + 1, converter_settings.reference_max_intervals);
	if (reference_nodes.size()) {
		logProgress(ProgressNormal, QString("Distance to reference path from AABB center is %1. The current quality scale factor is %2.").arg(distance_path).arg(quality_scale_factor));
	}

	double cell_size = converter_settings.octree_cell_size * pow(2.0, (quality_scale_factor - 1));

	// Create quality level 0 group if below the cell size.
	if (size_max < (cell_size * 2.0f)) {
		while (instances.size()) {
			GIGroup group;
			group.aabb.reset();

			LibGens::GITextureGroup *gi_group = gi_group_info->createGroup();
			gi_group->setQualityLevel(0);

			int instance_max_size = min(65535, instances.size());
			for (int i = 0; i < instance_max_size; i++) {
				string instance_name = instances[i];
				int instance_index = gi_group_info->getInstanceIndex(instance_name);
				LibGens::AABB aabb = instance_aabbs[instance_name];
				float sphere_size = aabb.sizeMax() / 2.0F;

				// Divide the quality size multiplier by the current scale factor.
				int texture_size = (int)(converter_settings.quality_size_multiplier / quality_scale_factor * sphere_size);
				texture_size = nextPowerOfTwo(texture_size);
				texture_size = texture_size >> 1;
				texture_size = min(converter_settings.max_texture_size, texture_size);
				texture_size = max(texture_size, 4);
				gi_group->addInstanceIndex(instance_index);

				LibGens::GISubtexture *subtexture = new LibGens::GISubtexture();
				subtexture->setName(instances[i]);
				subtexture->setPixelWidth(texture_size);
				subtexture->setPixelHeight(texture_size);
				gi_group->addSubtextureToOrganize(subtexture);
				logProgress(ProgressNormal, QString("Added #%1 instance (%2) index with texture size %3x%4. (Sphere Size: %5) Sub-Index: %6 [%7, %8, %9][%10, %11, %12]").arg(instance_index).arg(instance_name.c_str()).arg(texture_size).arg(texture_size).arg(sphere_size).arg(i).arg(aabb.start.x).arg(aabb.start.y).arg(aabb.start.z).arg(aabb.end.x).arg(aabb.end.y).arg(aabb.end.z));

				group.aabb.merge(aabb);

				// Add to render list
				if (!converter_settings.render_output_file.isEmpty()) {
					RenderItem item;
					item.instance_name = instance_name.c_str();
					item.size = texture_size;
					render_items.append(item);
				}
			}
			instances.erase(instances.begin(), instances.begin() + instance_max_size);

			int group_index = gi_group_info->getGroupIndex(gi_group);
			group.level = 0;
			group.index = group_index;
			group.instance_count = instance_max_size;
			gi_group->setCenter(group.aabb.center());
			gi_group->setRadius(group.aabb.sizeMax() / 2.0f);

			logProgress(ProgressNormal, QString("Created group #%1 with %2 instances and Sphere %3 %4 %5 - %6. [%7, %8, %9][%10, %11, %12]").arg(group_index).arg(instance_max_size).arg(gi_group->getCenter().x).arg(gi_group->getCenter().y).arg(gi_group->getCenter().z).arg(gi_group->getRadius()).arg(group.aabb.start.x).arg(group.aabb.start.y).arg(group.aabb.start.z).arg(group.aabb.end.x).arg(group.aabb.end.y).arg(group.aabb.end.z));
			return_groups.append(group);
		}

		return return_groups;
	}
	// Split groups if above the desired cell size.
	else {
		logProgress(ProgressNormal, QString("Splitting %1 instances on [%2, %3, %4][%5, %6, %7].").arg(instances_size).arg(current_aabb.start.x).arg(current_aabb.start.y).arg(current_aabb.start.z).arg(current_aabb.end.x).arg(current_aabb.end.y).arg(current_aabb.end.z));

		LibGens::AABB group_aabbs[8];
		for (int corner = 0; corner < 8; corner++) {
			LibGens::AABB group_aabb = current_aabb.half(LIBGENS_MATH_AXIS_X, (corner & 0x1) > 0);
			group_aabb = group_aabb.half(LIBGENS_MATH_AXIS_Y, (corner & 0x2) > 0);
			group_aabb = group_aabb.half(LIBGENS_MATH_AXIS_Z, (corner & 0x4) > 0);
			group_aabbs[corner] = group_aabb;

			logProgress(ProgressNormal, QString("Group Corner AABB #%1: [%2, %3, %4][%5, %6, %7].").arg(corner).arg(group_aabb.start.x).arg(group_aabb.start.y).arg(group_aabb.start.z).arg(group_aabb.end.x).arg(group_aabb.end.y).arg(group_aabb.end.z));
		}

		// Build a list of the best-fitting instances for each corner
		QList<string> corner_instances[8];
		for (int i = 0; i < instances_size; i++) {
			string instance_name = instances[i];
			LibGens::AABB instance_aabb = instance_aabbs[instance_name];
			int winner_corner = 0;
			float winner_size = 0.0f;
			for (int corner = 0; corner < 8; corner++) {
				float intersection_size = group_aabbs[corner].intersection(instance_aabb).size();
				if (intersection_size > winner_size) {
					winner_corner = corner;
					winner_size = intersection_size;
				}
			}

			corner_instances[winner_corner].append(instance_name);
		}

		instances.clear();

		QList<GIGroup> created_groups;
		// Create groups on all 8 corners
		for (int corner = 0; corner < 8; corner++) {
			created_groups += createGroups(group_aabbs[corner], corner_instances[corner], gi_group_info, reference_nodes, render_items, instance_aabbs);
		}

		// Merge all the created groups into new groups
		// Go through quality levels 0 and 1 and create groups above their quality level as necessary.
		for (int ql = 0; ql < 2; ql++) {
			QList<GIGroup> groups_in_level;
			foreach(GIGroup create_group, created_groups) {
				if (create_group.level == ql) {
					groups_in_level.append(create_group);
				}
			}

			if (groups_in_level.size()) {
				logProgress(ProgressNormal, QString("Detected %1 groups with quality level %2. Creating upper group for them.").arg(groups_in_level.size()).arg(ql));

				while (groups_in_level.size()) {
					LibGens::GITextureGroup *gi_group = gi_group_info->createGroup();
					gi_group->setQualityLevel(ql + 1);
				
					GIGroup group;
					group.aabb.reset();
					group.instance_count = 0;
					group.level = ql + 1;
					group.index = gi_group_info->getGroupIndex(gi_group);
				
					QList<GIGroup> leftover_groups;
					foreach(GIGroup group_in_level, groups_in_level) {
						if (group_in_level.instance_count) {
							if ((group.instance_count + group_in_level.instance_count) <= 65535) {
								LibGens::GITextureGroup *sub_group = gi_group_info->getGroupByIndex(group_in_level.index);
								if (sub_group)
									gi_group->addSubtextureToOrganize(sub_group, 0.5);

								gi_group->addInstanceIndex(group_in_level.index);
								group.instance_count += group_in_level.instance_count;
								group.aabb.merge(group_in_level.aabb);
							}
							else {
								leftover_groups.append(group_in_level);
								logProgress(ProgressNormal, QString("Cannot add %1 to this group with %2 instances.").arg(group_in_level.instance_count).arg(group.instance_count));
							}
						}
						else {
							logProgress(ProgressNormal, QString("Group with index %1 has no instances.").arg(group_in_level.index));
						}
					}

					if (!leftover_groups.isEmpty())
						logProgress(ProgressNormal, QString("Groups leftover: %1.").arg(leftover_groups.size()));

					groups_in_level = leftover_groups;

					gi_group->setCenter(group.aabb.center());
					gi_group->setRadius(group.aabb.sizeMax() / 2.0f);
					logProgress(ProgressNormal, QString("Created group #%1 with quality level %2 and %3 sub-groups.").arg(group.index).arg(group.level).arg(gi_group->getInstanceIndexCount()));
					return_groups.append(group);
				}
			}
		}


		return return_groups;
	}
}

unsigned int GIWindow::nextPowerOfTwo(unsigned int v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

void GIWindow::compressFileCAB(QString filename) {
	QStringList arguments;
	arguments << filename << filename;
	QProcess conversion_process;
	conversion_process.start("makecab", arguments);
	conversion_process.waitForFinished();
	QString conversion_output = conversion_process.readAllStandardOutput();
	logProgress(ProgressNormal, QString("Cabinet Maker Output: " + conversion_output));
}

void GIWindow::expandFileCAB(QString filename, QString new_filename) {
	logProgress(ProgressNormal, "Decompressing " + filename + "...");
	QStringList arguments;
	arguments << filename << new_filename;
	QProcess decompression_process;
	decompression_process.start("expand", arguments);
	decompression_process.waitForFinished();
	QString decompression_output = decompression_process.readAllStandardOutput();
	logProgress(ProgressNormal, QString("Expand Cabinet Output: " + decompression_output));
	QFile::remove(filename);
}

bool GIWindow::packGenerations(QString output_path, QString output_name, QString path, QString stage_path, QString stage_add_path) {
	// Pack Stage.pfd
	{
		logProgress(ProgressNormal, "Saving Stage.pfd from " + stage_path + "...");
		LibGens::ArPack stage_ar_pack;
		QStringList entry_list = QDir(stage_path).entryList(QStringList() << "*.ar");
		foreach(QString entry, entry_list) {
			// Compress all AR files that have gia in their filename, since they were just created by the converter.
			QString entry_filename = (stage_path + "/" + entry);
			compressFileCAB(entry_filename);
			stage_ar_pack.addFile(entry_filename.toStdString());
			logProgress(ProgressNormal, "Added " + entry + " to Stage.pfd.");
		}

		QString stage_ar_pack_filename = output_path + "/Stage.pfd";
		stage_ar_pack.save(stage_ar_pack_filename.toStdString());
		stage_ar_pack.savePFI(path.toStdString() + "/Stage.pfi");
		logProgress(ProgressNormal, "Saved " + stage_ar_pack_filename + ".");
	}

	// Pack Stage-Add.pfd
	{
		logProgress(ProgressNormal, "Saving Stage-Add.pfd from " + stage_add_path + "...");
		LibGens::ArPack stage_add_ar_pack;
		QStringList entry_list = QDir(stage_add_path).entryList(QStringList() << "*.ar");
		foreach(QString entry, entry_list) {
			// Compress all AR files that have gia in their filename, since they were just created by the converter.
			QString entry_filename = (stage_add_path + "/" + entry);
			compressFileCAB(entry_filename);
			stage_add_ar_pack.addFile(entry_filename.toStdString());
			logProgress(ProgressNormal, "Added " + entry + " to Stage-Add.pfd.");
		}

		QString stage_add_ar_pack_filename = output_path + "/Stage-Add.pfd";
		stage_add_ar_pack.save(stage_add_ar_pack_filename.toStdString());
		stage_add_ar_pack.savePFI(path.toStdString() + "/Stage-Add.pfi");
		logProgress(ProgressNormal, "Saved " + stage_add_ar_pack_filename + ".");
	}

	QDir temp_dir(path);

	// Finally, pack the resources directory
	LibGens::ArPack resources_pack;
	QStringList entry_list = temp_dir.entryList(QStringList() << "*");
	foreach(QString entry, entry_list) {
		resources_pack.addFile((path + "/" + entry).toStdString());
	}

	QString resources_pack_filename = QString("%1/%2.ar.00").arg(output_path).arg(output_name);
	resources_pack.save(resources_pack_filename.toStdString());
	logProgress(ProgressNormal, "Saved " + resources_pack_filename + ".");
	return true;
}


QColor GIWindow::debugColor(int quality_level, int size) {
	QColor start, end;
	if (quality_level == 0) {
		start = QColor(255, 128, 0);
		end = QColor(255, 0, 0);
	}
	else if (quality_level == 1) {
		start = QColor(228, 255, 0);
		end = QColor(0, 255, 0);
	}
	else if (quality_level == 2) {
		start = QColor(255, 0, 255);
		end = QColor(0, 0, 255);
	}


	int size_pow = log((float) size) / log(2.0f);
	size_pow -= 5;

	float factor = max(min(size_pow / 6.0f, 1.0f), 0.0f);
	int red = start.red() + (end.red() - start.red()) * factor;
	int green = start.green() + (end.green() - start.green()) * factor;
	int blue = start.blue() + (end.blue() - start.blue()) * factor;
	return QColor(red, green, blue);
}