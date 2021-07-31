#include <ultra64.h>
#include <PR/gs2dex.h>
#include <PR/gu.h>
#define ABSF(f) ((f) < 0 ? -(f) : (f))
#include "s2d_engine/s2d_print.h"
#include "notes.h"
#include "longnotes.h"
#include "funkin.h"
#include "game/camera.h"
#include "game/game_init.h"
#include "game/object_list_processor.h"
#include "object_fields.h"
#include "behavior_data.h"
#include "mario_animation_ids.h"
#include "game/level_update.h"

// uncomment for funny "you lose even if you win"
// #define SIIVAGUNNER

#define LEEWAY -50.0f
#define GOOD_NOTE_DT 82.0f

#define HEALTH_START 200

u32 color_array[] = {
    GRN_COL,
    RED_COL,
    TRQ_COL,
    BLU_COL,
    TEMPL_COL,
};

u32 color_array2[] = {
    0x00FF00FF,
    0xFF0000FF,
    TRQ_COL,
    0x0000FFFF,
};

uObjMtx noteBuffer[MAX_SPRITE_COUNT];
u32 envColorBuffer[MAX_SPRITE_COUNT];
u16 palBuffer[MAX_SPRITE_COUNT][16];
uObjTxtrTLUT_t tlutBuffer[MAX_SPRITE_COUNT];
int noteIndex = 0;

// gameplay memes
f32 held_lens[4] = {0, 0, 0, 0};
f32 held_latch[4] = {0,0,0,0};

void setup_mtx(uObjMtx *buf, int x, int y, f32 scale) {
    buf->m.A = FTOFIX32(scale);
    buf->m.D = FTOFIX32(scale);

    buf->m.X = x << 2;
    buf->m.Y = y << 2;
}

void setup_mtx2(uObjMtx *buf, int x, int y) {
    buf->m.A = 0x4000;
    buf->m.D = 0x4000;

    buf->m.X = x << 2;
    buf->m.Y = y << 2;
}

void call_longnote_sprite_dl(u32 col, int idx, int x, int y, uObjMtx *buffer, int buf_idx) {
    gDPPipeSync(gDisplayListHead++);

    gDPSetCycleType(gDisplayListHead++, G_CYC_1CYCLE);

    gDPSetEnvColor(
        gDisplayListHead++,
        ((col >> 24) & 0xFF) - 20,
        ((col >> 16) & 0xFF) - 20,
        ((col >>  8) & 0xFF) - 20,
        0xFF
        );

    gDPSetCombineMode(gDisplayListHead++,
        G_CC_FADEA, G_CC_FADEA
        );

    gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SPRITE, G_RM_XLU_SPRITE2);
    gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
    gSPObjRenderMode(gDisplayListHead++, G_OBJRM_XLU | G_OBJRM_BILERP);
    gSPObjLoadTxtr(gDisplayListHead++, &longnote_tex[idx]);
    setup_mtx2(&buffer[buf_idx], x, y);
    gSPObjMatrix(gDisplayListHead++, &buffer[buf_idx]);
    gSPObjSprite(gDisplayListHead++, &longnote_obj);
    gDPPipeSync(gDisplayListHead++);
    gDPSetCombineMode(gDisplayListHead++, G_CC_DECALRGBA, G_CC_DECALRGBA);
}

void assign_tlut(int index, u32 color) {
    u16 *pal = segmented_to_virtual(&note_pal_tex_0);
    uObjTxtrTLUT_t *tlut = segmented_to_virtual(&note_pal_TLUT);

    bcopy(pal, palBuffer[index], sizeof(u16) * 16);

    bcopy(tlut, &tlutBuffer[index], sizeof(tlutBuffer[0]));
    tlutBuffer[index].image = (u64 *) palBuffer[index];
    tlutBuffer[index].flag  = (u32  ) palBuffer[index];

    palBuffer[index][0xC] = GPACK_RGBA5551 (
        ((color) >> 24) & 0xFF,
        ((color) >> 16) & 0xFF,
        ((color) >> 8) & 0xFF,
        ((color)) & 0xFF
    );
}

