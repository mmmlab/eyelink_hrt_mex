// EyelinkHRT.h //
// by Melchi M. Michel 8/2013 - 5/2017
// A light(er) weight--believe it or not--version of the High Resolution Tracker class
// defined in my VisEx (VISual EXperiments) library.

#pragma once  
// This is a header guard. It's non-standard, but supported in MSVC, GCC, and Intel compilers
#include <fstream>
#include <sstream>
#include <vector>
#include "GazeDatum.h"
#include "LiteTracker.h"

#if (__cplusplus > 199711L)
	#include<chrono>
	#include<thread>
	namespace stdx = std;
#else
// Import boost libraries (unnecessary if compiling under C++11 or later)
	#include <boost/chrono.hpp>
	#include <boost/thread.hpp>
	namespace stdx = boost;
#endif

#define _ITERATOR_DEBUG_LEVEL 0

// To Do: Add a state that allows for temporal integration when not recording.

class EyelinkHRT{
	enum HRTState{
		HRT_STOPPED,
		HRT_TRACKING,
		HRT_RECORDING
	};
	enum SaccadeState{
		HRT_FIXATING,
		HRT_SACCADING
	};
private:
	// Private Variables
	static EyelinkHRT *unique_instance;
	static LiteTracker *eyetracker; 
	static HRTState state;
	static SaccadeState saccade_state;
	static stdx::thread *hrtThread;
	static stdx::mutex mutex;
	static stdx::chrono::milliseconds MAX_TRACK_TIME;
	static stdx::chrono::milliseconds temporal_resolution;
	static bool blink_detected;
	static stdx::chrono::high_resolution_clock::time_point start_time;
	static stdx::chrono::milliseconds current_time;
	static Point2D current_pos;
	static Point2D current_velocity;
	static Point2D current_accel;
	static double current_blink_voltage;
	static std::vector<GazeDatum> gaze_data;
	static bool thread_alive;
	static std::vector<GazeDatum> short_gazelist;
	static double saccade_velocity_threshold;

	// Private Methods
	static void updateCurrentVelocity();
	static void updateCurrentVelocityAndAccel();
	static void updateSaccadeState();
	EyelinkHRT(LiteTracker *tracker);

public:

	// Public Methods
	static void track();
	static void startTracking();
	static void stopTracking();
	static void startRecording();
	static void stopRecording();
	static void setTemporalResolution(int ms);
	static GazeDatum getCurrentPos();
	static unsigned int getCurrentTime();
	static Point2D getCurrentPos(unsigned int ms);
	static Point2D getCurrentAcceleration();// in deg/sec^2
	static GazeDatum getCurrentVelocity(); // in deg/sec
	static bool checkForBlink();
	static void resetBlinkDetector();
	static std::vector<GazeDatum> getGazeData();
	//For testing purposes only:
	static void setBlinkDetected(bool b);
	static stdx::mutex* getMutexPtr();
	//////////////////////////////////

	int getFinalTime();

	friend std::ofstream &operator<<(std::ofstream &fs, EyelinkHRT &hrt){
		std::vector<GazeDatum> local_gaze_data = hrt.getGazeData();
		int arraysize = local_gaze_data.size();//*sizeof(GazeDatum);
		GazeDatum* dataPtr = &local_gaze_data[0];
		// Print out (in binary format) the length of the data vector
		// followed by the vector itself
		fs.write(reinterpret_cast<char*>(&arraysize),sizeof(int));
		fs.write(reinterpret_cast<char*>(dataPtr),arraysize*sizeof(GazeDatum));
		return fs;
	}

	friend std::ostringstream &operator<<(std::ostringstream &oss, EyelinkHRT &hrt){
		std::vector<GazeDatum> local_gaze_data = hrt.getGazeData();
		oss<<"\nGaze Position Data:\n"<<std::endl;
		for(size_t i=0;i<hrt.gaze_data.size();++i){
			oss.precision(4);
			oss<<"x = "<<local_gaze_data[i].pos.x<<";\t";
			oss<<"y = "<<local_gaze_data[i].pos.y<<";\t";
			//oss<<"blink = "<<hrt.blink_data[i]<<";\t"; //temporary change
			oss<<"t = "<<local_gaze_data[i].time<<std::endl;
		}
		oss.flush();
		return oss;
	}
	static EyelinkHRT *getInstance(LiteTracker *tracker);
	~EyelinkHRT();
};