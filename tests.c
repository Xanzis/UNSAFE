#include <stdio.h>
#include <stdlib.h>
#include "matutil.h"

int matutil() {
	printf("Testing Matutil ...\n")
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

	printf("Test values\n");
	MAT_printmatrix(testmatrix);
	MAT_printvector(testvec);

	printf("Here goes\n");

	vector *res = MAT_solve_gausselim(testmatrix, testvec);

	printf("Solution values\n");
	MAT_printvector(res);

	vector *rec_b = MAT_multiply_mv(testmatrix, res);
	printf("Recreated b vector (From A . x)\n");
	MAT_printvector(rec_b);

	rec_b = MAT_multiply_mv(backupmat, res);
	printf("Backup, unaltered A:\n");
	MAT_printmatrix(backupmat);
	printf("Reminder: starting b vector\n");
	MAT_printvector(backupvec);
	printf("Recreated b vector from original A:\n");
	MAT_printvector(rec_b);

	return 0;
}

int main() {
	matutil();
	return 0;
}