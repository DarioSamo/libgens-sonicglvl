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

#pragma once

#define LIBGENS_ERROR_H_FILE_LOG          "libgens.log"
#define LIBGENS_ERROR_H_FILE_LOG_ERROR    "Couldn't create libgens.log."
#define LIBGENS_ERROR_H_HEADER_LOG        "-- LibGens Error Log --"
#define LIBGENS_ERROR_H_CODE_NULL         "Error::NULL_REFERENCE"
#define LIBGENS_ERROR_H_CODE_WARNING      "WARNING"
#define LIBGENS_ERROR_H_CODE_FILE         "FILE_NOT_FOUND"
#define LIBGENS_ERROR_H_CODE_EXCEPTION    "EXCEPTION"
#define LIBGENS_ERROR_H_CODE_FATAL        "FATAL"
#define LIBGENS_ERROR_H_CODE_UNKNOWN      "UNKNOWN"

using namespace std;

namespace LibGens {
	class Error {
	public:
		enum Code {
			NULL_REFERENCE,
			WARNING,
			FILE_NOT_FOUND,
			EXCEPTION,
			FATAL,
			LOG
		};

		static bool FileLogging;
		static int initialize();
		static void setLogging(bool v);
		static bool getLogging();
		static string ErrorCodeID(Code id);

		static void addMessage(Code error_code_p, string description_p);
		static void printfMessage(Code error_code_p, char *msg, ...);
	};
};
