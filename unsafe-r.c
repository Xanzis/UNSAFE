#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matutil.h"
#include "inutil-r.h"
#include "undefs.h"

void unsafeerror(char *error_text) {
	printf("Critical error in unsafe-r.c\nError message follows:\n");
	printf("%s\n", error_text);
	exit(1);
}

vector* solve_forces(frame *f) {
	// Takes force and constraint inputs of f and solves for the frame forces
	// There should be <=n forces and strictly three constraints
	// Returns a vector of x components first, then y components by index of nodes in frame

	float net_x = 0;
	float net_y = 0;
	float net_torque = 0;

	coor origin = (coor) {0, 0};
	// Torque magnitude is A B sin theta
	force frc;
	float theta_1, len_1, theta;
	coor p_vec;
	for (int i = 0; i < f->forcecount; i++) {
		frc = f->forces[i];
		p_vec = UN_get_node(f, frc.n_id)->loc;
		theta_1 = atan2(p_vec.y, p_vec.x); // Angle to position vector (correct sign)
		theta = theta_1 - frc.theta; // Angle between position and force vectors
		len_1 = UN_dist(origin, p_vec); // Length of p_vec. Inefficient as hell, I know TODO speedup

		net_torque += len_1 * frc.mag * sin(theta); // Add on the torque applied by F

		net_x += frc.mag * cos(frc.theta);
		net_y += frc.mag * sin(frc.theta); // Maybe precompute these later
	}

	// Build matrix and solve
	if (f->constraintcount != 3) {
		UN_printframe(f);
		unsafeerror("Frame must have 3 constraints");
	}
	matrix *cst_mat = MAT_matrix(3, 3, MAT_YES);
	vector *net_vals = MAT_vector(3, MAT_NO); // Net x, net y, net torque
	net_vals->vec[0] = -1 * net_x; // Constraint solutions must counteract the forces
	net_vals->vec[1] = -1 * net_y;
	net_vals->vec[2] = -1 * net_torque;

	constraint cst;

	for (int i = 0; i < f->constraintcount; i++) {
		cst = f->constraints[i];
		p_vec = UN_get_node(f, cst.n_id)->loc;
		theta_1 = atan2(p_vec.y, p_vec.x);
		theta = theta_1 - cst.theta;

		cst_mat->mat[0][i] = cos(cst.theta); // coefficient for x force
		cst_mat->mat[1][i] = sin(cst.theta); // coefficient for y force
		cst_mat->mat[2][i] = UN_dist(origin, p_vec) * sin(theta); // coefficient for torque
	}

	// Solve this system
	vector *constraintvals;
	constraintvals = MAT_solve_gausselim(cst_mat, net_vals);

	MAT_printvector(constraintvals);

	// Fill out the force table
	int offset = f->nodecount; // offset to y_forces[0]
	vector *res = MAT_vector(offset * 2, MAT_YES);
	int idx;
	for (int i = 0; i<f->forcecount; i++) {
		frc = f->forces[i];
		idx = UN_get_node_idx(f, frc.n_id);
		res->vec[idx] += frc.mag * cos(frc.theta);
		res->vec[idx + offset] += frc.mag * sin(frc.theta);
	}
	for (int i = 0; i<f->constraintcount; i++) {
		cst = f->constraints[i];
		idx = UN_get_node_idx(f, cst.n_id);
		res->vec[idx] += constraintvals->vec[i] * cos(cst.theta); // Using the solved constraint magnitudes
		res->vec[idx + offset] += constraintvals->vec[i] * sin(cst.theta);
	}

	MAT_freematrix(cst_mat);
	MAT_freevector(constraintvals);
	MAT_freevector(net_vals);

	return res;
}

