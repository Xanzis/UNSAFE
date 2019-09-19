#include <stdio.h>
#include <stdlib.h>
#include "../lib/matutil.h"
#include "../lib/inutil-r.h"

int matutil() {
	printf("Testing Matutil ...\n");
	float testmat_def[9] = {22, 15, 3, -3, -1, 0, -2, 1, 0};
	matrix *testmatrix = MAT_matrix(3, 3, 0);
	matrix *backupmat = MAT_matrix(3, 3, 0);
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 3; i++) {
			testmatrix->mat[j][i] = testmat_def[i + (3 * j)];
			backupmat->mat[j][i] = testmat_def[i + (3 * j)];
		}
	}

	float testvec_def[3] = {8, -11, 3};
	vector *testvec = MAT_vector(3, 0);
	vector *backupvec = MAT_vector(3, 0);
	for (int i = 0; i < 3; i++) {
		testvec->vec[i] = testvec_def[i];
		backupvec->vec[i] = testvec_def[i];
	}

	//printf("Test values\n");
	//MAT_printmatrix(testmatrix);
	//MAT_printvector(testvec);

	printf("Here goes\n");

	vector *res = MAT_solve_gausselim(testmatrix, testvec);

	printf("Solution values\n");
	MAT_printvector(res);

	vector *rec_b = MAT_multiply_mv(testmatrix, res);
	printf("Recreated b vector (From A . x)\n");
	MAT_printvector(rec_b);

	MAT_freevector(rec_b);

	rec_b = MAT_multiply_mv(backupmat, res);
	printf("Backup, unaltered A:\n");
	MAT_printmatrix(backupmat);
	printf("Reminder: starting b vector\n");
	MAT_printvector(backupvec);
	printf("Recreated b vector from original A:\n");
	MAT_printvector(rec_b);

	printf("Freeing memory ... ");
	MAT_freematrix(testmatrix);
	MAT_freematrix(backupmat);
	MAT_freevector(testvec);
	MAT_freevector(backupvec);
	MAT_freevector(res);
	MAT_freevector(rec_b);
	printf("Done.\n");

	return 0;
}

int inutil() {
	printf("Testing Inutil ... \n");
	table *framevals = IN_load_table("frame1.us");
	printf("Frame loaded.\n");
	section *sct = IN_find_section(framevals, "Beams");
	printf("Beams found\n");
	item *it = IN_get_item(sct, 2);
	printf("Line id 2 found\n");
	printf("First item of line id 2: %d\n", IN_get_int(it, 0));

	sct = IN_find_section(framevals, "Forces");
	printf("Forces found\n");
	it = IN_get_item(sct, 0);
	printf("Line id 0 found\n");
	printf("Second item of line id 2: %f\n", IN_get_float(it, 1));

	sct = IN_find_section(framevals, "Nodes");
	printf("Nodes found\n");
	it = IN_get_item(sct, 1);
	printf("Line id 0 found\n");
	printf("Second item of line id 2: %f\n", IN_get_float(it, 1));

	printf("Freeing table ... ");
	IN_free_table(framevals);
	printf("Done.\n");
	return 0;
}

int main() {
	matutil();
	inutil();
	return 0;
}