void call_note_sprite_dl(int idx, int color, int x, int y, uObjMtx *buffer, int buf_idx, f32 scale) {
    gDPPipeSync(gDisplayListHead++);
    gDPSetTexturePersp(gDisplayListHead++, G_TP_NONE);
    gDPSetTextureLOD(gDisplayListHead++, G_TL_TILE);
    gDPSetTextureConvert(gDisplayListHead++, G_TC_FILT);
    gDPSetAlphaCompare(gDisplayListHead++, G_AC_THRESHOLD);
    gDPSetBlendColor(gDisplayListHead++, 0, 0, 0, 0x01);
    gDPSetCombineLERP(gDisplayListHead++,
        0,0,0,TEXEL0,
        0,0,0,TEXEL0,
        0,0,0,TEXEL0,
        0,0,0,TEXEL0
        );
    gDPSetCycleType(gDisplayListHead++, G_CYC_1CYCLE);
    gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SPRITE, G_RM_XLU_SPRITE2);
    gDPSetTextureLUT(gDisplayListHead++, G_TT_RGBA16);
    gSPObjRenderMode(gDisplayListHead++, G_OBJRM_XLU);
    gSPObjLoadTxtr(gDisplayListHead++, &note_tex[idx]);

    assign_tlut(buf_idx, color_array[color]);
    gSPObjLoadTxtr(gDisplayListHead++, &tlutBuffer[buf_idx]);
    setup_mtx(&buffer[buf_idx], x+16, y+16, scale);
    gSPObjMatrix(gDisplayListHead++, &buffer[buf_idx]);
    gSPObjSprite(gDisplayListHead++, &note_obj);
}


struct TrackHit {
    int x;
    f32 scale;
};
struct TrackHit trackHitScales[] = {
    {TRACK1, 0.5f},
    {TRACK2, 0.5f},
    {TRACK3, 0.5f},
    {TRACK4, 0.5f},
    {TRACK5, 0.5f},
    {TRACK6, 0.5f},
    {TRACK7, 0.5f},
    {TRACK8, 0.5f},
    // padding in case the converter outputs a bad value
    {0, 1.0f},
    {0, 1.0f},
    {0, 1.0f},
};

int track_array[] = {
    TRACK1,
    TRACK2,
    TRACK3,
    TRACK4,

    TRACK5,
    TRACK6,
    TRACK7,
    TRACK8,
};


void funkin_debug1(void) {
    char t[0x200];
    char *y = t;
    y += sprintf(y, SCALE "25");
    for (int i = 0; i < ARRAY_COUNT(trackHitScales); i++) {
        y += sprintf(y, "S %.2f\n", trackHitScales[i].scale
            );
    }

    s2d_print_alloc(50, 50, ALIGN_LEFT, t);
}

int find_track_scale(int x) {
    for (int i = 0; i < ARRAY_COUNT(trackHitScales); i++) {
        if (trackHitScales[i].x == x) {
            return i;
        }
    }
    return -1;
}

void funkin_track_scales(f32 timer) {
    for (int i = 0; i < funkin_notecount; i++) {
        if (funkin_notes[i].timer_offset > timer) break;

        f32 dt = ABSF(funkin_notes[i].timeHit - timer - LEEWAY);
        if (dt < 120.0f && timer > 500.0f) {
            trackHitScales[funkin_notes[i].track].scale =
            approach_f32_asymptotic(
                trackHitScales[funkin_notes[i].track].scale,
                0.7f,
                0.5f
                );
        } else {
            trackHitScales[funkin_notes[i].track].scale =
            approach_f32_asymptotic(
                trackHitScales[funkin_notes[i].track].scale,
                0.5f,
                0.5f
                );
        }
    }
}

int track_array2[] = {
    TRACK1,
    TRACK2,
    TRACK3,
    TRACK4,

    TRACK5,
    TRACK6,
    TRACK7,
    TRACK8,
};

void funkin_draw_note(int x, int y, int color, int direction) {
    if (color == TEMPL) {

        call_note_sprite_dl(direction, color, x, y, noteBuffer, noteIndex++, 
            trackHitScales[find_track_scale(track_array2[find_track_scale(x)])].scale
            // 0.6f
        );
    } else {
        call_note_sprite_dl(direction, color, x, y, noteBuffer, noteIndex++, 0.5f);
    }
    if (noteIndex >= MAX_SPRITE_COUNT) {
        noteIndex = 0;
    }
}

funkin_health = HEALTH_START;

#include "spritefiles/maro.h"
#include "spritefiles/iconbf.h"

#define VS_OFFSET -00

