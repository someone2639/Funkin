#include <ultra64.h>
#include <PR/gs2dex.h>
#define ABSF(f) ((f) < 0 ? -(f) : (f))
#include "s2d_engine/s2d_print.h"
#include "notes.h"
#include "longnotes.h"
#include "funkin.h"
#include "game/game_init.h"

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

void setup_mtx(uObjMtx *buf, int x, int y, int scale) {
    buf->m.A = 0x8000;
    buf->m.D = 0x8000;

    buf->m.X = x << 2;
    buf->m.Y = y << 2;
}

void setup_mtx2(uObjMtx *buf, int x, int y, int scale) {
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
        (col >> 24) & 0xFF - 20,
        (col >> 16) & 0xFF - 20,
        (col >> 8) & 0xFF - 20,
        0xFF
        );

    gDPSetCombineMode(gDisplayListHead++,
        G_CC_FADEA, G_CC_FADEA
        );

    gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SPRITE, G_RM_XLU_SPRITE2);
    gDPSetTextureLUT(gDisplayListHead++, G_TT_NONE);
    gSPObjRenderMode(gDisplayListHead++, G_OBJRM_XLU | G_OBJRM_BILERP);
    gSPObjLoadTxtr(gDisplayListHead++, &longnote_tex[idx]);
    setup_mtx2(&buffer[buf_idx], x, y, 1);
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
    tlutBuffer[index].image = palBuffer[index];
    tlutBuffer[index].flag = palBuffer[index];

    palBuffer[index][0xC] = GPACK_RGBA5551 (
        ((color) >> 24) & 0xFF,
        ((color) >> 16) & 0xFF,
        ((color) >> 8) & 0xFF,
        ((color)) & 0xFF
    );
}

void call_note_sprite_dl(int idx, int color, int x, int y, uObjMtx *buffer, int buf_idx) {
    gDPPipeSync(gDisplayListHead++);
    gDPSetCycleType(gDisplayListHead++, G_CYC_1CYCLE);
    gDPSetRenderMode(gDisplayListHead++, G_RM_XLU_SPRITE, G_RM_XLU_SPRITE2);
    gDPSetTextureLUT(gDisplayListHead++, G_TT_RGBA16);
    gSPObjRenderMode(gDisplayListHead++, G_OBJRM_XLU | G_OBJRM_BILERP);
    gSPObjLoadTxtr(gDisplayListHead++, &note_tex[idx]);

    assign_tlut(buf_idx, color_array[color]);
    gSPObjLoadTxtr(gDisplayListHead++, &tlutBuffer[buf_idx]);
    setup_mtx(&buffer[buf_idx], x, y, 1);
    gSPObjMatrix(gDisplayListHead++, &buffer[buf_idx]);
    gSPObjSprite(gDisplayListHead++, &note_obj);
}

void funkin_draw_note(int x, int y, int color, int direction) {
    call_note_sprite_dl(direction, color, x, y, noteBuffer, noteIndex++);
    if (noteIndex >= MAX_SPRITE_COUNT) {
        noteIndex = 0;
    }
}

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
}

f32 funkin_timer = 0;
f32 funkin_hit_timer = 0;
void funkin_reset_timers(void) {
    funkin_timer = 0;
    funkin_hit_timer = 0;
}

int track_array[] = {
    TRACK3,
    TRACK4,
    TRACK2,
    TRACK1,

    TRACK7,
    TRACK8,
    TRACK6,
    TRACK5,
};

#define THE_LONG_CONSTANT 64

#define DIST_FROM_TOP(p, i) (((int)(\
                            p[i].timer_offset - startTime\
                            ) / 4))

// i sure hope you didnt intend to edit this part of the code :)
// i sure hope i dont either :)
#define _ 8

// void funkin_gen_longnote_player(int track, int y, int len, int idx) {
//     int start = y;
//     int i,j;

//     for (i = y, j = 0; i < y + len; i += THE_LONG_CONSTANT, j++) {
//         call_longnote_sprite_dl(color_array2[track], 0, track + _, y + (j * 11) + _, noteBuffer, noteIndex++);
//         if (noteIndex >= MAX_SPRITE_COUNT) {
//             noteIndex = 0;
//         }
//     }
//     call_longnote_sprite_dl(color_array2[track], 1, track + _, y + (j * 11) + _, noteBuffer, noteIndex++);
//     if (noteIndex >= MAX_SPRITE_COUNT) {
//         noteIndex = 0;
//     }
//     funkin_draw_note(track_array[funkin_notes_player[idx].track + 4],
//                          y,
//                          funkin_notes_player[idx].track,
//                          funkin_notes_player[idx].track
//                          );
// }

