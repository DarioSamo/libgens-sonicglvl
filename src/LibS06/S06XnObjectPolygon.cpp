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

#include "LibGens.h"
#include <algorithm>
#include "S06XnFile.h"

namespace LibGens {
	void SonicPolygonPoint::read(File *file, bool big_endian, unsigned char format_flag) {
		file->readInt16E(&position_index, big_endian);

		if (format_flag & 0x1) {
			file->readInt16E(&color_index, big_endian);
		}

		if (format_flag & 0x2) {
			file->moveAddress(2);
			printf("The format flag for the triangle strip has 0x2 enabled, this has not been cracked yet. Report with .gno\n");
			getchar();
		}

		if (format_flag & 0x4) {
			file->readInt16E(&normal_index, big_endian);
		}

		if (format_flag & 0x8) {
			file->moveAddress(2);
			printf("The format flag for the triangle strip has 0x8 enabled, this has not been cracked yet. Report with .gno\n");
			getchar();
		}

		if (format_flag & 0x10) {
			file->readInt16E(&uv_index, big_endian);
		}

		if (format_flag & 0x20) {
			file->readInt16E(&uv_index, big_endian);
			file->readInt16E(&uv2_index, big_endian);
		}
	}

	void SonicPolygonTable::read(File *file, bool big_endian) {
		unsigned int table_count=0;
		size_t table_address=0;

		file->readInt32E(&table_count, big_endian);
		file->readInt32EA(&table_address, big_endian);

		file->goToAddress(table_address);
		file->readInt32E(&flag, big_endian);

		if (table_count == 4) {
			size_t index_table_address=0;
			unsigned int index_table_size=0;
			file->readInt32EA(&index_table_address, big_endian);
			file->readInt32E(&index_table_size, big_endian);
			file->goToAddress(index_table_address);

			unsigned char strip_flag=0;
			unsigned int table_flag_2=0;
			file->moveAddress(20);
			
			file->readUChar(&strip_flag);
			unsigned short face_total = 0;

			Error::addMessage(Error::WARNING, "Reading strip with flag " + ToString((int)strip_flag) + " at address " + ToString(file->getCurrentAddress()));

			while (file->getCurrentAddress() < (index_table_address+index_table_size)) {
				unsigned char strip_type=0;
				printf("Reading strip type at address %d\n", file->getCurrentAddress());
				file->readUChar(&strip_type);

				if (strip_type == 0x99) {
				
					file->readInt16E(&face_total, big_endian);
		
					if ((unsigned int)strip_flag <= 53) {
						SonicPolygonPoint last_point_1;
						SonicPolygonPoint last_point_2;
						SonicPolygonPoint point;
						int count=0;

						for (size_t i=0; i<face_total; i++) {
							last_point_1 = last_point_2;
							last_point_2 = point;
						
							point.read(file, big_endian, strip_flag);
							count++;

							if ((point == last_point_1) || (point == last_point_2) || (last_point_1 == last_point_2)) {
								continue;
							}
			
							if (count >= 3) {
								if (count%2==1) {
									SonicPolygon *polygon=new SonicPolygon(point, last_point_2, last_point_1);
									faces.push_back(polygon);
								}
								else {
									SonicPolygon *polygon=new SonicPolygon(last_point_1, last_point_2, point);
									faces.push_back(polygon);
								}
							}

							if (point.position_index == (unsigned short)0xFFFF) {
								count = 0;
							}
						}
					}
					else {
						printf("Unknown Strip Flag %d at address %d\n", (int) strip_flag, file->getCurrentAddress());
						getchar();
						break;
					}
				}
				else if (strip_type == 0) {
					break;
				}
				else {
					printf("Unknown Strip Type %d at address %d\n", (int) strip_type, file->getCurrentAddress());
					getchar();
					break;
				}
			}
		}
		else if (table_count == 0) {
			unsigned char format_flag=0;
			unsigned int format_size=0;

			if (flag == 0x21000A) {
				format_flag = 1;
				format_size = 4;
			}
			else if (flag == 0x81000A) {
				format_flag = 16;
				format_size = 4;
			}
			else if (flag == 0xE1002A) {
				format_flag = 17;
				format_size = 6;
			}
			else {
				printf("Unknown Strip List Format Type %d at address %d\n", (int) flag, file->getCurrentAddress());
				getchar();
				return;
			}

			unsigned int total_strips=0;
			size_t strips_address=0;
			size_t faces_address=0;
			vector<unsigned short> strip_sizes;
			file->readInt32E(&total_strips, big_endian);
			file->readInt32EA(&strips_address, big_endian);
			file->readInt32EA(&faces_address, big_endian);

			for (size_t i=0; i<total_strips; i++) {
				file->goToAddress(strips_address+i*2);
				unsigned short strip_count=0;
				file->readInt16E(&strip_count, big_endian);
				strip_sizes.push_back(strip_count);
			}

			size_t additional_index=0;
			for (size_t m=0; m<strip_sizes.size(); m++) {
				SonicPolygonPoint last_point_1;
				SonicPolygonPoint last_point_2;
				SonicPolygonPoint point;
				int count=0;

				for (size_t i=additional_index; i<additional_index+strip_sizes[m]; i++) {
					last_point_1 = last_point_2;
					last_point_2 = point;
	
					file->goToAddress(faces_address + i*format_size);
					point.read(file, big_endian, format_flag);
					count++;

					if ((point == last_point_1) || (point == last_point_2) || (last_point_1 == last_point_2)) {
						continue;
					}
			
					if (count >= 3) {
						if (count%2==1) {
							SonicPolygon *polygon=new SonicPolygon(point, last_point_2, last_point_1);
							faces.push_back(polygon);
						}
						else {
							SonicPolygon *polygon=new SonicPolygon(last_point_1, last_point_2, point);
							faces.push_back(polygon);
						}
					}
				}

				additional_index += strip_sizes[m];
			}
		}
		else {
			printf("Unknown Polygon Table Type %d at address %d\n", (int) table_count, table_address);
			getchar();
		}
	}
};