void funkin_hud(void) {
    #define HUD_Y 20
    // cpu
    funkin_draw_note(TRACK1, HUD_Y, TEMPL, LEFT);
    funkin_draw_note(TRACK2, HUD_Y, TEMPL, DOWN);
    funkin_draw_note(TRACK3, HUD_Y, TEMPL, UP);
    funkin_draw_note(TRACK4, HUD_Y, TEMPL, RIGHT);

    funkin_draw_note(TRACK5, HUD_Y, TEMPL, LEFT);
    funkin_draw_note(TRACK6, HUD_Y, TEMPL, DOWN);
    funkin_draw_note(TRACK7, HUD_Y, TEMPL, UP);
    funkin_draw_note(TRACK8, HUD_Y, TEMPL, RIGHT);    

    #undef HUD_Y

    #define E(spr) ((uObjSprite*)segmented_to_virtual(spr))

    E(&maro_obj)->s.objX = (240 - ((funkin_health / 2) + VS_OFFSET)) << 2;
    E(&iconbf_obj)->s.objX = (240 - ((funkin_health / 2) + (VS_OFFSET - 24))) << 2;

    E(&maro_obj)->s.objY = 180 << 2;
    E(&iconbf_obj)->s.objY = 180 << 2;

    E(&maro_obj)->s.scaleW = E(&maro_obj)->s.scaleH = 2 << 10;
    E(&iconbf_obj)->s.scaleW = E(&iconbf_obj)->s.scaleH = 2 << 10;


}
#define TIMER_START_OFFSET 0
f32 funkin_timer = TIMER_START_OFFSET;
f32 funkin_hit_timer = TIMER_START_OFFSET;
void funkin_reset_timers(void) {
    funkin_timer = TIMER_START_OFFSET;
    funkin_hit_timer = TIMER_START_OFFSET;
}



#define THE_LONG_CONSTANT 64

#define DIST_FROM_TOP(p, i) (((int)(\
                            p[i].timer_offset - startTime\
                            ) / 4))

int directionArray[] = {
    LEFT,
    DOWN,
    UP,
    RIGHT,
};

// i sure hope you didnt intend to edit this part of the code :)
// i sure hope i dont either :)
// update: i had to edit this part of the code :)
#define _ 8

void funkin_gen_longnote(int track, int y, int len, int idx) {
    int i,j;

    for (i = y, j = 0; i < y + len; i += THE_LONG_CONSTANT, j++) {
        call_longnote_sprite_dl(color_array2[track % 4], 0, track + _, y + (j * 11) + _, noteBuffer, noteIndex++);
        if (noteIndex >= MAX_SPRITE_COUNT) {
            noteIndex = 0;
        }
    }
    call_longnote_sprite_dl(color_array2[track % 4], 1,
        track + _,
        y + (j * 11) + _, noteBuffer, noteIndex++);
    if (noteIndex >= MAX_SPRITE_COUNT) {
        noteIndex = 0;
    }
    funkin_draw_note(track_array[funkin_notes[idx].track],
                         y,
                         funkin_notes[idx].track % 4,
                         directionArray[funkin_notes[idx].track % 4]
                         );
}

#undef _


#define MAX_DRAW_TIME 2400.0f
#define TIMER_SWAY 800.0f

#define COND_DRAW funkin_notes[i].who_sings <= FUNKIN_BF
// #define COND_DRAW 1



void funkin_draw_beatmap_from_offset(f32 startTime) {
    for (int i = 0; i < funkin_notecount; i++) {
        if (funkin_notes[i].timer_offset < (startTime - TIMER_SWAY)) continue;
        if (funkin_notes[i].timer_offset > (startTime + MAX_DRAW_TIME)) break;

        if (COND_DRAW) {
            if (funkin_notes[i].length == 0) {
                funkin_draw_note(track_array[funkin_notes[i].track],
                             DIST_FROM_TOP(funkin_notes, i),
                             funkin_notes[i].track % 4,
                             directionArray[funkin_notes[i].track % 4]
                             );
            } else {
                funkin_gen_longnote(
                             track_array[funkin_notes[i].track],
                             DIST_FROM_TOP(funkin_notes, i),
                             funkin_notes[i].length,
                             i
                             );
            }
        }
    }
}

// TODO: handle that HUD bar thing
int score = 0;
int combo = 0;

u8 isScored[MAX_SPRITE_COUNT];

u16 buttonArray[4] = {
    L_CBUTTONS,
    D_CBUTTONS,
    U_CBUTTONS,
    R_CBUTTONS,
};

u16 buttonArray2[4] = {
    L_JPAD,
    D_JPAD,
    U_JPAD,
    R_JPAD,
};


extern u8 funkin_focus_char;


