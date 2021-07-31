#ifndef FUNKIN_H
#define FUNKIN_H

// Change how many notes can be on screen;
// a higher number will take up more RAM
#define MAX_SPRITE_COUNT 300

#define GRN 0
#define RED 1
#define TRQ 2
#define BLU 3
#define TEMPL 4

#define GRN_COL 0x57bf6fFF
#define RED_COL 0xf9393fFF
#define TRQ_COL 0x6dc0c7FF
#define BLU_COL 0x7e6ab5FF
#define TEMPL_COL 0x87a3adFF

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#define CPU_TRACK_BASE 16
#define TRACK1 (CPU_TRACK_BASE + (0 * 32) + 0)
#define TRACK2 (CPU_TRACK_BASE + (1 * 32) + 1)
#define TRACK3 (CPU_TRACK_BASE + (2 * 32) + 2)
#define TRACK4 (CPU_TRACK_BASE + (3 * 32) + 3)

#define PLAYER_TRACK_BASE 168
#define TRACK5 (PLAYER_TRACK_BASE + (0 * 32) + 0)
#define TRACK6 (PLAYER_TRACK_BASE + (1 * 32) + 1)
#define TRACK7 (PLAYER_TRACK_BASE + (2 * 32) + 2)
#define TRACK8 (PLAYER_TRACK_BASE + (3 * 32) + 3)

extern u16 note_pal_tex_0[];

void funkin_draw_note(int x, int y, int color, int direction);
extern int noteIndex;

// i honestly dont want to bitshift so lets do a union
typedef union Color {
	u32 u;
	u8 c[4];
} Color;

// chart structs

extern float funkin_bpm;

#define FUNKIN_CPU 0
#define FUNKIN_BF  1

struct funkin_note {
	u8 who_sings;
	u8 track;
	u16 length;
	f32 timer_offset;
	f32 timeHit;
	f32 timeReleased;
};
extern int funkin_notecount;

extern struct funkin_note funkin_notes[];
extern struct funkin_note funkin_notes_player[];

#endif