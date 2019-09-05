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
	// Maybe compute angle, too: we'll see what stuff is needed
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
};

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

frame* UN_init_frame(int bcount, int ncount, int fcount, int ccount);
void UN_free_frame(frame *f);
node* UN_get_node(frame *f, int id);
beam* UN_get_beam(frame *f, int id);

void UN_compute_beam_vals(frame *f);

#endif