#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "undefs.h"

void UN_printcoor(coor c) {
	printf("<Coordinate X%8.4f Y%8.4f>", c.x, c.y);
}

void UN_printnode(node n) {
	printf("Node id %05d at ", n.id);
	UN_printcoor(n.loc);
}

void UN_printbeam(beam b) {
	printf("Beam id %05d. Connecting nodes %05d %05d", b.id, b.n1_id, b.n2_id);
}

void UN_printforce(force f) {
	printf("Force id %05d. Angle %8.4f Magnitude %8.4f", f.id, f.theta, f.mag);
}

void UN_printconstraint(constraint c) {
	printf("Constraint id %05d. Node id %05d Angle %8.4f", c.id, c.n_id, c.theta);
}

void UN_printframe(frame *f) {
	printf("Frame readout");
	printf("\nNodes:");
	for (int i = 0; i < f->nodecount; i++) {
		printf("\n    ");
		UN_printnode(f->nodes[i]);
	}
	printf("\nBeams:");
	for (int i = 0; i < f->beamcount; i++) {
		printf("\n    ");
		UN_printbeam(f->beams[i]);
	}
	printf("\nForces:");
	for (int i = 0; i < f->forcecount; i++) {
		printf("\n    ");
		UN_printforce(f->forces[i]);
	}
	printf("\nConstraints:");
	for (int i = 0; i < f->constraintcount; i++) {
		printf("\n    ");
		UN_printconstraint(f->constraints[i]);
	}
	printf("\n----\n");
}

float UN_dist(coor a, coor b) {
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

void UN_init_frame(frame *res, int bcount, int ncount, int fcount, int ccount, int wcount) {
	// frame *res = (frame *) malloc(sizeof (frame)); //Switching to allocation in main()
	res->beams = (beam *) malloc(bcount * sizeof (beam));
	res->beamcount = bcount;
	res->nodes = (node *) malloc(ncount * sizeof (node));
	res->nodecount = ncount;
	res->forces = (force *) malloc(fcount * sizeof (force));
	res->forcecount = fcount;
	res->constraints = (constraint *) malloc(ccount * sizeof (constraint));
	res->constraintcount = ccount;
	res->walls = (wall *) malloc(wcount * sizeof (wall));
	res->wallcount = wcount;
}

void UN_free_frame(frame *f) {
	free(f->beams);
	free(f->nodes);
	free(f->forces);
	free(f->constraints);
	free(f->walls);
	free(f);
}

node* UN_get_node(frame *f, int id) {
	for (int i = 0; i < f->nodecount; i++) {
		if (f->nodes[i].id == id) {
			return f->nodes + i;
		}
	}
	return NULL;
}

int UN_get_node_idx(frame *f, int id) {
	for (int i = 0; i < f->nodecount; i++) {
		if (f->nodes[i].id == id) {
			return i;
		}
	}
	return -1;
}

beam* UN_get_beam(frame *f, int id) {
	for (int i = 0; i < f->beamcount; i++) {
		if (f->beams[i].id == id) {
			return f->beams + i;
		}
	}
	return NULL;
}

void UN_compute_beam_vals(frame *f) {
	coor a, b;
	for (int i = 0; i < f->beamcount; i++) {
		a = UN_get_node(f, f->beams[i].n1_id)->loc;
		b = UN_get_node(f, f->beams[i].n2_id)->loc;
		f->beams[i].length = UN_dist(a, b);
	}
}

vector* UN_get_forces(frame *f) {
	// Takes forces defined in f and returns a vector of forces on each node
	int offset = f->nodecount; // offset to y_forces[0]
	vector *res = MAT_vector(offset * 2, MAT_YES);
	int idx;
	force frc;
	for (int i = 0; i<f->forcecount; i++) {
		frc = f->forces[i];
		idx = UN_get_node_idx(f, frc.n_id);
		res->vec[idx] += frc.mag * cos(frc.theta);
		res->vec[idx + offset] += frc.mag * sin(frc.theta);
	}
	return res;
}