void funkin_handle_camera(f32 startTime) {
    struct Object *bf = cur_obj_nearest_object_with_behavior(bhvFunkin);
    int i;
    for (i = 0; i < funkin_notecount; i++) {
        if (funkin_notes[i].timer_offset + funkin_notes[i].length > startTime) break;
    }

    // if (i == funkin_notecount - 1) {
    //     funkin_focus_char = FUNKIN_BF;
    // } else 
    funkin_focus_char = funkin_notes[i].who_sings;
}

u32 funkin_stick_correct_direction(u32 track) {
    switch (track) {
        case 0: // LEFT
            return (gPlayer1Controller->stickX < -30);
        case 1: // DOWN
            return (gPlayer1Controller->stickY < -30);
        case 2: // UP
            return (gPlayer1Controller->stickY > 30);
        case 3: // RIGHT
            return (gPlayer1Controller->stickX > 30);
    }
}

void funkin_record_note_hit_timings(f32 startTime) {
    int i;
    for (i = 0; i < funkin_notecount; i++) {
        if (funkin_notes[i].timer_offset > startTime) break;
    }

    if (funkin_notes[i].who_sings == FUNKIN_BF) {
        // f32 l = funkin_notes[i].length;
        if (gPlayer1Controller->buttonPressed & (buttonArray[funkin_notes[i].track % 4] | buttonArray2[funkin_notes[i].track % 4])
        ) {
            funkin_notes[i].timeHit = startTime;
        }
        if (funkin_notes[i].timeHit == 0.0f &&
            (gPlayer1Controller->buttonDown & (buttonArray[funkin_notes[i].track % 4] | buttonArray2[funkin_notes[i].track % 4])
            || funkin_stick_correct_direction(funkin_notes[i].track % 4)
        )) {
            funkin_notes[i].timeHit = startTime;
        }
    } else {
        funkin_notes[i].timeHit = funkin_notes[i].timer_offset;
    }
}

// TODO: yeah figure this out
// i need to record release timings of notes well after they've left the view
// of the hit function
void funkin_record_note_release_timings(f32 startTime) {
    int i;
    for (i = 0; i < funkin_notecount; i++) {
        if ((funkin_notes[i].timer_offset + funkin_notes[i].length) > startTime) break;
    }

    if (funkin_notes[i].who_sings == FUNKIN_BF) {
        f32 l = funkin_notes[i].length;
        if (funkin_notes[i].timeHit != 0.0f
            && (gPlayer1Controller->buttonDown    & buttonArray[funkin_notes[i].track % 4]) == 0
            && (gPlayer1Controller->buttonPressed & buttonArray[funkin_notes[i].track % 4]) == 0) {
            funkin_notes[i].timeReleased = startTime;
        }
    }
}


 
static int funkin_score_dt(f32 dt) {
    int toReturn;

    if      (dt > 150.0f) toReturn = -10;
    else if (dt > 100.0f) toReturn =  -5;
    else if (dt >  90.0f) toReturn =   0;
    else if (dt >  80.0f) toReturn =   1;
    else if (dt >  50.0f) toReturn =   5;
    else if (dt >  40.0f) toReturn =  10;
    else if (dt >  20.0f) toReturn =  20;
    else if (dt >  10.0f) toReturn =  30;
    else                  toReturn =  50;

    if (toReturn > 0) {
        combo++;
        funkin_health += 10;
    } else {
        combo = 0;
        funkin_health -= 15;
    }

    return toReturn;
}
#include "s2d_engine/s2d_print.h"

// calculates the entire score every frame
// this is the only way i could do this
void funkin_calculate_score(f32 startTime) {
    combo = 0;
    funkin_health = HEALTH_START;

    for (int i = 0; i < funkin_notecount; i++) {
        if (funkin_notes[i].timer_offset > startTime) break;

        if (funkin_notes[i].who_sings == FUNKIN_BF) {
            f32 dt = ABSF(funkin_notes[i].timeHit - funkin_notes[i].timer_offset - LEEWAY);
            if (funkin_notes[i].length == 0) {
                score += funkin_score_dt(dt);
                if (score < 0) score = 0;
            } else {
                // f32 dlength = ABSF(
                //     ((f32) funkin_notes[i].length)
                //     - (
                //         funkin_notes[i].timeReleased
                //         - funkin_notes[i].timer_offset
                //       )
                //     - LEEWAY
                //     );

                score += funkin_score_dt(dt);
                      // +   funkin_score_dt(dlength));
                if (funkin_health > 400) funkin_health = 400;
                if (score < 0) score = 0;
            }
        }
    }
    if (score < 0) score = 0;
    if (funkin_health > 400) funkin_health = 400;
}

