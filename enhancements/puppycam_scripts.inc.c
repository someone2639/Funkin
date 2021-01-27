static void newcam_angle_rotate(struct newcam_hardpos *params)
{
    newcam_yaw += 0x100;
    print_text(32,32,"hi i am a script");
    print_text_fmt_int(32,48,"%d",newcam_yaw);
}


static void newcam_open_cam(struct newcam_hardpos *params);
static void (*newcam_c_up_cam_return)(struct newcam_hardpos *params) = newcam_open_cam;

// The speed of changing Mario's look angle with the analog stick
#define NEWCAM_C_UP_SPEED 6

// C-Up Cam that can be used in any puppycam script;
// Just set newcam_c_up_cam_return to the function you
// want to return to after leaving C-up mode
static void newcam_c_up_cam(struct newcam_hardpos *params) {
	newcam_distance_target = 175;

	newcam_yaw  += (s32) gPlayer1Controller->stickX * NEWCAM_C_UP_SPEED;
	newcam_tilt += (s32) gPlayer1Controller->stickY * NEWCAM_C_UP_SPEED;

	if (gPlayer1Controller->buttonPressed & D_CBUTTONS) {
		params->newcam_hard_script = newcam_c_up_cam_return;
		newcam_tilt = 1500;
		newcam_yaw = -gMarioState->faceAngle[1]-0x4000;
		set_mario_action(gMarioState, ACT_IDLE, 0);
	}
}

static int newcam_open_cam_zoomed_out = 0;

// Change the number of angles here
#define NUM_ANGLES 8
#define _OPEN_CAM_DELTA (0x10000 / NUM_ANGLES)

// Great for open areas!
// Comes with super-zoom out and a new C-Up mode!
static void newcam_open_cam(struct newcam_hardpos *params) {
	if (!newcam_open_cam_zoomed_out) newcam_distance_target = 1000;

	// Look at Mario
    params->newcam_hard_lookX = (s16)(gMarioState->pos[0]);
	params->newcam_hard_lookY = (s16)(gMarioState->pos[1] + 125.0f);
    params->newcam_hard_lookZ = (s16)(gMarioState->pos[2]);

    // The main 8-direction control
	if (gPlayer1Controller->buttonPressed & R_CBUTTONS)
		newcam_yaw -= _OPEN_CAM_DELTA;
	if (gPlayer1Controller->buttonPressed & L_CBUTTONS)
		newcam_yaw += _OPEN_CAM_DELTA;


	// Entry for C-Up mode
	if (gPlayer1Controller->buttonPressed & U_CBUTTONS) {
		if (newcam_open_cam_zoomed_out == 1) {
			newcam_open_cam_zoomed_out = 0;
		}
		else if (gMarioState->action & ACT_FLAG_ALLOW_FIRST_PERSON) {
			newcam_yaw = -gMarioState->faceAngle[1]-0x4000;
			params->newcam_hard_script = newcam_c_up_cam;
			set_mario_action(gMarioState, ACT_WAITING_FOR_DIALOG, 0);
		}
	}

	// Super-zoom out
	if (gPlayer1Controller->buttonPressed & D_CBUTTONS) {
		newcam_distance_target += 400;
		newcam_open_cam_zoomed_out = 1;
	}


	// Parallel Cam Feature: Center behind Mario
	if (gPlayer1Controller->buttonPressed & U_JPAD) {
		newcam_yaw = -gMarioState->faceAngle[1]-0x4000;
	}

	// Parallel Cam Feature: Reset the axes of the camera
	if (gPlayer1Controller->buttonPressed & D_JPAD) {
		newcam_yaw = newcam_yaw % _OPEN_CAM_DELTA >= 0x1000 ?
					 newcam_yaw + _OPEN_CAM_DELTA - (newcam_yaw % _OPEN_CAM_DELTA) :
					 newcam_yaw - (newcam_yaw % _OPEN_CAM_DELTA);
	}

	// Parallel Cam Feature: fine tune the camera angle
	// with the D-pad
	if (gPlayer1Controller->buttonDown & R_JPAD)
		newcam_yaw += 100;
	if (gPlayer1Controller->buttonDown & L_JPAD)
		newcam_yaw -= 100;
}