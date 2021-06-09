

extern f32 funkin_timer;
extern int funkin_bpm;
#define ABSF(f) ((f) < 0 ? -(f) : (f))


void register_obj_rhythm(void) {

}

u32 boomboxscalar = 1;

f32 approach_f32_asymptotic(f32 current, f32 target, f32 multiplier);

#define BPM_TO_FPS(b) ((60.0f / b) * 100.0f)

int rhythm = 0;

#include "s2d_engine/s2d_print.h"

void boombox_loop(void) {
	if (ABSF((BPM_TO_FPS(funkin_bpm) * (10 * rhythm)) - funkin_timer) < 20.0f) {
		boomboxscalar ^= 1;
	}
	if ((BPM_TO_FPS(funkin_bpm) * (10 * rhythm)) < funkin_timer) {
		rhythm++;
	}


	if (boomboxscalar) {
		o->header.gfx.scale[0] = approach_f32_asymptotic(o->header.gfx.scale[0], 1.05f, 0.5f);
		o->header.gfx.scale[1] = approach_f32_asymptotic(o->header.gfx.scale[1], 1.05f, 0.5f);
		// o->header.gfx.scale[2] = approach_f32_asymptotic(o->header.gfx.scale[2], 1.1f, 0.5f);
	} else {
		o->header.gfx.scale[0] = approach_f32_asymptotic(o->header.gfx.scale[0], 0.95f, 0.5f);
		o->header.gfx.scale[1] = approach_f32_asymptotic(o->header.gfx.scale[1], 0.95f, 0.5f);
		// o->header.gfx.scale[2] = approach_f32_asymptotic(o->header.gfx.scale[2], 0.9f, 0.5f);
	}

	// static char t[0x40];
	// sprintf(t, SCALE "25" "A %d", boomboxscalar);
	// s2d_print_deferred(50, 50, t);
}