#define COMBO_SWAY 110.0f

void funkin_calculate_combo(f32 startTime) {
    combo = 0;
    for (int i = 0; i < funkin_notecount; i++) {
        if (funkin_notes[i].timer_offset < (startTime - TIMER_SWAY)) continue;
        if (funkin_notes[i].timer_offset > startTime) break;

        if (funkin_notes[i].who_sings == FUNKIN_BF) {
            f32 dt = ABSF(funkin_notes[i].timeHit - funkin_notes[i].timer_offset - LEEWAY);
            if (funkin_notes[i].length == 0) {
                if (dt <= GOOD_NOTE_DT) combo++;
                else combo = 0;
            } else {
                // f32 dlength = ABSF(
                //     ((f32) funkin_notes[i].length)
                //     - (funkin_notes[i].timeReleased - funkin_notes[i].timer_offset)
                //     - LEEWAY
                //     );

                if (dt <= GOOD_NOTE_DT) combo++;
                else combo = 0;
            }
        }
    }
}

#include "chart/chart.c"

#define BPM_TO_FPS(b) ((60.0f / b) * 100.0f)

int started_music_latch = 0;
#include "audio/external.h"
#include "seq_ids.h"

// if you're smart pls implement this :)
void funkin_update_buttons_held(void) {
    // for (int i = 0; i < 4; i++) {
    //     if (gPlayer1Controller->buttonDown & buttonArray[i]) {
    //         held_lens[i] += BPM_TO_FPS(funkin_bpm);
    //     } else {
    //         if (held_lens[i] != 0.0f) {
    //             held_latch[i] = held_lens[i];
    //         }
    //         held_lens[i] = 0.0f;
    //     }
    // }
}

#include "spritefiles/ready.h"
#include "spritefiles/set.h"
#include "spritefiles/go.h"
#include "spritefiles/lose.h"

Gfx *countdownPtrs[] = {
    NULL,
    NULL,
    ready_bg_dl,
    set_bg_dl,
    go_bg_dl,
    NULL,
    NULL,
    NULL,
    NULL,
};
int funkin_countdown = 0;


void funkin_draw_countdown(void) {
    if (countdownPtrs[funkin_countdown] != NULL) {
        gSPDisplayList(gDisplayListHead++, countdownPtrs[funkin_countdown]);
    }
}


u16 buttonArraySwitchCaseLUT[] = {
    L_CBUTTONS,
    U_CBUTTONS,
    D_CBUTTONS,
    R_CBUTTONS,
    0,
    0,
};

u16 buttonArraySwitchCaseLUT2[] = {
    L_JPAD,
    U_JPAD,
    D_JPAD,
    R_JPAD,
    0,
    0,
};


void funkin_choose_bf_model(void) {
    struct Object *bf = cur_obj_nearest_object_with_behavior(bhvFunkin);
    if (bf) {
        u16 p = gPlayer1Controller->buttonPressed;
        u16 d = gPlayer1Controller->buttonDown;
        int i;
        for (i = 0; i < ARRAY_COUNT(buttonArraySwitchCaseLUT); i++) {
            if (d & buttonArraySwitchCaseLUT[i]
             || d & buttonArraySwitchCaseLUT2[i]
            ) {
                break;
            }
            if (p & buttonArraySwitchCaseLUT[i]
             || p & buttonArraySwitchCaseLUT2[i]
            ) {
                break;
            }
        }
        bf->oAnimState = (i == ARRAY_COUNT(buttonArraySwitchCaseLUT) - 1) ? 0 : i + 1;

        if (gPlayer1Controller->stickX < -30) bf->oAnimState = 1;
        if (gPlayer1Controller->stickY < -30) bf->oAnimState = 3;
        if (gPlayer1Controller->stickY > 30)  bf->oAnimState = 2;
        if (gPlayer1Controller->stickX > 30)  bf->oAnimState = 4;
    }
}


u8 mario_anim_table[] = {
    MARIO_ANIM_IDLE_WITH_LIGHT_OBJ,
    MARIO_ANIM_SLIDEJUMP,
    MARIO_ANIM_FIRST_PUNCH,
    MARIO_ANIM_BREAKDANCE,
    MARIO_ANIM_SOFT_BACK_KB,
};

