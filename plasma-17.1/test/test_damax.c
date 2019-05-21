/**
 *
 * @file
 *
 *  PLASMA is a software package provided by:
 *  University of Tennessee, US,
 *  University of Manchester, UK.
 *
 * @generated from test/test_dzamax.c, normal z -> d, Tue Apr 16 14:34:40 2019
 *
 **/

#include "test.h"
#include "flops.h"
#include "core_blas.h"
#include "core_lapack.h"
#include "plasma.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <omp.h>

#define REAL

/***************************************************************************//**
 *
 * @brief Tests DAMAX.
 *
 * @param[in]  param - array of parameters
 * @param[out] info  - string of column labels or column values; length InfoLen
 *
 * If param is NULL and info is NULL,     print usage and return.
 * If param is NULL and info is non-NULL, set info to column labels and return.
 * If param is non-NULL and info is non-NULL, set info to column values
 * and run test.
 ******************************************************************************/
void test_damax(param_value_t param[], char *info)
{
    //================================================================
    // Print usage info or return column labels or values.
    //================================================================
    if (param == NULL) {
        if (info == NULL) {
            // Print usage info.
            print_usage(PARAM_COLROW);
            print_usage(PARAM_M);
            print_usage(PARAM_N);
            print_usage(PARAM_PADA);
            print_usage(PARAM_NB);
        }
        else {
            // Return column labels.
            snprintf(info, InfoLen,
                     "%*s %*s %*s %*s %*s",
                     InfoSpacing, "Storev",
                     InfoSpacing, "M",
                     InfoSpacing, "N",
                     InfoSpacing, "PadA",
                     InfoSpacing, "NB");
        }
        return;
    }
    // Return column values.
    snprintf(info, InfoLen,
             "%*c %*d %*d %*d %*d",
             InfoSpacing, param[PARAM_COLROW].c,
             InfoSpacing, param[PARAM_M].i,
             InfoSpacing, param[PARAM_N].i,
             InfoSpacing, param[PARAM_PADA].i,
             InfoSpacing, param[PARAM_NB].i);

    //================================================================
    // Set parameters.
    //================================================================
    plasma_enum_t colrow = plasma_storev_const(param[PARAM_COLROW].c);

    int m = param[PARAM_M].i;
    int n = param[PARAM_N].i;

    int lda = imax(1, m + param[PARAM_PADA].i);

    int test = param[PARAM_TEST].c == 'y';

    //================================================================
    // Set tuning parameters.
    //================================================================
    plasma_set(PlasmaNb, param[PARAM_NB].i);

    //================================================================
    // Allocate and initialize arrays.
    //================================================================
    double *A =
        (double*)malloc((size_t)lda*n*sizeof(double));
    assert(A != NULL);

    int seed[] = {0, 0, 0, 1};
    lapack_int retval;
    retval = LAPACKE_dlarnv(1, seed, (size_t)lda*n, A);
    assert(retval == 0);

    size_t size = colrow == PlasmaColumnwise ? n : m;
    double *values = (double*)malloc(size*sizeof(double));
    assert(values != NULL);

    double *valref = NULL;
    if (test) {
        valref = (double*)malloc(size*sizeof(double));
        assert(valref != NULL);
    }

    //================================================================
    // Run and time PLASMA.
    //================================================================
    plasma_time_t start = omp_get_wtime();
    plasma_damax(colrow, m, n, A, lda, values);
    plasma_time_t stop = omp_get_wtime();
    plasma_time_t time = stop-start;

    param[PARAM_TIME].d = time;
    param[PARAM_GFLOPS].d = flops_dlange(m, n, colrow) / time / 1e9;

    //================================================================
    // Test results by comparing to a reference implementation.
    //================================================================
    if (test) {
        if (colrow == PlasmaColumnwise) {
            for (int j = 0; j < n; j++) {
                CBLAS_INDEX idx = cblas_idamax(m, &A[lda*j], 1);
                valref[j] = fabs(A[lda*j+idx]);
            }
        }
        else {
            for (int i = 0; i < m; i++) {
                CBLAS_INDEX idx = cblas_idamax(n, &A[i], lda);
                valref[i] = fabs(A[i+lda*idx]);
            }
        }

        // Calculate difference.
        cblas_daxpy(size, -1.0, values, 1, valref, 1);

        // Set error to maximum difference.
        double error = valref[cblas_idamax(size, valref, 1)];

        param[PARAM_ERROR].d   = error;
        param[PARAM_SUCCESS].i = error == 0.0;
    }

    //================================================================
    // Free arrays.
    //================================================================
    free(A);
    free(values);
    if (test)
        free(valref);
}
