/*  $Id: matrix.c,v 1.8 2003/04/07 16:27:10 ukai Exp $ */
/* 
 * matrix.h, matrix.c: Liner equation solver using LU decomposition.
 *
 * by K.Okabe  Aug. 1999 
 *
 * LUfactor, LUsolve, Usolve and Lsolve, are based on the functions in
 * Meschach Library Version 1.2b.
 */

/**************************************************************************
**
** Copyright (C) 1993 David E. Steward & Zbigniew Leyk, all rights reserved.
**
**			     Meschach Library
** 
** This Meschach Library is provided "as is" without any express 
** or implied warranty of any kind with respect to this software. 
** In particular the authors shall not be liable for any direct, 
** indirect, special, incidental or consequential damages arising 
** in any way from use of the software.
** 
** Everyone is granted permission to copy, modify and redistribute this
** Meschach Library, provided:
**  1.  All copies contain this copyright notice.
**  2.  All modified copies shall carry a notice stating who
**      made the last modification and the date of such modification.
**  3.  No charge is made for this software or works derived from it.  
**      This clause shall not be construed as constraining other software
**      distributed on the same medium as this software, nor is a
**      distribution fee considered a charge.
**
***************************************************************************/

#include "config.h"
#include "matrix.h"
#include <gc.h>

/* 
 * Macros from "fm.h".
 */

#define New(type)       ((type*)GC_MALLOC(sizeof(type)))
#define NewAtom(type)   ((type*)GC_MALLOC_ATOMIC(sizeof(type)))
#define New_N(type,n)   ((type*)GC_MALLOC((n)*sizeof(type)))
#define NewAtom_N(type,n)       ((type*)GC_MALLOC_ATOMIC((n)*sizeof(type)))
#define Renew_N(type,ptr,n)   ((type*)GC_REALLOC((ptr),(n)*sizeof(type)))

#define SWAPD(a,b) { double tmp = a; a = b; b = tmp; }
#define SWAPI(a,b) { int tmp = a; a = b; b = tmp; }

#ifdef HAVE_FLOAT_H
#include <float.h>
#endif				/* not HAVE_FLOAT_H */
#if defined(DBL_MAX)
static double Tiny = 10.0 / DBL_MAX;
#elif defined(FLT_MAX)
static double Tiny = 10.0 / FLT_MAX;
#else				/* not defined(FLT_MAX) */
static double Tiny = 1.0e-30;
#endif				/* not defined(FLT_MAX */

/* 
 * LUfactor -- gaussian elimination with scaled partial pivoting
 *          -- Note: returns LU matrix which is A.
 */

int
LUfactor(Matrix A, int *indexarray)
{
    int dim = A->dim, i, j, k, i_max, k_max;
    Vector scale;
    double mx, tmp;

    scale = new_vector(dim);

    for (i = 0; i < dim; i++)
	indexarray[i] = i;

    for (i = 0; i < dim; i++) {
	mx = 0.;
	for (j = 0; j < dim; j++) {
	    tmp = fabs(M_VAL(A, i, j));
	    if (mx < tmp)
		mx = tmp;
	}
	scale->ve[i] = mx;
    }

    k_max = dim - 1;
    for (k = 0; k < k_max; k++) {
	mx = 0.;
	i_max = -1;
	for (i = k; i < dim; i++) {
	    if (fabs(scale->ve[i]) >= Tiny * fabs(M_VAL(A, i, k))) {
		tmp = fabs(M_VAL(A, i, k)) / scale->ve[i];
		if (mx < tmp) {
		    mx = tmp;
		    i_max = i;
		}
	    }
	}
	if (i_max == -1) {
	    M_VAL(A, k, k) = 0.;
	    continue;
	}

	if (i_max != k) {
	    SWAPI(indexarray[i_max], indexarray[k]);
	    for (j = 0; j < dim; j++)
		SWAPD(M_VAL(A, i_max, j), M_VAL(A, k, j));
	}

	for (i = k + 1; i < dim; i++) {
	    tmp = M_VAL(A, i, k) = M_VAL(A, i, k) / M_VAL(A, k, k);
	    for (j = k + 1; j < dim; j++)
		M_VAL(A, i, j) -= tmp * M_VAL(A, k, j);
	}
    }
    return 0;
}