matrix* build_connectivity_matrix(frame *f) {
	// Takes cmat (either NULL or preinitialized matrix) and fills out with connectivity matrix
	//    described by the beam-node connections in f
	// Such that for the resulting matrix M, node_net_forces = M . beam_net_forces
	// Where beam_net_forces is sequential as in f, and node_net forces is all x sequential first,
	//    then all y of those nodes

	// Check the inputs are valid
	int beamcount = f->beamcount;
	if (beamcount % 2) unsafeerror("Beamcount must be even");
	if (2 * (f->nodecount) != beamcount) {
		fprintf(stderr, "Frame nodecount %d Frame beamcount %d\n", f->nodecount, beamcount);
		unsafeerror("Could not build connectivity matrix.\nBeamcount must be double nodecount for solvable matrix");
	}
	
	matrix *cmat = MAT_matrix(beamcount, beamcount, MAT_YES);

	// Done with format/error checking

	// Populate the matrix
	// A row has one equation. The column index is the same as the beam index
	beam b;
	node *n_1;
	node *n_2;
	int n1_idx, n2_idx;
	float coeff_x, coeff_y;
	int offset = beamcount / 2; // Y forces are node_number + offset
	for (int i = 0; i < f->beamcount; i++) {
		b = f->beams[i];
		n_1 = UN_get_node(f, b.n1_id); // Get the nodes the beam connects to
		n_2 = UN_get_node(f, b.n2_id);

		n1_idx = UN_get_node_idx(f, b.n1_id);
		n2_idx = UN_get_node_idx(f, b.n2_id);

		coeff_x = (float) (n_1->loc.x - n_2->loc.x) / (float) b.length;
		cmat->mat[n1_idx][i] = coeff_x; // Node 1 x force adds coeff of beam[i] stress
		cmat->mat[n2_idx][i] = -1 * coeff_x; // Node 2 x force is the inverse of the force on node 1

		coeff_y = (float) (n_1->loc.y - n_2->loc.y) / (float) b.length;
		cmat->mat[n1_idx + offset][i] = coeff_y; // Set the y coefficient
		cmat->mat[n2_idx + offset][i] = coeff_y;
	}
	// x and y connections should now be populated.
	// Uhhhhhh that oughta be it
	return cmat;
}

void setup(frame *f, char *fileloc) {
	// Read in the file and carry the inutil data over into the unsafe frame
	printf("Reading file ... ");
	table *ftable = IN_load_table(fileloc);
	printf("Done.\n");
	printf("Filling out table ... ");
	int ncount, bcount, fcount, ccount;
	section *nsect, *bsect, *fsect, *csect;
	nsect = IN_find_section(ftable, "Nodes");
	bsect = IN_find_section(ftable, "Beams");
	fsect = IN_find_section(ftable, "Forces");
	csect = IN_find_section(ftable, "Constraints");
	UN_init_frame(f, bsect->itemcount, nsect->itemcount, fsect->itemcount, csect->itemcount);

	item* temp_item_ptr;

	// Populate nodes
	coor temp_coor;
	
	for (int i = 0; i<nsect->itemcount; i++) {
		temp_item_ptr = nsect->items + i;
		temp_coor = (coor) {IN_get_float(temp_item_ptr, 0), IN_get_float(temp_item_ptr, 1)};
		f->nodes[i] = (node) {temp_item_ptr->id, temp_coor};
	}
	// Populate beams
	for (int i = 0; i<bsect->itemcount; i++) {
		temp_item_ptr = bsect->items + i;
		f->beams[i] = (beam) {temp_item_ptr->id, IN_get_int(temp_item_ptr, 0), 
			IN_get_int(temp_item_ptr, 1)};
	}
	// Beam length still needs seperate evaluation once frame is loaded
	// Populate forces
	for (int i = 0; i<fsect->itemcount; i++) {
		temp_item_ptr = fsect->items + i;
		f->forces[i] = (force) {temp_item_ptr->id, IN_get_int(temp_item_ptr, 0), 
			IN_get_float(temp_item_ptr, 1), IN_get_float(temp_item_ptr, 2)};
	}
	// Populate constraints
	for (int i = 0; i<csect->itemcount; i++) {
		temp_item_ptr = csect->items + i;
		f->constraints[i] = (constraint) {temp_item_ptr->id, IN_get_int(temp_item_ptr, 0), 
			IN_get_float(temp_item_ptr, 1)};
	}

	// Ensure beam references are all sanitary
	for (int i = 0; i < f->beamcount; i++) {
		if (UN_get_node_idx(f, f->beams[i].n1_id) == -1) unsafeerror("Bad node reference in beam");
		if (UN_get_node_idx(f, f->beams[i].n2_id) == -1) unsafeerror("Bad node reference in beam");
	}
	printf("Done.\n");

	printf("Computing beam values ... ");
	// Calculate beam values (other precomputation should occur here)
	UN_compute_beam_vals(f);
	printf("Done.\n");

	// Free all allocated inutil space
	IN_free_table(ftable);
}

int main() {
	frame *f = (frame *) malloc(sizeof (frame));
	setup(f, "boxframe.us");
	printf("Building connectivity matrix ... ");
	matrix *con_mat = build_connectivity_matrix(f);
	printf("Done.\n");
	printf("Setup complete.\n");

	UN_printframe(f);

	printf("Solving node forces ... ");
	vector * node_forces = solve_forces(f);
	printf("Done.\n");

	MAT_printvector(node_forces);

	MAT_freevector(node_forces);
	MAT_freematrix(con_mat);
	UN_free_frame(f);
	return 1;
}