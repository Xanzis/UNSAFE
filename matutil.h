#ifndef _MATRIXUTIL_
#define _MATRIXUTIL_

// Collection of Linear Algebra Utilities
// Amended from CFD/

// Working with floats until further notice
// All vectors and matrices are zero-indexed.
// Vectors are column vectors until further notice
// Matrices are indexed: mat[row][col]

typedef struct matrix matrix;
struct matrix {
	int rows;
	int cols;
	float **mat;
};

typedef struct vector vector;
struct vector {
	int rows;
	float *vec;
};

void matutilerror(char *error_text);

matrix* MAT_matrix(int r, int c, int init_zeros);
vector* MAT_vector(int r, int init_zeros);

void MAT_freematrix(matrix *m);
void MAT_freevector(vector *v);

void MAT_printmatrix(matrix *m);
void MAT_printvector(vector *v);

void MAT_zeromatrix(matrix *m);
void MAT_zerovector(vector *v);

void MAT_addmatrix(matrix *ma, matrix *mb);
void MAT_addvector(vector *va, vector *vb);
void MAT_multiply_ms(matrix *m, float s);
void MAT_multiply_vs(vector *v, float s);
matrix* MAT_copymatrix(matrix *m);
vector* MAT_copyvector(vector *v);
matrix* MAT_multiply_mm(matrix *ma, matrix *mb);
vector* MAT_multiply_mv(matrix *m, vector *v);

vector* MAT_solve_gausselim(matrix *m, vector *v);

#endif