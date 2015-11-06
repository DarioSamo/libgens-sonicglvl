#pragma once

#include "ObjectSet.h"

namespace LibGens {
	class Level;

	class LostWorldObjectSet : public ObjectSet {
	public:
		LostWorldObjectSet(string filename_p, ObjectLibrary *library);
		LostWorldObjectSet();
		void readORC(File *file, ObjectLibrary *library);
		void saveORC(Level *level, bool PC);
		void fixTransform(Level *level);
	};
}