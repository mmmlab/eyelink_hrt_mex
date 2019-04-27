#include "EyeLink.h"

#include <eyelink/core_expt.h>
#include "gl_coregraphics.h"

EyeLink::EyeLink(void):during_trial_record(false){}

EyeLink::~EyeLink(void){
    close_eyelink_connection();
    close_eyelink_system();
}


bool EyeLink::init(const int& main_window, const int& screen_width, const int& screen_height, const int& sample_rate, const std::vector<std::string>& command_strings, const std::vector<float>& bg_color, const std::vector<float>& fg_color){
    this->main_window    = main_window;
    this->screen_width   = screen_width;
    this->screen_height  = screen_height;
    this->bg_color        = bg_color;
    this->fg_color        = fg_color;
    

    if (open_eyelink_connection(0))
        return false;

    // open Eyelink data file
    // create filename on EyeLink computer, when transferring it over, we'll determine the file name for saving it on the experiment computer
    if (open_data_file("temp.edf") != OK_RESULT)
        return false;

    /* inform EyeLink of display resolution */
    if (eyecmd_printf("screen_pixel_coords = %ld %ld %ld %ld",0, 0, screen_width-1, screen_height-1) != OK_RESULT)
        return false;
    /* Add resolution to EDF file */
    if (eyemsg_printf("DISPLAY_COORDS %ld %ld %ld %ld",0, 0, screen_width-1, screen_height-1) != OK_RESULT)
        return false;

    /* set sample rate */
    if (eyecmd_printf("sample_rate = %ld", sample_rate) != OK_RESULT)
        return false;

    // set EDF file contents 
    if (eyecmd_printf("file_event_filter = LEFT,RIGHT,FIXATION,SACCADE,BLINK,MESSAGE,BUTTON") != OK_RESULT)
        return false;
    if (eyecmd_printf("file_sample_data  = LEFT,RIGHT,GAZE,AREA,GAZERES,STATUS") != OK_RESULT)
        return false;

    // process the commands passed by the user, e.g. calibration type = HV13
    return sendCommands(command_strings);
}

void EyeLink::initializeCallTrackerSetup(const int& mode){
    CameraSetupConfig cfg = {
        screen_width,  // width 
        screen_height,	// height


        {bg_color[0],bg_color[1],bg_color[2],bg_color[3]},	// background color
        {fg_color[0],fg_color[1],fg_color[2],fg_color[3]},  // foreground color
        0.025,//0.0625,             // targetOuterSize`
        0.01,//0.0625/4,	        // targetInnerSize
        {0.5,0.5},	        // imgPerc

        screen_width/2,    // drift correct x position
        screen_height/2,	// drift correct y position
        1,	// draw
        1,	// allow_setup
        -1,	// l
		 1,	// t
        1,	// r
        -1,	// b

        NULL	// internalUse
    };

    if(mode ==1)
        do_gl_tracker_setup(main_window, &cfg);
    else
        do_gl_drift_correct(main_window, &cfg);
}

void EyeLink::calibrate(){
    initializeCallTrackerSetup(1);
}

void cleanupCalibration(){
	cleanup_calibration();
};

void EyeLink::correctDrift(){
    initializeCallTrackerSetup(2);
}

bool EyeLink::isCalibrating(){
    return is_calibrating();
}

enum EyeLink::TrialState EyeLink::EyeLinkOK(){
    // checks if EyeLink is in usable state, can add more checks later if needed
    if(!eyelink_is_connected()){  // check if we are still connected.
        throw std::exception("Eyelink connection lost \n");
	}
    // check for trial state if during trial
    if (!during_trial_record){
        return NOT_DURING_TRIAL;
	}else{
        INT16 status = check_recording();
        switch (status){
        case TRIAL_OK:
            return TRIAL_IS_OK;
        	break;
        case REPEAT_TRIAL:
            // signal that we need a trial restart
            return TRIAL_NEEDS_REPEATING;
            break;
        case TRIAL_ERROR:
            throw std::exception("EyeLink: An error occurred during recording");
        default:
            throw std::exception("EyeLink: Expt ended by experimenter from Host PC");
        }
    }
}

bool EyeLink::sendCommands(std::vector<std::string> command_strings){
    // process the commands passed by the user, e.g. calibration type = HV13
	//stopTrialRecord();
    for (unsigned int i=0; i<command_strings.size(); i++){
        #ifdef _EL_DEBUG
            printf("Sending command \"%s\"\n",command_strings[i].c_str());
        #endif
        if (eyecmd_printf(const_cast<char*>(command_strings[i].c_str())) != OK_RESULT){
            printf("Problem processing command \"%s\"\n",command_strings[i].c_str());
            return false;
        }
    }
	//startTrialRecord(1,1);
    return true;
}

void EyeLink::startTrialRecord(const int& trial_count, const int& nr_trials){
    // This supplies the title at the bottom of the eye tracker display
    eyecmd_printf("record_status_message 'TRIAL %d/%d'",trial_count+1,nr_trials);

    // record trial number in data file
    eyemsg_printf("TRIALID %ld", trial_count);
    // start recording
    start_recording(1,1,0,0);
    during_trial_record = true;
}

void EyeLink::stopTrialRecord(){
    stop_recording();
    set_offline_mode(); // should be unnecessary, but doesn't hurt
    during_trial_record = false;
}

void EyeLink::writeSYNCTIME(){
    eyemsg_printf("SYNCTIME");
}

void EyeLink::writeTrialAborted(const int& trial_count){
    eyemsg_printf("TRIAL_ABORTED %ld", trial_count);
}

void EyeLink::writeTrialFinished(const int& trial_count){
    eyemsg_printf("TRIAL_COMPLETED %ld", trial_count);
}

void EyeLink::transferDataFile(const std::string& filename ){
    // eyelink
    close_data_file();
    // generate filename
    if (int(receive_data_file("", const_cast<char*>(filename.c_str()), 0)) < 0){
        throw std::exception("Eyelink file transfer failed"); // output is file size if successful, 0 if canceled, and negative for other errors
	}
}
