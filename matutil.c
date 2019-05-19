// Collection of Linear Algebra Utilities
// Amended from CFD/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matutil.h"

#define MAT_YES 1
#define MAT_NO 0

// Working with floats until further notice
// All vectors and matrices are zero-indexed.
// Vectors are column vectors until further notice
// Matrices are indexed: mat[row][col]
// To help with memory management, all matrix/vector function arguments are pointers

void matutilerror(char *error_text) {
	printf("Critical error in matutil.c\nError message follows:\n");
	printf("%s\n", error_text);
	exit(1);
}

matrix* MAT_matrix(int r, int c, int init_zeros) {
	matrix *mp = (matrix *) malloc(sizeof (matrix));
	if (!mp) matutilerror("MAT_matrix: failure to allocate mp");

	float **rows = (float **) malloc(r * sizeof (float *));
	if (!rows) matutilerror("MAT_matrix: failure to allocate rows");
	
	float *row;
	for (int i = 0; i < r; i++) {
		row = (float *) malloc(c * sizeof (float));
		if (!row) matutilerror("MAT_matrix: failure to allocate row");
		rows[i] = row;
	}

	*mp = (matrix) {r, c, rows};

	if (init_zeros) MAT_zeromatrix(mp);
	return mp;
}

vector* MAT_vector(int r, int init_zeros) {
	vector *vp = (vector *) malloc(sizeof (vector));

	float *rows = (float *) malloc(r * sizeof (float));
	if (!rows) matutilerror("MAT_vector: failure to allocate rows");

	*vp = (vector) {r, rows};

	if (init_zeros) MAT_zerovector(vp);
	return vp;
}

void MAT_freematrix(matrix *m) {
	float **rows = m->mat;
	for (int i = 0; i < m->rows; i++) {
		free(rows[i]);
	}
	free(rows);
	free(m);
}

void MAT_freevector(vector *v) {
	free(v->vec);
	free(v);
}

void MAT_zeromatrix(matrix *m){
	for (int i = 0; i < m->rows; i++) {
		for (int j = 0; j < m->cols; j++) {
			m->mat[i][j] = (float) 0;
		}
	}
}

void MAT_zerovector(vector *v){
	for (int i = 0; i < v->rows; i++) {
		v->vec[i] = (float) 0;
	}
}

