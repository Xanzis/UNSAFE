#ifndef _VISUTIL_
#define _VISUTIL_

#include "undefs.h"

unsigned char VIS_RED[3] = {255, 0, 0};
unsigned char VIS_GREEN[3] = {0, 255, 0};
unsigned char VIS_BLUE[3] = {0, 0, 255};

typedef struct plot plot;
struct plot {
	int res_x;
	int res_y;
	float scale;
	float offsetx;
	float offsety;
	unsigned char *data;
}

plot* VIS_init_plot(int x, int y);
void VIS_set_scale(plot *p, frame *f);
void VIS_add_pixel(plot *p, coor c);

void VIS_add_beam(plot *p, frame *f, int beam_id);
void VIS_add_node(plot *p, frame *f, int node_id);

void VIS_add_frame(plot *p, frame *f);

void VIS_save_png(plot *p);
#endif