int j = 9;

u8 isPlaying[] = {0,0,0,0,0,0,0,0};

static void funkin_cpu_playing_helper(f32 startTime) {
    // for (int i = 0; i < 8; i++) {
    //     isPlaying[i] = 0;
    // }
    for (int i = 0; i < funkin_notecount; i++) {
        if (funkin_notes[i].timer_offset > startTime) break;

        // if (funkin_notes[i].timer_offset < startTime &&
        //    (funkin_notes[i].timer_offset + funkin_notes[i].length > startTime)) {
        //     isPlaying[funkin_notes[i].track] = 2;
        // } else 
        if (ABSF(funkin_notes[i].timer_offset - startTime) < 40.0f){
            if (funkin_notes[i].who_sings <= FUNKIN_BF) {
                isPlaying[funkin_notes[i].track] = 1;
            }
        } else {
            isPlaying[funkin_notes[i].track] = 0;
        }
    }

    // char playing_str[0x50];
    // sprintf(playing_str, "E %d %d %d %d", isPlaying[0],
    //                                       isPlaying[1],
    //                                       isPlaying[2],
    //                                       isPlaying[3]
    //                                         );
    // s2d_print_alloc(50, 150, playing_str);
}

// TODO: fix n' rewrite

int marioIsAnimating = 0;
int tableEntry = 0;

void funkin_set_mario_anim(f32 startTime) {
    int i;
    int pressed = -1;

    funkin_cpu_playing_helper(startTime);

    for (i = 0; i < 4; i++) {
        if (isPlaying[i] == 1) {
            pressed = i + 1;
            tableEntry = i + 1;
        }
    }

    if (pressed == -1) {
        if (!marioIsAnimating) {
            set_mario_animation(gMarioState, mario_anim_table[0]);
        }
    } else {
        set_mario_animation(gMarioState, mario_anim_table[pressed]);
        marioIsAnimating = 1;
    }

    if (marioIsAnimating) {
        if (is_anim_past_end(gMarioState)) {
            marioIsAnimating = 0;
        }
    }
}


#include "sounds.h"

u32 funkin_sounds[] = {
    SOUND_MARIO_MAMA_MIA, // 3
    SOUND_PEACH_FOR_MARIO, // 2
    SOUND_PEACH_DEAR_MARIO, // 1
    SOUND_MENU_THANK_YOU_PLAYING_MY_GAME, // go!
    SOUND_ACTION_TERRAIN_STEP, // quiet enough sound to do stuff
};
u8 funkin_soundlatches[] = {0,0,0,0,0};


void funkin_handle_countdown(f32 timer) {
    for (int i = 0; i < funkin_notecount; i++) {
        if (funkin_notes[i].who_sings < 2) break;
        if (funkin_notes[i].timer_offset > timer) break;
        if (funkin_notes[i].who_sings < 8) {
            if (funkin_soundlatches[funkin_notes[i].who_sings - 2] == 0) {
                play_sound(funkin_sounds[funkin_notes[i].who_sings - 2], gGlobalSoundSource);
                funkin_soundlatches[funkin_notes[i].who_sings - 2] = 1;
                funkin_countdown++;
                break;
            }
        }
    }
}


void funkin_debug(f32 startTime) {
    // char debug_str[0x100];
    // int i;

    // for (i = 1; i < funkin_notecount; i++) {
    //     if (funkin_notes[i].timer_offset > startTime) break;
    // }
    // struct funkin_note f;

    // f = funkin_notes[i - 1];
    // sprintf(debug_str, "%s %.2f %d %d %.2f %.2f %d",
    //     f.who_sings == FUNKIN_BF ? "bf" : "ma",
    //     f.timer_offset,
    //     f.track,
    //     f.length,
    //     f.timeHit,
    //     f.timeReleased,
    //     gMarioObject ? gMarioObject->header.gfx.animInfo.animID : -1
    // );
    // s2d_print_alloc(30, 30, debug_str);

    // f = funkin_notes[i];
    // sprintf(debug_str, "%s %.2f %d %d %.2f %.2f %d",
    //     f.who_sings == FUNKIN_BF ? "bf" : "ma",
    //     f.timer_offset,
    //     f.track,
    //     f.length,
    //     f.timeHit,
    //     f.timeReleased,
    //     gMarioObject ? gMarioObject->header.gfx.animInfo.animID : -1
    // );
    // s2d_print_alloc(30, 46, debug_str);

    // f = funkin_notes[i + 1];
    // sprintf(debug_str, "%s %.2f %d %d %.2f %.2f %d",
    //     f.who_sings == FUNKIN_BF ? "bf" : "ma",
    //     f.timer_offset,
    //     f.track,
    //     f.length,
    //     f.timeHit,
    //     f.timeReleased,
    //     gMarioObject ? gMarioObject->header.gfx.animInfo.animID : -1
    // );
    // s2d_print_alloc(30, 62, debug_str);
}

