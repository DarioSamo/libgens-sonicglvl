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

namespace LibGens {
	class KeyframeSet;

	class AnimationSet {
		protected:
			vector<KeyframeSet *> keyframe_sets;
			float fps;
			float start_time;
			float end_time;
			string animation_name;
			float current_frame;
		public:
			AnimationSet();
			void addTime(float time_s);
			float getCurrentValue(unsigned int flag);
			string getName();
			float getFPS();
	};
}