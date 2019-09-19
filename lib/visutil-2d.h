#ifndef _VISUTIL_
#define _VISUTIL_

#include "undefs.h"

#define VIS_max(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

typedef struct plot plot;
struct plot {
	int res_x;
	int res_y;
	float scale;
	float offsetx;
	float offsety;
	unsigned char *data;
};

void visutilerror(char *error_text);

plot* VIS_init_plot(int x, int y);
void VIS_free_plot(plot *p);
void VIS_set_scale(plot *p, frame *f);
void VIS_add_pixel(plot *p, coor c, unsigned char *color);

void VIS_add_beam(plot *p, frame *f, int beam_id);
void VIS_add_node(plot *p, frame *f, int node_id);

void VIS_add_frame(plot *p, frame *f);

void VIS_save_png(plot *p);
#endif