#include "sick.h"
#include "good.h"
#include "bad.h"

#define SET_BG_XY(bg, x, y) {\
    uObjBg *f = segmented_to_virtual(bg);\
    f->s.frameX = x<<2; f->s.frameY = y<<2;\
}

// #define SET_BG_XY(bg, x, y) 

#define SET_BG_SCALE(bg, sc) {\
    uObjBg *f = segmented_to_virtual(bg);\
    f->s.scaleW = (1.0f/sc) * (1 << 10); f->s.scaleH = (1.0f/sc) * (1 << 10);\
}

s16 toadX, toadY;
void funkin_note_hit_feedback(f32 timer) {
    int i;
    for (i = 0; i < funkin_notecount; i++) {
        if (funkin_notes[i].who_sings != FUNKIN_BF) continue;
        if (funkin_notes[i].timer_offset < timer - 300.0f) continue;
        if (funkin_notes[i].timer_offset > timer) break;
        f32 dt = ABSF(funkin_notes[i].timeHit - funkin_notes[i].timer_offset - LEEWAY);
        if (dt < 40.0f) {
            SET_BG_XY(&sick_bg, toadX, toadY);
            SET_BG_SCALE(&sick_bg, 0.75f);
            gSPDisplayList(gDisplayListHead++, sick_bg_dl);
        } else if (dt < GOOD_NOTE_DT) {
            SET_BG_XY(&good_bg, toadX, toadY);
            SET_BG_SCALE(&good_bg, 0.75f);
            gSPDisplayList(gDisplayListHead++, good_bg_dl);
        } else {
            SET_BG_XY(&bad_bg, toadX, toadY);
            SET_BG_SCALE(&bad_bg, 0.75f);
            gSPDisplayList(gDisplayListHead++, bad_bg_dl);
        }
    }
    // char t[0x40];
    // sprintf(t, SCALE "25""X %d Y %d", toadX, toadY);
    // s2d_print_alloc(50,50, ALIGN_LEFT, t);
}

int score_latch = 0;
int combo_latch = 0;
int initialscore = 0;
int initialcombo = 0;


/**
 * FUNKIN TODO LIST
 * - playtesters
 **/

funkin_text_scale = 25;
funkin_scoretext_x = 10;
funkin_scoretext_y = 200;
funkin_scoretext_x2 = 10;
funkin_scoretext_y2 = 216;

funkin_stopscroll = 0;


#define WIN_CONDITION funkin_timer > funkin_notes[funkin_notecount - 1].timer_offset + 2000.0f
// #define WIN_CONDITION gPlayer1Controller->buttonDown & L_TRIG

lose_latch = 0;
void funkin_eval_win_condition(void) {
    if (WIN_CONDITION) {
        play_music(SEQ_PLAYER_ENV, 0, 0);
        stop_background_music(SEQUENCE_ARGS(4, SEQ_STREAMED_BACKGROUND));
        stop_background_music(SEQUENCE_ARGS(4, SEQ_STREAMED_VOICES));
        funkin_stopscroll = 1;

#ifdef SIIVAGUNNER
        gSPDisplayList(gDisplayListHead++, lose_bg_dl);
        if (lose_latch == 0) {
            lose_latch = 1;
            play_sound(SOUND_PEACH_POWER_OF_THE_STARS, gGlobalSoundSource);
        }
#else
        funkin_text_scale = approach_s16_asymptotic(funkin_text_scale, 80, 3);
        funkin_scoretext_x = approach_s16_asymptotic(funkin_scoretext_x, 100, 3);
        funkin_scoretext_y = approach_s16_asymptotic(funkin_scoretext_y, 120, 3);
        funkin_scoretext_x2 = approach_s16_asymptotic(funkin_scoretext_x2, 100, 3);
        funkin_scoretext_y2 = approach_s16_asymptotic(funkin_scoretext_y2, 180, 3);
        s2d_print_alloc(90, 35, ALIGN_LEFT, DROPSHADOW "2 2" "You Win!");
        s2d_print_alloc(100, 220, ALIGN_LEFT, SCALE "25" DROPSHADOW "2 2" "Hack by someone2639");
#endif
    }
    if (funkin_health <= 0) {
        gSPDisplayList(gDisplayListHead++, lose_bg_dl);
        if (lose_latch == 0) {
            lose_latch = 1;
            play_sound(SOUND_PEACH_POWER_OF_THE_STARS, gGlobalSoundSource);
        }
        play_music(SEQ_PLAYER_ENV, 0, 0);
        stop_background_music(SEQUENCE_ARGS(4, SEQ_STREAMED_BACKGROUND));
        stop_background_music(SEQUENCE_ARGS(4, SEQ_STREAMED_VOICES));
        funkin_stopscroll = 1;
    }
}


