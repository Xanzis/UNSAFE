#ifndef _UNDEFS_
#define _UNDEFS_

typedef struct coor coor;
struct coor {
	float x;
	float y;
};

typedef struct node node;
struct node {
	int id;
	coor loc;
};

typedef struct beam beam;
struct beam {
	int id;
	int n1_id;
	int n2_id;
	float length;
	float orig_length; // For unsafe
	float force;
};

typedef struct force force;
struct force {
	int id;
	int n_id;
	float theta;
	float mag;
};

typedef struct constraint constraint;
struct constraint {
	int id;
	int n_id;
	float theta;
	float force;
};

typedef struct wall wall;
struct wall {
	int id;
	float m;
	float b;
	float theta; // normal to wall
	char above; // nonzero if points are constrained to above line
}

typedef struct frame frame;
struct frame {
	int beamcount;
	beam *beams;
	int nodecount;
	node *nodes;
	int forcecount;
	force *forces;
	int constraintcount;
	constraint *constraints;
};

void UN_printcoor(coor c);
void UN_printnode(node n);
void UN_printbeam(beam b);
void UN_printforce(force f);
void UN_printconstraint(constraint c);
void UN_printframe(frame *f);

float UN_dist(coor a, coor b);

void UN_init_frame(frame *res, int bcount, int ncount, int fcount, int ccount);
void UN_free_frame(frame *f);
node* UN_get_node(frame *f, int id);
int UN_get_node_idx(frame *f, int id);
beam* UN_get_beam(frame *f, int id);

void UN_compute_beam_vals(frame *f);
vector* get_forces(frame *f);

#endif