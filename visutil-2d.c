#include <stdio.h>
#include <math.h>
#include "visutil-2d.h"
#include "undefs.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

unsigned char VIS_RED[3] = {255, 0, 0};
unsigned char VIS_GREEN[3] = {0, 255, 0};
unsigned char VIS_BLUE[3] = {0, 0, 255};

void visutilerror(char *error_text) {
	printf("Critical error in visutil-2d.c\nError message follows:\n");
	printf("%s\n", error_text);
	exit(1);
}

plot* VIS_init_plot(int x, int y) {
	plot *res = (plot *) malloc(sizeof (plot));

	res->res_x = x;
	res->res_y = y;
	res->data = (unsigned char *) malloc(3 * x * y * sizeof (unsigned char));
	memset(res->data, 255, 3 * x * y * sizeof (unsigned char));

	return res;
}

void VIS_free_plot(plot *p) {
	free(p->data);
	free(p);
}

void VIS_add_pixel(plot *p, coor c, unsigned char *color) {
	// The bottom left corner is the (x_res * (y_res - 1)) * 3th item in data.
	// Moving up a line subtracts (xres) * 3
	// Moving right a line adds 3.
	// Rounds to nearest pixel coordinate to the supplied coord and sets a pixel

	int pixloc = (p->res_x * (p->res_y - 1)) * 3; // origin

	c.x += p->offsetx;
	c.y += p->offsety;

	int xpos = (int) round((float) c.x * p->scale);
	int ypos = (int) round((float) c.y * p->scale);

	if (xpos >= p->res_x) visutilerror("VIS_add_pixel: xpos too high");
	if (xpos < 0) visutilerror("VIS_add_pixel: xpos too low");
	if (ypos >= p->res_y) visutilerror("VIS_add_pixel: ypos too high");
	if (ypos < 0) visutilerror("VIS_add_pixel: ypos too low");

	pixloc += 3 * xpos;
	pixloc -= 3 * p->res_x * ypos;

	if (color) {
		p->data[pixloc] = color[0];
		p->data[pixloc + 1] = color[1];
		p->data[pixloc + 2] = color[2];
	}
	else {
		p->data[pixloc] = 0;
		p->data[pixloc + 1] = 0;
		p->data[pixloc + 2] = 0;
	}
}

void VIS_set_scale(plot *p, frame *f) {
	float xmax, xmin, ymax, ymin;
	int once = 0;

	coor c;
	for (int i = 0; i < f->nodecount; i++) {
		c = f->nodes[i].loc;
		if (!once) {
			xmax = c.x; xmin = c.x; ymax = c.y; ymin = c.y;
			once = 1;
		}
		else {
			if (c.x > xmax) xmax = c.x;
			if (c.x < xmin) xmin = c.x;
			if (c.y > ymax) ymax = c.y;
			if (c.y < ymin) ymin = c.y;
		}
	}

	float xrange = xmax - xmin;
	float yrange = ymax - ymin;

	float xscale = (float) p->res_x / (xrange * 1.2);
	float yscale = (float) p->res_y / (yrange * 1.2);

	float xbuffer = xmin - (0.1 * xrange);
	float ybuffer = ymin - (0.1 * yrange);

	if (xscale < yscale) {
		p->scale = xscale;
	}
	else {
		p->scale = yscale;
	}
	
	p->offsetx = -1 * xbuffer;
	p->offsety = -1 * ybuffer;
}


void VIS_add_beam(plot *p, frame *f, int beam_id) {
	coor c1, c2;

	beam *b = UN_get_beam(f, beam_id);
	c1 = UN_get_node(f, b->n1_id)->loc;
	c2 = UN_get_node(f, b->n2_id)->loc;
	float force = b->force; // Assuming the frame has already been solved! TODO fix this

	float xdiff = c2.x - c1.x;
	float ydiff = c2.y - c1.y;

	unsigned char color[3] = {0, 0, 0};

	int count = 100;
	for (int i = 0; i <= count; i++) {
		c1.x += xdiff / (float) count;
		c1.y += ydiff / (float) count;

		if (force > 0) {
			color[0] = 0;
			color[1] = (unsigned char) VIS_max((int) 255, (int) 2 * force);
		}
		else {
			color[1] = 0;
			color[0] = (unsigned char) VIS_max((int) 255, (int) -2 * force);
		}
		// Eww Eww Eww Eww TODO fix this

		VIS_add_pixel(p, c1, color);
	}
	// this better just be a placehodler for a better line drawing function 
	// (hopefully with line weights)
}

void VIS_add_node(plot *p, frame *f, int node_id) {
	node *n = UN_get_node(f, node_id);
	VIS_add_pixel(p, n->loc, VIS_BLUE);
}

void VIS_add_frame(plot *p, frame *f) {
	// plot beams
	for (int i = 0; i < f->beamcount; i++) {
		VIS_add_beam(p, f, f->beams[i].id);
	}

	//plot nodes
	for (int i = 0; i < f->nodecount; i++) {
		VIS_add_node(p, f, f->nodes[i].id);
	}
}

void VIS_save_png(plot *p) {
	stbi_write_png("out.png", p->res_x, p->res_y, 3, p->data, p->res_x * 3);
}