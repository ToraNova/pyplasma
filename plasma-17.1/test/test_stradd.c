/**
 *
 * @file
 *
 *  PLASMA is a software package provided by:
 *  University of Tennessee, US,
 *  University of Manchester, UK.
 *
 * @generated from test/test_ztradd.c, normal z -> s, Tue Apr 16 14:34:50 2019
 *
 **/

#include "test.h"
#include "flops.h"
#include "core_blas.h"
#include "core_lapack.h"
#include "plasma.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <omp.h>

#define REAL

/***************************************************************************//**
 *
 * @brief Tests STRADD
 *
 * @param[in]  param - array of parameters
 * @param[out] info  - string of column labels or column values; length InfoLen
 *
 * If param is NULL and info is NULL,     print usage and return.
 * If param is NULL and info is non-NULL, set info to column labels and return.
 * If param is non-NULL and info is non-NULL, set info to column values
 * and run test.
 ******************************************************************************/
void test_stradd(param_value_t param[], char *info)
{
    //================================================================
    // Print usage info or return column labels or values
    //================================================================
    if (param == NULL) {
        if (info == NULL) {
            // Print usage info
            print_usage(PARAM_UPLO);
            print_usage(PARAM_TRANSA);
            print_usage(PARAM_M);
            print_usage(PARAM_N);
            print_usage(PARAM_ALPHA);
            print_usage(PARAM_BETA);
            print_usage(PARAM_PADA);
            print_usage(PARAM_PADB);
            print_usage(PARAM_NB);
        }
        else {
            // Return column labels
            snprintf(info, InfoLen,
                     "%*s %*s %*s %*s %*s %*s %*s %*s %*s",
                     InfoSpacing, "UpLo",
                     InfoSpacing, "TransA",
                     InfoSpacing, "m",
                     InfoSpacing, "n",
                     InfoSpacing, "alpha",
                     InfoSpacing, "beta",
                     InfoSpacing, "PadA",
                     InfoSpacing, "PadB",
                     InfoSpacing, "nb");
        }
        return;
    }
    // Return column values
    snprintf(info, InfoLen,
             "%*c %*c %*d %*d %*.4f %*.4f %*d %*d %*d",
             InfoSpacing, param[PARAM_UPLO].c,
             InfoSpacing, param[PARAM_TRANSA].c,
             InfoSpacing, param[PARAM_M].i,
             InfoSpacing, param[PARAM_N].i,
             InfoSpacing, creal(param[PARAM_ALPHA].z),
             InfoSpacing, creal(param[PARAM_BETA].z),
             InfoSpacing, param[PARAM_PADA].i,
             InfoSpacing, param[PARAM_PADB].i,
             InfoSpacing, param[PARAM_NB].i);

    //================================================================
    // Set parameters
    //================================================================
    plasma_enum_t uplo   = plasma_uplo_const(param[PARAM_UPLO].c);
    plasma_enum_t transa = plasma_trans_const(param[PARAM_TRANSA].c);

    int m = param[PARAM_M].i;
    int n = param[PARAM_N].i;

    int Am, An;
    int Bm, Bn;

    if (transa == PlasmaNoTrans) {
        Am = m;
        An = n;
    }
    else {
        Am = n;
        An = m;
    }

    Bm = m;
    Bn = n;

    int lda = imax(1, Am + param[PARAM_PADA].i);
    int ldb = imax(1, Bm + param[PARAM_PADB].i);

    int    test = param[PARAM_TEST].c == 'y';
    float eps  = LAPACKE_slamch('E');

    //================================================================
    // Set tuning parameters
    //================================================================
    plasma_set(PlasmaNb, param[PARAM_NB].i);

    //================================================================
    // Allocate and initialize arrays
    //================================================================
    float *A =
        (float*)malloc((size_t)lda*An*sizeof(float));
    assert(A != NULL);

    float *B =
        (float*)malloc((size_t)ldb*Bn*sizeof(float));
    assert(B != NULL);

    int seed[] = {0, 0, 0, 1};
    lapack_int retval;
    retval = LAPACKE_slarnv(1, seed, (size_t)lda*An, A);
    assert(retval == 0);

    retval = LAPACKE_slarnv(1, seed, (size_t)ldb*Bn, B);
    assert(retval == 0);

    float *Bref = NULL;
    if (test) {
        Bref = (float*)malloc(
            (size_t)ldb*Bn*sizeof(float));
        assert(Bref != NULL);

        memcpy(Bref, B, (size_t)ldb*Bn*sizeof(float));
    }

#ifdef COMPLEX
    float alpha = param[PARAM_ALPHA].z;
    float beta  = param[PARAM_BETA].z;
#else
    float alpha = creal(param[PARAM_ALPHA].z);
    float beta  = creal(param[PARAM_BETA].z);
#endif

    //================================================================
    // Run and time PLASMA
    //================================================================
    plasma_time_t start = omp_get_wtime();

    retval = plasma_stradd(uplo, transa, m, n, alpha, A, lda, beta, B, ldb);

    plasma_time_t stop = omp_get_wtime();

    if (retval != PlasmaSuccess) {
        plasma_error("plasma_stradd() failed");
        param[PARAM_TIME].d    = 0.0;
        param[PARAM_GFLOPS].d  = 0.0;
        param[PARAM_ERROR].d   = 1.0;
        param[PARAM_SUCCESS].i = false;
        return;
    }
    else {
        plasma_time_t time    = stop-start;
        param[PARAM_TIME].d   = time;
        param[PARAM_GFLOPS].d = flops_sgeadd(m, n) / time / 1e9;
    }

    //================================================================
    // Test results by comparing to result of core_stradd function
    //================================================================
    if (test) {
        // Calculate relative error |B_ref - B|_F / |B_ref|_F < 3*eps
        // Using 3*eps covers complex arithmetic

        //=============
        // PlasmaLower
        //=============
        if (uplo == PlasmaLower) {
            switch (transa) {
            //=================
            // PlasmaConjTrans
            //=================
            case PlasmaConjTrans:
                for (int j = 0; j < n; j++) {
                    for (int i = j; i < m; i++) {
                        Bref[ldb*j+i] =
                            beta * Bref[ldb*j+i] + alpha * (A[lda*i+j]);
                    }
                }
                break;
            //=================
            // PlasmaTrans
            //=================
            case PlasmaTrans:
                for (int j = 0; j < n; j++) {
                    for (int i = j; i < m; i++) {
                        Bref[ldb*j+i] =
                            beta * Bref[ldb*j+i] + alpha * A[lda*i+j];
                    }
                }
                break;
            //=================
            // PlasmaNoTrans
            //=================
            case PlasmaNoTrans:
                for (int j = 0; j < n; j++) {
                    for (int i = j; i < m; i++) {
                        Bref[ldb*j+i] =
                            beta * Bref[ldb*j+i] + alpha * A[lda*j+i];
                    }
                }
            }
        }
        //=============
        // PlasmaUpper
        //=============
        else if (uplo == PlasmaUpper) {
            switch (transa) {
            //=================
            // PlasmaConjTrans
            //=================
            case PlasmaConjTrans:
                for (int j = 0; j < n; j++) {
                    for (int i = 0; i < imin(j+1, m); i++) {
                        Bref[ldb*j+i] =
                            beta * Bref[ldb*j+i] + alpha * (A[lda*i+j]);
                    }
                }
                break;
            //=================
            // PlasmaTrans
            //=================
            case PlasmaTrans:
                for (int j = 0; j < n; j++) {
                    for (int i = 0; i < imin(j+1, m); i++) {
                        Bref[ldb*j+i] =
                            beta * Bref[ldb*j+i] + alpha * A[lda*i+j];
                    }
                }
                break;
            //=================
            // PlasmaNoTrans
            //=================
            case PlasmaNoTrans:
                for (int j = 0; j < n; j++) {
                    for (int i = 0; i < imin(j+1, m); i++) {
                        Bref[ldb*j+i] =
                            beta * Bref[ldb*j+i] + alpha * A[lda*j+i];
                    }
                }
            }
        }

        float work[1];

        // Calculate Frobenius norm of reference result B_ref
        float BnormRef  = LAPACKE_slange_work(
                               LAPACK_COL_MAJOR, 'F', Bm, Bn, Bref, ldb, work);

        // Calculate difference B_ref-B
        float zmone = -1.0;
        cblas_saxpy((size_t)ldb*Bn, (zmone), B, 1, Bref, 1);

        // Calculate Frobenius norm of B_ref-B
        float BnormDiff = LAPACKE_slange_work(
                               LAPACK_COL_MAJOR, 'F', Bm, Bn, Bref, ldb, work);

        // Calculate relative error |B_ref-B|_F / |B_ref|_F
        float error = BnormDiff/BnormRef;

        param[PARAM_ERROR].d   = error;
        param[PARAM_SUCCESS].i = error < 3*eps;
    }

    //================================================================
    // Free arrays
    //================================================================
    free(A);
    free(B);

    if (test)
        free(Bref);
}
