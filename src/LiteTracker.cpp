// LiteTracker.cpp
// This is a lightweight version of the Eyelink tracker class (with limited functionality)
// meant for use in tandem with Matlab mex files and the PTB Eyelink toolbox.
#include "LiteTracker.h"

// added following to use portable std::chrono for timing
#if __cplusplus > 199711L
	#include<chrono>
	namespace chrono = std::chrono;
#else
// Import boost libraries (unnecessary if compiling under C++11 or later)
	#include <boost/chrono.hpp>
	namespace chrono = boost::chrono;
#endif

using chrono::milliseconds;
typedef chrono::high_resolution_clock hr_clock;

/// Utility functions
unsigned long get_time(){
	hr_clock::time_point chron_tp = hr_clock::now();
	milliseconds chron_ms = chrono::duration_cast<milliseconds>(chron_tp.time_since_epoch());
	return chron_ms.count();
}


///////////////////////////////////////////////////////
////////// Method Definitions /////////////////////////
LiteTracker::LiteTracker(int tracking_eye){
	printf("\n...constructing LiteTracker...\n");
	this->tracking_eye = tracking_eye;
#ifndef SIMULATE_EYETRACKER
	if(!eyelink_is_connected()){
		open_eyelink_connection(0);
	}
#endif //SIMULATE_EYETRACKER
	is_recording = true;
}

LiteTracker *LiteTracker::getInstance(int tracking_eye){
	printf("\n...entered LiteTracker::getInstance()...\n");
	if(unique_instance == NULL){
		unique_instance = new LiteTracker(tracking_eye);
	}
	LiteTracker::tracking_eye = tracking_eye;
	return unique_instance;
}

bool LiteTracker::refreshDataSample(){
	if(eyelink_is_connected()&&(eyelink_newest_float_sample(&current_data)>0)){
		//last_sample_time = timeGetTime();
		last_sample_time = get_time();
		return true;
	}
	return false;
}

Point2D LiteTracker::getGazePosition(){
	double x,y;
#ifndef SIMULATE_EYETRACKER
	if(is_recording&&(get_time()!=last_sample_time)){
		refreshDataSample();
	}
	x = current_data.gx[tracking_eye];
	y = current_data.gy[tracking_eye];
#else
	if(is_recording){
		last_sample_time = get_time();
	}
	x = 0.0;
	y = 0.0;
#endif //SIMULATE_EYETRACKER
	this->position = Point2D(x,y);
	return position;
}
bool LiteTracker::getBlinkSignal(){
#ifndef SIMULATE_EYETRACKER
	if(get_time()!=last_sample_time){
		refreshDataSample();
	}
	this->blink_signal = !(current_data.pa[1]>0.0);	// pa[0] is size (diameter?), pa[1] is area.
	printf("\n...detected blink...\n");				// both values should be zero during a blink.
#else
	blink_signal = false;
#endif //SIMULATE_EYETRACKER
	return blink_signal;							
}

LiteTracker *LiteTracker::unique_instance = NULL;
bool LiteTracker::is_recording;
int LiteTracker::tracking_eye = 1;