#include <ultra64.h>
#include <PR/gs2dex.h>
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
uObjMtx noteBuffer[MAX_SPRITE_COUNT];
u32 envColorBuffer[MAX_SPRITE_COUNT];
u16 palBuffer[MAX_SPRITE_COUNT][16];
uObjTxtrTLUT_t tlutBuffer[MAX_SPRITE_COUNT];
int noteIndex = 0;

// gameplay memes
u32 held_lens[4] = {0, 0, 0, 0};

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

    Color col2 = *(Color *)&col;

    gDPSetEnvColor(
        gDisplayListHead++,
        col2.c[0],
        col2.c[1],
        col2.c[2],
        col2.c[3]
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

#define THE_LONG_CONSTANT 44

#define DIST_FROM_TOP(p, i) (((int)(\
                            p[i].timer_offset - startTime\
                            ) / 2))

#define _ 8

void funkin_gen_longnote_player(int track, int y, int len, int idx) {
    int start = y;
    int i,j;

    for (i = y, j = 0; i < y + len; i += THE_LONG_CONSTANT, j++) {
        // call_note_sprite_dl(direction, color, x, y, noteBuffer, noteIndex++);
        call_longnote_sprite_dl(color_array[track], 0, track + _, y + (j * 11) + _, noteBuffer, noteIndex++);
        if (noteIndex >= MAX_SPRITE_COUNT) {
            noteIndex = 0;
        }
    }
    call_longnote_sprite_dl(color_array[track], 1, track + _, y + (j * 11) + _, noteBuffer, noteIndex++);
    if (noteIndex >= MAX_SPRITE_COUNT) {
        noteIndex = 0;
    }
    funkin_draw_note(track_array[funkin_notes_player[idx].track + 4],
                         y,
                         funkin_notes_player[idx].track,
                         funkin_notes_player[idx].track
                         );
}

void funkin_gen_longnote_cpu(int track, int y, int len, int idx) {
    int start = y;
    int i,j;

    for (i = y, j = 0; i < y + len; i += THE_LONG_CONSTANT, j++) {
        // call_note_sprite_dl(direction, color, x, y, noteBuffer, noteIndex++);
        call_longnote_sprite_dl(color_array[track], 0, track + _, y + (j * 11) + _, noteBuffer, noteIndex++);
        if (noteIndex >= MAX_SPRITE_COUNT) {
            noteIndex = 0;
        }
    }
    call_longnote_sprite_dl(color_array[track], 1,
        track + _,
        y + (j * 11) + _, noteBuffer, noteIndex++);
    if (noteIndex >= MAX_SPRITE_COUNT) {
        noteIndex = 0;
    }
    funkin_draw_note(track_array[funkin_notes_cpu[idx].track],
                         y,
                         funkin_notes_cpu[idx].track,
                         funkin_notes_cpu[idx].track
                         );
}

#undef _


#define MAX_DRAW_TIME 2400.0f
#define TIMER_SWAY 300.0f


// TODO: see if any other notes happen
void funkin_draw_beatmap_from_offset(f32 startTime) {
    for (int i = 0; i < funkin_cpu_notecount; i++) {
        if (funkin_notes_cpu[i].timer_offset < (startTime - TIMER_SWAY)) continue;
        if (funkin_notes_cpu[i].timer_offset > (startTime + MAX_DRAW_TIME)) break;

        if (funkin_notes_cpu[i].length == 0) {
            funkin_draw_note(track_array[funkin_notes_cpu[i].track],
                         DIST_FROM_TOP(funkin_notes_cpu, i),
                         funkin_notes_cpu[i].track,
                         funkin_notes_cpu[i].track
                         );
        } else {
            funkin_gen_longnote_cpu(
                         track_array[funkin_notes_cpu[i].track],
                         DIST_FROM_TOP(funkin_notes_cpu, i),
                         funkin_notes_cpu[i].length,
                         i
                         );
        }
    }

    for (int i = 0; i < funkin_player_notecount; i++) {
        if (funkin_notes_player[i].timer_offset < (startTime - TIMER_SWAY)) continue;
        if (funkin_notes_player[i].timer_offset > (startTime + MAX_DRAW_TIME)) break;

        if (funkin_notes_player[i].length == 0) {
            funkin_draw_note(track_array[funkin_notes_player[i].track + 4],
                         DIST_FROM_TOP(funkin_notes_player, i),
                         funkin_notes_player[i].track,
                         funkin_notes_player[i].track
                         );
        }
        else {
            funkin_gen_longnote_player(
                         track_array[funkin_notes_player[i].track + 4],
                         DIST_FROM_TOP(funkin_notes_player, i),
                         funkin_notes_player[i].length,
                         i
                         );
        }
    }
}

// keep 500+ by the end to win
// insta-lose at 0
// 
int score = 500;

void funkin_check_inputs(void) {
    for (int i = 0; i < funkin_player_notecount; i++) {
        if (funkin_notes_player[i].timer_offset < (startTime - TIMER_SWAY)) continue;
        
        f32 dt = funkin_notes_player[i].timer_offset - funkin_hit_timer;
        if (dt < -40.0f) {
            score -= 5;
        }

        if (funkin_notes_player[i].timer_offset - funkin_hit_timer) {

        }
    }
}

#include "chart/chart.c"

void funkin_game_loop(void) {

    funkin_hud();

    funkin_draw_beatmap_from_offset(funkin_timer);

    funkin_check_inputs();

    funkin_timer += 30.0f * 60.0f / (f32)funkin_bpm;
    // funkin_timer+=100;
    if (funkin_timer > 2500.0f) {
        funkin_hit_timer += 30.0f * 60.0f / (f32)funkin_bpm;
    }
}
