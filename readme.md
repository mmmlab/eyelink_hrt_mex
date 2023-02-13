# Instructions for Using this Eyelink HRT-mex Code

*******
### Overview

This repo contains code implementing an asynchronous high-resolution tracking
interface to the Eyelink 2K trackers from MATLAB/mex. It is not intended for
standalone use, but rather as a complement to the [MATLAB Eyelink Toolbox](https://www.mathworks.com/matlabcentral/fileexchange/3176-eyelink-toolbox) that 
is distributed as a part of [Psychtoolbox](http://psychtoolbox.org/).
********
### MATLAB Commands
The main functionality provided by this mex library is the asynchronous tracker,
which is accessed via the commands `eyelink_hrt('start',TRACKING_EYE)` and `eyelink_hrt('stop',TRACKING_EYE)`, where `TRACKING_EYE` should be either 0 (left eye) or 1 (right eye).

- `eyelink_hrt('start',TRACKING_EYE)` asyncronously launches an independent thread that accumulates a new gaze sample and its associated timestamp every millisecond, storing them in a memory buffer until `eyelink_hrt('stop')` is called.
- `eyelink_hrt('stop')` returns an Nx3 array of time-stamped gaze positions (x,y,t), where the gaze positions (x,y) are represented in screen coordinates and the timestamps (t) are represented in seconds elapsed since the last call of `eyelink_hrt('start')`.

*... this section to be continued ...*

*********
### Using the High Resolution Tracker with the Eyelink Toolbox

Note that this code does not include any provisions for setting up or calibrating the Eyelink tracker. That's because it's meant as a complement to the Eyelink Toolbox, and we expect that the Toolbox will be used for all non time-critical functions. If you run the `eyelink_hrt` commands without first initializing and calibrating the Eyelink tracker, it will connect to the tracker and return results, but those results will be meaningless.

Instead, we expect users to execute configuration and setup code via the Eyelink Toolbox *before* issuing any `eyelink_hrt` calls. Here's an example of a typical setup procedure using the Eyelink Toolbox to configure the Eyelink and establish a connection:

```
    % Initialize 'el' eyelink struct with proper defaults for output to
    % window 'DP.WINPTR':
    el=EyelinkInitDefaults(DP.WINPTR);

    % Initialize Eyelink connection (real or dummy). The flag '1' requests
    % use of callback function and eye camera image display:
    if ~EyelinkInit([], 1)
        fprintf('Eyelink Init aborted.\n');
        cleanup;
        return;
    end

    % Send any additional setup commands to the tracker
    Eyelink('Command','calibration_type = HV9'); % 9-point calibration
    Eyelink('Command','recording_parse_type = GAZE');
    Eyelink('Command','link_sample_data = LEFT,RIGHT,GAZE,AREA,STATUS');
    Eyelink('Command','link_event_filter = LEFT,RIGHT,FIXATION,SACCADE,BLINK');
    Eyelink('Command','sample_rate = 1000'); % 1000 Hz
    Eyelink('Command','heuristic_filter = 1'); % 
    Eyelink('Command','screen_pixel_coords = 0 0 1279 1023'); % screen res 1280 x 1024


    % Perform tracker setup: The flag 1 requests interactive setup with
    % video display:
    result = Eyelink('StartSetup',1);
    Eyelink('StartRecording');
```




