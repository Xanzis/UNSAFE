#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lib/matutil.h"
#include "lib/inutil-r.h"
#include "lib/undefs.h"
#include "lib/visutil-2d.h"

void unsafeerror(char *error_text) {
	printf("Critical error in unsafe.c\nError message follows:\n");
	printf("%s\n", error_text);
	exit(1);
}

matrix* build_connectivity_matrix(frame *f) {
	// returns a connectivity matrix for frame f
	// This particular matrix is not square; it is rectangular such that
	//    M <beam forces> = <node forces (x then y)>

	int beamcount = f->beamcount;
	int nodecount = f->nodecount;
	
	matrix *cmat = MAT_matrix(nodecount * 2, beamcount);

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
		cmat->mat[n2_idx + offset][i] = -1 * coeff_y;
	}
	// x and y connections should now be populated.
	return cmat;
}

void setup(frame *f, char *fileloc) {
	// Read in the file and carry the inutil data over into the unsafe frame
	// Largely identical to the unsafe-r counterpart, with walls in place of constraints.
	printf("Reading file ... ");
	table *ftable = IN_load_table(fileloc);
	printf("Done.\n");
	printf("Filling out table ... ");
	int ncount, bcount, fcount, ccount, wcount;
	section *nsect, *bsect, *fsect, *wsect;
	nsect = IN_find_section(ftable, "Nodes");
	bsect = IN_find_section(ftable, "Beams");
	fsect = IN_find_section(ftable, "Forces");
	wsect = IN_find_section(ftable, "Walls");
	UN_init_frame(f, bsect->itemcount, nsect->itemcount, fsect->itemcount, 0, fsect->wallcount);

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
	// Populate walls
	for (int i = 0; i<wsect->itemcount; i++) {
		temp_item_ptr = wsect->items + i;
		f->walls[i] = (wall) {temp_item_ptr->id, IN_get_float(temp_item_ptr, 0), 
			IN_get_float(temp_item_ptr, 1), IN_get_float(temp_item_ptr, 2),
			(char) IN_get_int(temp_item_ptr, 3)};
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
	setup(f, "examples/unsafetest.us");

	return 1;
}