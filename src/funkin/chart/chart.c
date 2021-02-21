#include "funkin/funkin.h"
int funkin_bpm = 100;
struct funkin_note funkin_notes_player[] = {
	{4900.000000, 2, 0},
	{5500.000000, 3, 450},
	{5950.000000, 3, 600},
	{9700.000000, 1, 300},
	{10300.000000, 0, 300},
	{10900.000000, 3, 300},
	{14500.000000, 1, 150},
	{15100.000000, 3, 0},
	{15400.000000, 0, 0},
	{15700.000000, 1, 450},
	{19300.000000, 3, 0},
	{19600.000000, 1, 0},
	{20200.000000, 0, 0},
	{20500.000000, 2, 300},
	{24100.000000, 0, 0},
	{24400.000000, 3, 0},
	{24700.000000, 1, 750},
	{29200.000000, 3, 0},
	{29500.000000, 0, 750},
	{28900.000000, 1, 0},
	{33700.000000, 2, 0},
	{34000.000000, 3, 0},
	{34300.000000, 0, 1050},
	{38500.000000, 0, 0},
	{38800.000000, 3, 0},
	{39400.000000, 2, 0},
	{39700.000000, 1, 450},
	{43300.000000, 2, 450},
	{43900.000000, 3, 300},
	{44500.000000, 0, 450},
	{48100.000000, 1, 0},
	{48700.000000, 2, 0},
	{49300.000000, 1, 0},
	{49450.000000, 1, 0},
	{49600.000000, 1, 0},
	{49900.000000, 2, 0},
	{52900.000000, 2, 450},
	{53500.000000, 3, 300},
	{54100.000000, 0, 450},
	{57700.000000, 3, 1650},
	{58150.000000, 4, 0},
	{62500.000000, 2, 0},
	{62800.000000, 3, 0},
	{63100.000000, 0, 1050},
	{64600.000000, 6, 0},
	{67300.000000, 0, 0},
	{67600.000000, 3, 0},
	{68200.000000, 2, 0},
	{68500.000000, 1, 450},
	{72100.000000, 2, 0},
	{72400.000000, 3, 0},
	{72700.000000, 0, 0},
	{73000.000000, 2, 0},
	{73300.000000, 1, 450},
	{74200.000000, 6, 0},
	{76900.000000, 0, 0},
	{77200.000000, 3, 0},
	{77800.000000, 2, 0},
	{78100.000000, 1, 450},
};
int funkin_cpu_notecount = 59;

struct funkin_note funkin_notes_cpu[] = {
	{2500.000000, 2, 0},
	{3100.000000, 3, 450},
	{3550.000000, 3, 600},
	{7300.000000, 1, 300},
	{7900.000000, 0, 300},
	{8500.000000, 3, 300},
	{12100.000000, 1, 150},
	{12700.000000, 3, 0},
	{13000.000000, 0, 0},
	{13300.000000, 1, 450},
	{16900.000000, 3, 0},
	{17200.000000, 1, 0},
	{17800.000000, 0, 0},
	{18100.000000, 2, 300},
	{21700.000000, 0, 0},
	{22000.000000, 3, 0},
	{22300.000000, 1, 750},
	{26500.000000, 1, 0},
	{26800.000000, 3, 0},
	{27100.000000, 0, 750},
	{31300.000000, 2, 0},
	{31600.000000, 3, 0},
	{31900.000000, 0, 1050},
	{36100.000000, 0, 0},
	{36400.000000, 3, 0},
	{37000.000000, 2, 0},
	{37300.000000, 1, 450},
	{40900.000000, 2, 450},
	{41500.000000, 3, 300},
	{42100.000000, 0, 450},
	{45700.000000, 1, 0},
	{46300.000000, 2, 0},
	{46900.000000, 1, 0},
	{47050.000000, 1, 0},
	{47200.000000, 1, 0},
	{47500.000000, 2, 0},
	{50500.000000, 2, 450},
	{51100.000000, 3, 300},
	{51700.000000, 0, 450},
	{55300.000000, 3, 1650},
	{60100.000000, 2, 0},
	{60400.000000, 3, 0},
	{60700.000000, 0, 1050},
	{64900.000000, 0, 0},
	{65200.000000, 3, 0},
	{65800.000000, 2, 0},
	{66100.000000, 1, 450},
	{67000.000000, 6, 0},
	{69700.000000, 2, 0},
	{70000.000000, 3, 0},
	{70300.000000, 0, 0},
	{70600.000000, 2, 0},
	{70900.000000, 1, 450},
	{74500.000000, 0, 0},
	{74800.000000, 3, 0},
	{75400.000000, 2, 0},
	{75700.000000, 1, 450},
	{76600.000000, 6, 0},
};
int funkin_player_notecount = 58;
