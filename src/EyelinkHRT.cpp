// EyelinkHRT.cpp
#include <cstdio>
#include <iostream>
#include <vector>
#include "EyelinkHRT.h"

namespace chrono = stdx::chrono;
using std::ofstream;
using std::ostringstream;
using chrono::milliseconds;
typedef chrono::high_resolution_clock hr_clock;
using std::vector;

//EyelinkHRT member functions

void EyelinkHRT::track(){
	while(thread_alive){
		mutex.lock();
		HRTState localstate = state;
		mutex.unlock();

		if(localstate==HRT_STOPPED){
			// do nothing
			// stdx::this_thread::yield();
		}else if(localstate==HRT_TRACKING){
			mutex.lock();
			current_pos = eyetracker->getGazePosition();
			//start_time = hr_clock::now();
			//////////////////////////
			//// Treat short_gazelist as a limited_capacity stack
			//// and push down the current GazeDatum
			short_gazelist[2] = short_gazelist[1];
			short_gazelist[1] = short_gazelist[0];
			short_gazelist[0] = GazeDatum(current_pos,0);
			updateCurrentVelocityAndAccel();
			//////////////////////////
			mutex.unlock();
		}else if(localstate==HRT_RECORDING){
			mutex.lock();
			// update gaze position information
			current_pos = eyetracker->getGazePosition();
			current_time = chrono::duration_cast<milliseconds>(hr_clock::now()-start_time);
			GazeDatum gd(current_pos,(int) current_time.count());
			gaze_data.push_back(gd);
			// check for blink
			blink_detected = eyetracker->getBlinkSignal();
			if(current_time>MAX_TRACK_TIME){
				stopRecording();
			}
			//////////////////////////
			//// Treat short_gazelist as a limited_capacity stack
			//// and push down the current GazeDatum
			short_gazelist[2] = short_gazelist[1];
			short_gazelist[1] = short_gazelist[0];
			short_gazelist[0] = gd;
			updateCurrentVelocityAndAccel();
			//////////////////////////
			mutex.unlock();
		}

		if(localstate!=HRT_STOPPED){ // i.e., if we're tracking or recording eye movements
			//stdx::this_thread::sleep_for(temporal_resolution);
			//stdx::this_thread::sleep_until(start_time+current_time+temporal_resolution);

			// The two sleep functions above are unreliable, so we have to keep the thread active
			hr_clock::time_point desired_time = start_time+current_time+temporal_resolution;
			while(hr_clock::now()<desired_time){} // explicit wait
		}
	}
}


void EyelinkHRT::startTracking(){
	mutex.lock();
	if(state==HRT_STOPPED){
		gaze_data.clear();
		blink_detected = false;
		state = HRT_TRACKING;
		start_time = hr_clock::now();
		current_time = chrono::duration_cast<milliseconds>(hr_clock::now()-start_time);
	}
	mutex.unlock();
}

void EyelinkHRT::stopTracking(){
	mutex.lock();
	state = HRT_STOPPED;
	mutex.unlock();
}

void EyelinkHRT::startRecording(){
	mutex.lock();
	HRTState localstate = state;
	mutex.unlock();
	if(localstate == HRT_STOPPED){
		startTracking();
	}
	mutex.lock();
	//cout<<"\n...recording eye movements...\n"<<endl;
	temporal_resolution = milliseconds(1);
	state = HRT_RECORDING;
	gaze_data.clear();
	gaze_data.reserve(10000); //i.e., reserve enough space for 10 seconds
	start_time = hr_clock::now();
	current_time = chrono::duration_cast<milliseconds>(hr_clock::now()-start_time);
	mutex.unlock();
}

void EyelinkHRT::stopRecording(){
	mutex.lock();
	if(state==HRT_RECORDING){
		state = HRT_TRACKING;
	}
	blink_detected = false;
	mutex.unlock();
}

void EyelinkHRT::setTemporalResolution(int ms){
	mutex.lock();
	temporal_resolution = milliseconds(ms);
	mutex.unlock();
}

unsigned int EyelinkHRT::getCurrentTime(){
	mutex.lock();
	unsigned int ctime = current_time.count();
	mutex.unlock();
	return ctime;
}

GazeDatum EyelinkHRT::getCurrentPos(){
	mutex.lock();
	GazeDatum temp;
	temp.pos = current_pos;
	temp.time = current_time.count();
	mutex.unlock();
	return temp;
	
}

Point2D EyelinkHRT::getCurrentPos(unsigned int integration_time){
	// This version integrates position across the last 'integration time' ms
	mutex.lock();
	const unsigned int c_time = current_time.count();
	mutex.unlock();
	const unsigned int s_time = c_time-integration_time;
	vector<GazeDatum> data_copy = getGazeData();
	Point2D position_sum(0,0);
	int t = data_copy.size()-1;
	int nr_timesteps = 0;
	// integrate across positions from s_time to c_time
	while(data_copy[t].time>=s_time){
		position_sum = position_sum+data_copy[t].pos;
		++nr_timesteps;
		++t;
	}
	// return mean position
	return position_sum/double(nr_timesteps);	
}

