// Collection of Linear Algebra Utilities
// Amended from CFD/

#include <stdio.h>
#include <stdlib.h>
#include "matutil.h"

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

	float *rows = (float *) malloc(r * sizeof float);
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
			printf("%6.3d  ", m->mat[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void MAT_printvector(vector *v) {
	printf("\n");
	for (int i = 0; i < v->rows; i++) {
		printf("%6.3d\n", v->vec[i]);
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
	matrix *new = MAT_matrix(m->rows, m->cols, 0);
	for (int i = 0; i < m->rows; i++) {
		for (int j = 0; j < m->cols; j++) {
			new->mat[i][j] = m->mat[i][j];
		}
	}
	return new;
}

vector* MAT_copyvector(vector *v) {
	// Initialize a new vector, copy values, and return pointer to new vector
	vector *new = MAT_vector(v->rows, 0);
	for (int i = 0; i < v->rows; i++) {
		new->vec[i] = v->vec[i];
	}
	return new;
}

matrix* MAT_multiply_mm(matrix *ma, matrix *mb);
vector* MAT_multiply_mv(matrix *m, vector *v);

vector* MAT_solve_gausselim(matrix *m, vector *v);