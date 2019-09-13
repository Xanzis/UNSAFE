#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matutil.h"
#include "inutil-r.h"
#include "undefs.h"
#include "visutil-2d.h"

void unsafeerror(char *error_text) {
	printf("Critical error in unsafe-r.c\nError message follows:\n");
	printf("%s\n", error_text);
	exit(1);
}

vector* get_forces(frame *f) {
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

matrix* build_connectivity_matrix(frame *f) {
	// Takes cmat (either NULL or preinitialized matrix) and fills out with connectivity matrix
	//    described by the beam-node connections in f
	// Such that for the resulting matrix M, node_net_forces = M . [beam_net_forces, constraint_forces]
	// Where beam_net_forces is sequential as in f, and node_net forces is all x sequential first,
	//    then all y of those nodes

	// The last three values of the beam forces vector are the forces out of the three constraints applied to the system.

	// Check the inputs are valid
	int beamcount = f->beamcount;
	int nodecount = f->nodecount;
	if (2 * (nodecount) - 3 != beamcount) {
		fprintf(stderr, "Frame nodecount %d Frame beamcount %d\n", f->nodecount, beamcount);
		unsafeerror("Could not build connectivity matrix.\nBeamcount must be 2n - 3 for solvable matrix");
	}
	
	matrix *cmat = MAT_matrix(nodecount * 2, nodecount * 2, MAT_YES);

	// Done with format/error checking

	// Populate the matrix
	// A row has one equation. The column index is the same as the beam index
	beam b;
	node *n_1;
	node *n_2;
	int n1_idx, n2_idx;
	float coeff_x, coeff_y;
	int offset = nodecount; // Y forces are node_number + offset
	for (int i = 0; i < beamcount; i++) {
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

	// Populate the last three columns of the matrix with constraint information
	int cst_offset = nodecount * 2 - 3;
	if (f->constraintcount != 3) {
		fprintf(stderr, "Constraint count: %d\n", f->constraintcount);
		unsafeerror("System must have three constraints");
	}
	constraint c;
	for (int i = 0; i < f->constraintcount; i++) {
		c = f->constraints[i];
		n1_idx = UN_get_node_idx(f, c.n_id);

		// x coefficient of constraint force
		cmat->mat[n1_idx][i + cst_offset] = cos(c.theta);
		// y coefficient of constraint force
		cmat->mat[n1_idx + offset][i + cst_offset] = sin(c.theta);
	}

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

	printf("Collecting node forces ... ");
	vector * node_forces = get_forces(f);
	printf("Done.\n");

	MAT_printvector(node_forces);

	MAT_printmatrix(con_mat);

	printf("Here goes. Solving beam stresses ... ");
	vector *stress_solutions = MAT_solve_gausselim(con_mat, node_forces);
	printf("\nDone.");

	// Fill in the force values
	for (int i = 0; i < f->beamcount; i++) {
		f->beams[i].force = stress_solutions->vec[i];
	}
	for (int i = 0; i < 3; i++) {
		f->constraints[i].force = stress_solutions->vec[f->beamcount + i];
	}

	MAT_printvector(stress_solutions);

	// Visualize the resulting frame and save to file
	plot *plt = VIS_init_plot(400, 200);
	VIS_set_scale(plt, f); // TODO Make this more intuitive / hide this function - check if performed at first add_frame?
	VIS_add_frame(plt, f);
	VIS_save_png(plt);

	VIS_free_plot(plt);

	MAT_freevector(node_forces);
	MAT_freevector(stress_solutions);
	MAT_freematrix(con_mat);
	UN_free_frame(f);
	return 1;
}