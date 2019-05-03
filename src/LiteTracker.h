// LiteTracker.h
// This is a lightweight version of the Eyelink tracker class (with limited functionality)
// meant for use in tandem with Matlab mex files and the PTB Eyelink toolbox.
// code by Melchi M. Michel (8/23/2013)
#pragma once
#include <cstdio>
#include <string>
#include <core_expt.h>
#include "Point2D.h"

class LiteTracker{
	static LiteTracker *unique_instance;
	Point2D position;
	bool blink_signal;
	static bool is_recording;
	UINT32 tracker_time_offset;// offset in msec between tracker and display computer
	FSAMPLE current_data;// current data sample
	FEVENT current_event; // current event sample
	unsigned int last_sample_time;
	bool refreshDataSample();
	LiteTracker(int tracking_eye=1);
public:
	static int tracking_eye;
	Point2D getGazePosition();
	bool getBlinkSignal();
	~LiteTracker(){}
	static LiteTracker *getInstance(int tracking_eye);
};