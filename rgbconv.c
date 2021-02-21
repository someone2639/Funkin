#include <stdio.h>

#define CLAMP(x) ((x) * (255 / 32))

#define R(x) CLAMP(((x) >> 11) & 0x1F)
#define G(x) CLAMP(((x) >> 06) & 0x1F)
#define B(x) CLAMP(((x) >> 01) & 0x1F)
#define A(x) CLAMP(((x) >> 00) & 0x01)

int main(int argc, char **argv) {
	int r;
	sscanf(argv[1], "0x%04x", &r);
	printf("%02X%02X%02X%02X\n", R(r), G(r), B(r), A(r));
}