void funkin_gen_longnote(int track, int y, int len, int idx) {
    int start = y;
    int i,j;

    for (i = y, j = 0; i < y + len; i += THE_LONG_CONSTANT, j++) {
        // call_note_sprite_dl(direction, color, x, y, noteBuffer, noteIndex++);
        call_longnote_sprite_dl(color_array2[track], 0, track + _, y + (j * 11) + _, noteBuffer, noteIndex++);
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
                         funkin_notes[idx].track % 4
                         );
}

#undef _


#define MAX_DRAW_TIME 2400.0f
#define TIMER_SWAY 800.0f


void funkin_draw_beatmap_from_offset(f32 startTime) {
    for (int i = 0; i < funkin_notecount; i++) {
        if (funkin_notes[i].timer_offset < (startTime - TIMER_SWAY)) continue;
        if (funkin_notes[i].timer_offset > (startTime + MAX_DRAW_TIME)) break;

        if (funkin_notes[i].length == 0) {
            funkin_draw_note(track_array[funkin_notes[i].track],
                         DIST_FROM_TOP(funkin_notes, i),
                         funkin_notes[i].track % 4,
                         funkin_notes[i].track % 4
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

// keep 500+ by the end to win
// insta-lose at 0
// 
int score = 500;
int combo = 0;

u8 isScored[MAX_SPRITE_COUNT];

u16 buttonArray[4] = {
    U_JPAD,
    R_JPAD,
    D_JPAD,
    L_JPAD,
};

// TODO: rewrite
extern u8 funkin_focus_char;
#include "object_fields.h"
#include "behavior_data.h"

void funkin_handle_ready_notes(f32 startTime) {
    struct Object *bf = cur_obj_nearest_object_with_behavior(bhvFunkin);
    for (int i = 0; i < funkin_notecount; i++) {

    }
}

#include "chart/chart.c"

#define BPM_TO_FPS(b) ((30.0f * 60.0f) / ((f32) (b) / 1.85f))

int started_music_latch = 0;
#include "audio/external.h"
#include "seq_ids.h"

void funkin_update_buttons_held(void) {
    for (int i = 0; i < 4; i++) {
        if (gPlayer1Controller->buttonDown & buttonArray[i]) {
            held_lens[i] += BPM_TO_FPS(funkin_bpm);
        } else {
            if (held_lens[i] != 0.0f) {
                held_latch[i] = held_lens[i];
            }
            held_lens[i] = 0.0f;
        }
    }
}


void funkin_handle_switchcase(void) {
    struct Object *bf = cur_obj_nearest_object_with_behavior(bhvFunkin);
    if (bf) {
        if (funkin_focus_char) {
            bf->oAnimState = 3;
        } else {
            bf->oAnimState = 0;
        }
    }
}

void funkin_game_loop(void) {

    funkin_hud();

    funkin_draw_beatmap_from_offset(funkin_timer);

    funkin_update_buttons_held();

    funkin_handle_ready_notes(funkin_timer);

    funkin_handle_switchcase();

    funkin_timer += BPM_TO_FPS(funkin_bpm);
    // funkin_timer+=100;
    if (funkin_timer > 2350.0f) {
        funkin_hit_timer += BPM_TO_FPS(funkin_bpm);
        if (started_music_latch == 0) {
            started_music_latch = 1;

            play_music(SEQ_PLAYER_LEVEL, SEQ_STREAMED_BACKGROUND, 0);
            play_music(SEQ_PLAYER_ENV, SEQ_STREAMED_VOICES, 0);
        }
    }

    char t[0x40];
    sprintf(t, "SCORE: %d\vCOMBO: %d", score, combo);

    char t2[0x40];
    sprintf(t2, "A%.2f %.2f %.2f %.2f", held_lens[0], held_lens[1], held_lens[2], held_lens[3]);
    s2d_print_alloc(50, 150, t);
    s2d_print_alloc(50, 180, t2);
}

