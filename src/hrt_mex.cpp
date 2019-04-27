#include <string>
#include <algorithm>
#include <mex.h>
#include "LiteTracker.h"
#include "EyelinkHRT.h"

#define EPS (1e-10)

EyelinkHRT* hrt;
static LiteTracker* lt;
static bool is_initialized = false;

void cleanup();

void init(int tracking_eye){
	mexLock(); // new addition: locks mex file from being cleared.
	printf("\n...Initializing HRT tracker...\n");
	lt = LiteTracker::getInstance(tracking_eye);
	hrt = EyelinkHRT::getInstance(lt);
	hrt->startTracking();
	is_initialized = true;
	printf("\n...HRT tracker initialized...\n");
	mexAtExit(cleanup);
}

void startRecording(int tracking_eye){
	printf("\n...starting record...\n");
	if(!is_initialized){
		init(tracking_eye);
	}
	LiteTracker::tracking_eye = tracking_eye;
	hrt->startRecording();
}

void stopRecording(mxArray **output){
	printf("\n...ending record...\n");
	// 1. stop recording data
	hrt->stopRecording();
	// 2. get gaze data as a vector
	std::vector<GazeDatum> gd = hrt->getGazeData();
	// 3. load these data into a ublas matrix
	unsigned nr_samples = gd.size();
	std::vector<double> gaze_dat(3*nr_samples);

	for(unsigned int i=0; i<nr_samples;++i){
		gaze_dat[i] = gd[i].pos.x;
		gaze_dat[i+nr_samples*1] = gd[i].pos.y;
		gaze_dat[i+nr_samples*2] = gd[i].time*0.001; // convert time unit to seconds;
	}
	//printf("\n...creating matlab matrix...\n");
	// 4. create a MATLAB matrix to hold the output
	*output = mxCreateDoubleMatrix(nr_samples,3,mxREAL);
	printf("\noutput ptr = %u\n",output);
	if(output==NULL){
		mexErrMsgTxt("\nWARNING: FATAL MEMORY ALLOCATION ERROR!\n");
	}
	//printf("\n...copying data into matlab matrix...\n");
	memcpy(mxGetPr(*output), &gaze_dat[0], nr_samples*3*sizeof(double));
	//printf("\n...data copied...\n");
}

void getCurrentPos(int tracking_eye,mxArray **output){
	if(!is_initialized){
		init(tracking_eye);
	}
	GazeDatum gd = hrt->getCurrentPos();
	*output = mxCreateDoubleMatrix(3,1,mxREAL);
	double varr[3];
	varr[0] = gd.pos.x;
	varr[1] = gd.pos.y;
	varr[2] = gd.time*0.001;
	memcpy(mxGetPr(*output), varr, 3*sizeof(double));
}


void getCurrentTime(int tracking_eye,mxArray **output){
	if(!is_initialized){
		init(tracking_eye);
	}
	double time = hrt->getCurrentTime()*0.001;
	*output = mxCreateDoubleScalar(time);
}

void getVelocity(int tracking_eye,mxArray **output){
	if(!is_initialized){
		init(tracking_eye);
	}
	GazeDatum gd = hrt->getCurrentVelocity();
	Point2D vel = gd.pos;
	double speed = vel.vlength();
	double time = gd.time*0.001;
	double dir = atan((vel.x+EPS)/(vel.y+EPS))*180.0/M_PI;
	double varr[3] = {speed,time,dir};
	*output = mxCreateDoubleMatrix(3,1,mxREAL);
	memcpy(mxGetPr(*output), varr, 3*sizeof(double));
}

static void cleanup(){
	if(is_initialized){
		printf("\n...ending tracking...\n");
		hrt->stopTracking();
		printf("\n...deleting HRT...\n");
		delete hrt;
		delete lt;
		is_initialized = false;
		mexUnlock();
	}
}


void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]){ // Entry point from Matlab to C
	std::string command;
	int tracking_eye;
    /* Check for proper number of arguments */
    if(nrhs<1){ 
		mexErrMsgTxt("ERROR: At least one input argument is required."); 
    }else if (nlhs > 1){
		mexErrMsgTxt("ERROR: Too many output arguments.");
	}else{
		command = std::string(mxArrayToString(prhs[0]));
		std::transform(command.begin(),command.end(),command.begin(),::tolower);
	}

	if(command=="start"){
		if(nrhs<2){
			mexErrMsgTxt("ERROR: the second parameter must indicate the tracked eye (0=left;1=right).");
		}else{
			tracking_eye = mxGetScalar(prhs[1]);
			startRecording(tracking_eye);
		}
	}else if(command=="stop"){
		stopRecording(plhs);
	}else if(command=="cleanup"){
		cleanup(); 
		// disabled (5/16/2017); this could lead to attempted deletion of null pointer
		//mexErrMsgTxt("ERROR: the 'cleanup' command is deprecated");
	}else if(command=="time"){
		if(nrhs<2){
			mexErrMsgTxt("ERROR: the second parameter must indicate the tracked eye (0=left;1=right).");
		}else{
			tracking_eye = mxGetScalar(prhs[0]);
			getCurrentTime(tracking_eye,plhs);
		}
	}else if(command=="position"){
		if(nrhs<2){
			mexErrMsgTxt("ERROR: the second parameter must indicate the tracked eye (0=left;1=right).");
		}else{
			tracking_eye = mxGetScalar(prhs[0]);
			getCurrentPos(tracking_eye,plhs);
		}
	}else if(command=="velocity"){
		if(nrhs<2){
			mexErrMsgTxt("ERROR: the second parameter must indicate the tracked eye (0=left;1=right).");
		}else{
			tracking_eye = mxGetScalar(prhs[0]);
			getVelocity(tracking_eye,plhs);
		}
	}else{
		mexErrMsgTxt("ERROR: an input string (e.g., 'start', 'stop', or 'position') is required.");
	}
}