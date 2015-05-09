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

#include "Error.h"

namespace LibGens {
	bool Error::FileLogging=false;

	int Error::initialize() {
		FILE *fp=fopen(LIBGENS_ERROR_H_FILE_LOG, "wt");
		
		if (fp) {
			fclose(fp);
			return 1;
		}
		else {
			Error::addMessage(FILE_NOT_FOUND, LIBGENS_ERROR_H_FILE_LOG_ERROR);
			return 0;
		}
	}

	void Error::setLogging(bool v) {
		FileLogging = v;
	}

	bool Error::getLogging() {
		return FileLogging;
	}

	string Error::ErrorCodeID(Code id) {
		switch (id) {
			case NULL_REFERENCE :
				return LIBGENS_ERROR_H_CODE_NULL;
			case WARNING :
				return LIBGENS_ERROR_H_CODE_WARNING;
			case FILE_NOT_FOUND :
				return LIBGENS_ERROR_H_CODE_FILE;
			case EXCEPTION :
				return LIBGENS_ERROR_H_CODE_EXCEPTION;
			case FATAL :
				return LIBGENS_ERROR_H_CODE_FATAL;
			case LOG :
				return "";
		}

		return LIBGENS_ERROR_H_CODE_UNKNOWN;
	}


	void Error::addMessage(Code error_code_p, string description_p) {
		if (FileLogging) {
			FILE *fp=fopen(LIBGENS_ERROR_H_FILE_LOG, "wt");
			fseek(fp, SEEK_END, 0L);

			if (fp) {
				fprintf(fp, "%s - %s\n", ErrorCodeID(error_code_p).c_str(), description_p.c_str());
				fclose(fp);
			}
		}
	}

	
	void Error::printfMessage(Code error_code_p, char *msg, ...) {
		va_list fmtargs;
		char buffer[2048];

		va_start(fmtargs,msg);
		vsnprintf(buffer,sizeof(buffer)-1,msg,fmtargs);
		va_end(fmtargs);

		Error::addMessage(error_code_p, buffer);
	}
};