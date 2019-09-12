#include <stdio.h>
#include "visutil-2d.h"

plot* VIS_init_plot(int x, int y);
void VIS_set_scale(plot *p, frame *f);
void VIS_add_pixel(plot *p, coor c);

void VIS_add_beam(plot *p, frame *f, int beam_id);
void VIS_add_node(plot *p, frame *f, int node_id);

void VIS_add_frame(plot *p, frame *f);

void VIS_save_png(plot *p);