void EyelinkHRT::updateCurrentVelocity(){
	Point2D new_velocity,avg_velocity;
	GazeDatum current_gaze, previous_gaze;
	current_gaze = short_gazelist[0];
	previous_gaze = short_gazelist[1];
	new_velocity = 1000.0*(current_gaze.pos-previous_gaze.pos)/double(current_gaze.time-previous_gaze.time);
	//avg_velocity = 0.75*new_velocity+0.25*current_velocity;
	current_velocity = new_velocity;//avg_velocity;
}

GazeDatum EyelinkHRT::getCurrentVelocity(){
	GazeDatum velocity;
	mutex.lock();
	velocity = GazeDatum(current_velocity,current_time.count());
	mutex.unlock();
	return velocity;
}

void EyelinkHRT::updateCurrentVelocityAndAccel(){
	Point2D v1,v2;
	v1 = 1000.0*((short_gazelist[1].pos)-(short_gazelist[2].pos))/double(short_gazelist[1].time-short_gazelist[2].time);
	v2 = 1000.0*((short_gazelist[0].pos)-(short_gazelist[1].pos))/double(short_gazelist[0].time-short_gazelist[1].time);
	double t1,t2;
	t1 = 0.5*(short_gazelist[1].time + short_gazelist[2].time);
	t2 = 0.5*(short_gazelist[0].time + short_gazelist[1].time);
	current_velocity = 0.75*v2+0.25*v1;
	current_accel = 1000.0*(v2-v1)/(t2-t1);
}

Point2D EyelinkHRT::getCurrentAcceleration(){
	Point2D accel;
	mutex.lock();
	accel = current_accel;
	mutex.unlock();
	return accel;
}

vector<GazeDatum> EyelinkHRT::getGazeData(){
	vector<GazeDatum> data_copy;
	mutex.lock();
	data_copy = gaze_data;
	mutex.unlock();
	return data_copy;
}

bool EyelinkHRT::checkForBlink(){
	mutex.lock();
	bool temp_bool = blink_detected;
	mutex.unlock();
	return temp_bool;
}

void EyelinkHRT::resetBlinkDetector(){
	mutex.lock();
	blink_detected = false;
	mutex.unlock();
}

// For testing purposes only:
void EyelinkHRT::setBlinkDetected(bool b){
	mutex.lock();
	blink_detected = b;
	mutex.unlock();
}
///////////////////////////////////////

int EyelinkHRT::getFinalTime(){
	int final_time;
	mutex.lock();
	final_time = gaze_data[gaze_data.size()-1].time;
	mutex.unlock();
	return final_time;
}

stdx::mutex* EyelinkHRT::getMutexPtr(){
	return &mutex;
}

EyelinkHRT *EyelinkHRT::getInstance(LiteTracker *tracker){
	if(unique_instance == NULL){
		unique_instance = new EyelinkHRT(tracker);
	}
	return unique_instance;
}

EyelinkHRT::EyelinkHRT(LiteTracker *tracker_ptr){
	printf("\n...constructing High Res Tracker...\n");
	eyetracker = tracker_ptr;
	thread_alive = true;
	//hrtThreadPtr = stdx::shared_ptr<stdx::thread>(new stdx::thread(&EyelinkHRT::track));
	hrtThread = new stdx::thread(&EyelinkHRT::track);
	state = HRT_STOPPED;
}

EyelinkHRT::~EyelinkHRT(){
	printf("\n...deleting High Res Tracker...\n");
	printf("\n...deleting HRT thread object...\n");
	mutex.lock();
	thread_alive = false;
	mutex.unlock();
	if(hrtThread->joinable()){
		hrtThread->join();
	}
	delete hrtThread;
}

EyelinkHRT* EyelinkHRT::unique_instance = NULL;
vector<GazeDatum> EyelinkHRT::gaze_data;
vector<GazeDatum> EyelinkHRT::short_gazelist(3);
hr_clock::time_point EyelinkHRT::start_time;
milliseconds EyelinkHRT::current_time;
milliseconds EyelinkHRT::MAX_TRACK_TIME = chrono::seconds(60);
milliseconds EyelinkHRT::temporal_resolution = milliseconds(1);
Point2D EyelinkHRT::current_pos(0,0);
Point2D EyelinkHRT::current_velocity(0,0);
Point2D EyelinkHRT::current_accel(0,0);
bool EyelinkHRT::blink_detected = false;
EyelinkHRT::HRTState EyelinkHRT::state = HRT_STOPPED;
stdx::thread *EyelinkHRT::hrtThread = NULL;
stdx::mutex EyelinkHRT::mutex;
LiteTracker *EyelinkHRT::eyetracker = NULL;
bool EyelinkHRT::thread_alive = true;