void MAT_printmatrix(matrix *m) {
	printf("\n");
	for (int i = 0; i < m->rows; i++) {
		for (int j = 0; j < m->cols; j++) {
			printf("%6.3f  ", m->mat[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void MAT_printvector(vector *v) {
	printf("\n");
	for (int i = 0; i < v->rows; i++) {
		printf("%6.3f\n", v->vec[i]);
	}
	printf("\n");
}

void MAT_addmatrix(matrix *ma, matrix *mb) {
	// In-place addition (ma is target)
	if (ma->rows != mb->rows || ma->cols != mb->cols) matutilerror("MAT_addmatrix: misaligned matrix sizes");
	for (int i = 0; i < ma->rows; i++) {
		for (int j = 0; j < ma->cols; j++) {
			ma->mat[i][j] += mb->mat[i][j];
		}
	}
}

void MAT_addvector(vector *va, vector *vb) {
	// In-place addition (va is target)
	if (va->rows != vb->rows) matutilerror("MAT_addvector: misaligned vector sizes");
	for (int i = 0; i < va->rows; i++) {
		va->vec[i] += vb->vec[i];
	}
}

void MAT_multiply_ms(matrix *m, float s) {
	// In-place scalar multiplication
	for (int i = 0; i < m->rows; i++) {
		for (int j = 0; j < m->cols; j++) {
			m->mat[i][j] *= s;
		}
	}
}

void MAT_multiply_vs(vector *v, float s) {
	// In-place scalar multiplication
	for (int i = 0; i < v->rows; i++) {
		v->vec[i] *= s;
	}
}

matrix* MAT_copymatrix(matrix *m) {
	// Initialize a new matrix, copy values, and return pointer to new matrix
	matrix *new = MAT_matrix(m->rows, m->cols, MAT_NO);
	for (int i = 0; i < m->rows; i++) {
		for (int j = 0; j < m->cols; j++) {
			new->mat[i][j] = m->mat[i][j];
		}
	}
	return new;
}

vector* MAT_copyvector(vector *v) {
	// Initialize a new vector, copy values, and return pointer to new vector
	vector *new = MAT_vector(v->rows, MAT_NO);
	for (int i = 0; i < v->rows; i++) {
		new->vec[i] = v->vec[i];
	}
	return new;
}

matrix* MAT_multiply_mm(matrix *ma, matrix *mb) {
	// TODO: If improved time performance is required, switch to a sub-cubic algorithm
	if (ma->cols != mb->rows) matutilerror("MAT_multiply_mm: input matrices misaligned");

	matrix *res = MAT_matrix(ma->rows, mb->cols, MAT_NO);
	for (int res_row = 0; res_row < ma->rows; res_row++) {
		for (int res_col = 0; res_col < mb->cols; res_col++) {
			for (int i = 0; i < ma->cols; i++) {
				res->mat[res_row][res_col] += ma->mat[res_row][i] * mb->mat[i][res_col];
			}
		}
	}

	return res;
}

vector* MAT_multiply_mv(matrix *m, vector *v) {
	if (m->cols != v->rows) matutilerror("MAT_multiply_mv: input matrix / vector misaligned");

	vector *res = MAT_vector(m->rows, MAT_NO);

	for (int res_row = 0; res_row < m->rows; res_row++) {
		for (int i = 0; i < m->cols; i++) {
			res->vec[res_row] += m->mat[res_row][i] * v->vec[i];
		}
	}

	return res;
}

vector* MAT_solve_gausselim(matrix *m, vector *v) {
	// Gaussian elimination and backsubstitution. In a system A . x = b, This algorithm finds x given A *and* b.

	int n = m->cols;

	if (m->rows != n) matutilerror("MAT_solve_gausselim: input matrix is not square");
	if (v->rows != n) matutilerror("MAT_solve_gausselim: input vector / matrix sizes misaligned");

	vector *x = MAT_vector(n, MAT_NO);
	matrix *a = MAT_copymatrix(m);
	vector *b = MAT_copyvector(v);

	int max_row;
	float max_value;
	float temp; //generic thing for switching values or whatever
	float scaling;

	for (int col = 0; col < n; col++) {
		//find the row with the greatest value in the current column (past finished rows)
		max_row = -1;
		max_value = 0;
		for (int i = col; i < n; i++) {
			temp = fabsf(a->mat[i][col]);
			if (temp > max_value) {
				max_row = i;
				max_value = temp;
			}
		}
		if (max_row == -1) matutilerror("MAT_solve_gausselim: solve error 1");
		// This might actually happen even if the system is solvable. See if it's a problem.
		// Fix would be to ensure a row doesn't get chosen earlier if it's the only row with a value in a certain column.

		// swap the row that was just found to the current row
		// TODO: swapping row pointers is likely far faster
		for (int i = 0; i < n; i++) {
			temp = a->mat[col][i];
			a->mat[col][i] = a->mat[max_row][i];
			a->mat[max_row][i] = temp;
		}

		// also swap the b value
		temp = b->vec[col];
		b->vec[col] = b->vec[max_row];
		b->vec[max_row] = temp;

		//scale the current row to make the [col][col] value 1
		scaling = a->mat[col][col];
		a->mat[col][col] = 1;
		for (int i = col + 1; i < n; i++) {
			a->mat[col][i] /= scaling;
		}
		b->vec[col] /= scaling;

		// For all lower rows, subtract enough of the current row (and b) to make the col value 0
		for (int row = col + 1; row < n; row++) {
			scaling = a->mat[row][col] / a->mat[col][col];
			for (int i = col; i < n; i++) {
				a->mat[row][i] -= scaling * a->mat[col][i];
			}
			b->vec[row] -= scaling * b->vec[col];
		}
	}
	// Now in_matrix should be properly triangular, and b should match
	// Time for backsubstitution
	for (int row = n-1; row >= 0; row--) {
		temp = 0;
		for (int j = row + 1; j < n; j++) {
			temp += a->mat[row][j] * x->vec[j];
		}
		x->vec[row] = b->vec[row] - temp; // assuming that the leading coefficient is 1. Otherwise divide by leading coefficient
	}

	MAT_freematrix(a);
	MAT_freevector(b);

	return x;
}