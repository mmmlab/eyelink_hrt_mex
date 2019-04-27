/**
 *  GazeDatum.h - Defines class representing generic gaze data point 
 *
 *  Part of the VisExLib (the Visual Experiment Library)
 *
 *  Copyright (C) 2012-2014 by Melchi M. Michel <melchi.michel@rutgers.edu>
 *
 *  Licensed under GNU General Public License 3.0 or later. 
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */

#pragma once
#include <fstream>
#include <sstream>
#include "Point2D.h"

#ifndef SQR
#define SQR(A) ((A)*(A))
#endif

struct GazeDatum{
	Point2D pos;
	unsigned int time;
	friend std::ostream &operator<<(std::ostream &ss, GazeDatum &gd){
		ss.precision(4);
		ss<<"[ "<<gd.pos.x<<",\t"<<gd.pos.y<<",\t"<<gd.time<<"]";
		return ss;
	}
	GazeDatum(Point2D pos,unsigned int time){
		this->pos = pos;
		this->time = time;
	}
	GazeDatum(): pos(0,0), time(0){}
};