/* 
 * LUsolve -- given an LU factorisation in A, solve Ax=b.
 */

int
LUsolve(Matrix A, int *indexarray, Vector b, Vector x)
{
    int i, dim = A->dim;

    for (i = 0; i < dim; i++)
	x->ve[i] = b->ve[indexarray[i]];

    if (Lsolve(A, x, x, 1.) == -1 || Usolve(A, x, x, 0.) == -1)
	return -1;
    return 0;
}

/* m_inverse -- returns inverse of A, provided A is not too rank deficient
 *           -- uses LU factorisation */
#if 0
Matrix
m_inverse(Matrix A, Matrix out)
{
    int *indexarray = NewAtom_N(int, A->dim);
    Matrix A1 = new_matrix(A->dim);
    m_copy(A, A1);
    LUfactor(A1, indexarray);
    return LUinverse(A1, indexarray, out);
}
#endif				/* 0 */

Matrix
LUinverse(Matrix A, int *indexarray, Matrix out)
{
    int i, j, dim = A->dim;
    Vector tmp, tmp2;

    if (!out)
	out = new_matrix(dim);
    tmp = new_vector(dim);
    tmp2 = new_vector(dim);
    for (i = 0; i < dim; i++) {
	for (j = 0; j < dim; j++)
	    tmp->ve[j] = 0.;
	tmp->ve[i] = 1.;
	if (LUsolve(A, indexarray, tmp, tmp2) == -1)
	    return NULL;
	for (j = 0; j < dim; j++)
	    M_VAL(out, j, i) = tmp2->ve[j];
    }
    return out;
}

/* 
 * Usolve -- back substitution with optional over-riding diagonal
 *        -- can be in-situ but doesn't need to be.
 */

int
Usolve(Matrix mat, Vector b, Vector out, double diag)
{
    int i, j, i_lim, dim = mat->dim;
    double sum;

    for (i = dim - 1; i >= 0; i--) {
	if (b->ve[i] != 0.)
	    break;
	else
	    out->ve[i] = 0.;
    }
    i_lim = i;

    for (; i >= 0; i--) {
	sum = b->ve[i];
	for (j = i + 1; j <= i_lim; j++)
	    sum -= M_VAL(mat, i, j) * out->ve[j];
	if (diag == 0.) {
	    if (fabs(M_VAL(mat, i, i)) <= Tiny * fabs(sum))
		return -1;
	    else
		out->ve[i] = sum / M_VAL(mat, i, i);
	}
	else
	    out->ve[i] = sum / diag;
    }

    return 0;
}

/* 
 * Lsolve -- forward elimination with (optional) default diagonal value.
 */

int
Lsolve(Matrix mat, Vector b, Vector out, double diag)
{
    int i, j, i_lim, dim = mat->dim;
    double sum;

    for (i = 0; i < dim; i++) {
	if (b->ve[i] != 0.)
	    break;
	else
	    out->ve[i] = 0.;
    }
    i_lim = i;

    for (; i < dim; i++) {
	sum = b->ve[i];
	for (j = i_lim; j < i; j++)
	    sum -= M_VAL(mat, i, j) * out->ve[j];
	if (diag == 0.) {
	    if (fabs(M_VAL(mat, i, i)) <= Tiny * fabs(sum))
		return -1;
	    else
		out->ve[i] = sum / M_VAL(mat, i, i);
	}
	else
	    out->ve[i] = sum / diag;
    }

    return 0;
}

/* 
 * new_matrix -- generate a nxn matrix.
 */

Matrix
new_matrix(int n)
{
    Matrix mat;

    mat = New(struct matrix);
    mat->dim = n;
    mat->me = NewAtom_N(double, n * n);
    return mat;
}

/* 
 * new_matrix -- generate a n-dimension vector.
 */

Vector
new_vector(int n)
{
    Vector vec;

    vec = New(struct vector);
    vec->dim = n;
    vec->ve = NewAtom_N(double, n);
    return vec;
}