void fillrect(int ulx, int uly, int lrx, int lry) {
    #define gp gDisplayListHead

    gDPPipeSync(gp++);
    gDPSetRenderMode(gp++, G_RM_NOOP, G_RM_NOOP2);
    gDPSetCycleType(gDisplayListHead++,G_CYC_FILL);
    gDPSetFillColor(gp++, 0xFF5D<<16  | 0xFF5D);
    gDPFillRectangle(gp++, ulx, uly, lrx, lry);
    gDPPipeSync(gDisplayListHead++);
    gDPSetCycleType(gDisplayListHead++,G_CYC_1CYCLE);

}

void funkin_game_loop(void) {

    gMarioState->faceAngle[1] = 0x3800;

    funkin_note_hit_feedback(funkin_timer);

    funkin_hud();

    funkin_draw_beatmap_from_offset(funkin_timer);

    funkin_update_buttons_held();

    funkin_handle_camera(funkin_timer);
    funkin_record_note_hit_timings(funkin_timer);
    // funkin_record_note_release_timings(funkin_timer);

    score = 0;
    // combo = 0;
    funkin_calculate_score(funkin_timer);
    // funkin_calculate_combo(funkin_timer);

    if (score_latch == 0) {
        score_latch = 1;
        initialscore = -score;
    }
    // if (combo_latch == 0) {
    //     combo_latch = 1;
    //     initialcombo = -combo;
    // }

    // funkin_debug(funkin_timer);
    if (gPlayer1Controller->buttonDown & L_TRIG) {
        funkin_debug1();
    }

    score += initialscore;
    // combo += initialcombo;

    funkin_choose_bf_model();
    funkin_set_mario_anim(funkin_timer);

    if (funkin_stopscroll == 0) {
        funkin_timer += (BPM_TO_FPS(funkin_bpm));
    
        funkin_hit_timer += (BPM_TO_FPS(funkin_bpm));
    }

    funkin_handle_countdown(funkin_timer);
    funkin_track_scales(funkin_timer);

    if (funkin_countdown == 5) {
        if (started_music_latch == 0) {
            started_music_latch = 1;

            play_music(SEQ_PLAYER_LEVEL, SEQ_STREAMED_BACKGROUND, 0);
            play_music(SEQ_PLAYER_ENV, SEQ_STREAMED_VOICES, 0);
        }
    }

    char score_display[0x60];
    char combo_display[0x60];
    sprintf(score_display, DROPSHADOW "2 2" SCALE "%d" "Score: %d",funkin_text_scale, score);
    sprintf(combo_display, DROPSHADOW "2 2" SCALE "%d" "Combo: %d",funkin_text_scale, combo);

    // char debug_held_lens[0x40];
    // sprintf(debug_held_lens, SCALE "25" "A %f %d %d", funkin_timer, funkin_countdown, funkin_health);
    // sprintf(debug_held_lens, "A%.2f %.2f %.2f %.2f", held_lens[0], held_lens[1], held_lens[2], held_lens[3]);
    // s2d_print_alloc(50, 180,ALIGN_LEFT, debug_held_lens);

    funkin_eval_win_condition();


    if (funkin_health > 0) {
        fillrect(70, 202, 300, 208);
        gSPDisplayList(gDisplayListHead++, maro_sprite_dl);
        gSPDisplayList(gDisplayListHead++, iconbf_sprite_dl);
        s2d_print_alloc(funkin_scoretext_x, funkin_scoretext_y,ALIGN_LEFT, score_display);
        s2d_print_alloc(funkin_scoretext_x2, funkin_scoretext_y2,ALIGN_LEFT, combo_display);
    }

    funkin_draw_countdown();

}



//
