/**
 *
 * @file
 *
 *  PLASMA is a software package provided by:
 *  University of Tennessee, US,
 *  University of Manchester, UK.
 *
 * @generated from compute/pzlascl.c, normal z -> s, Tue Apr 16 14:33:54 2019
 *
 **/

#include "plasma_async.h"
#include "plasma_descriptor.h"
#include "plasma_internal.h"
#include "plasma_types.h"
#include "core_blas.h"

#define A(m, n) (float*)plasma_tile_addr(A, m, n)

/******************************************************************************/
void plasma_pslascl(plasma_enum_t uplo,
                    float cfrom, float cto,
                    plasma_desc_t A,
                    plasma_sequence_t *sequence, plasma_request_t *request)
{
    // Return if failed sequence.
    if (sequence->status != PlasmaSuccess)
        return;

    switch (uplo) {
    //==============
    // PlasmaUpper
    //==============
    case PlasmaUpper:
        for (int m = 0; m < A.mt; m++) {
            int mvam = plasma_tile_mview(A, m);
            int ldam = plasma_tile_mmain(A, m);
            if (m < A.nt) {
                int nvam = plasma_tile_nview(A, m);
                core_omp_slascl(
                    PlasmaUpper,
                    cfrom, cto,
                    mvam, nvam,
                    A(m, m), ldam,
                    sequence, request);
            }
            for (int n = m+1; n < A.nt; n++) {
                int nvan = plasma_tile_nview(A, n);
                core_omp_slascl(
                    PlasmaGeneral,
                    cfrom, cto,
                    mvam, nvan,
                    A(m, n), ldam,
                    sequence, request);
            }
        }
        break;
    //==============
    // PlasmaLower
    //==============
    case PlasmaLower:
        for (int m = 0; m < A.mt; m++) {
            int mvam = plasma_tile_mview(A, m);
            int ldam = plasma_tile_mmain(A, m);
            if (m < A.nt) {
                int nvam = plasma_tile_nview(A, m);
                core_omp_slascl(
                    PlasmaLower,
                    cfrom, cto,
                    mvam, nvam,
                    A(m, m), ldam,
                    sequence, request);
            }
            for (int n = 0; n < imin(m, A.nt); n++) {
                int nvan = plasma_tile_nview(A, n);
                core_omp_slascl(
                    PlasmaGeneral,
                    cfrom, cto,
                    mvam, nvan,
                    A(m, n), ldam,
                    sequence, request);
            }
        }
        break;
    //================
    // PlasmaGeneral
    //================
    case PlasmaGeneral:
    default:
        for (int m = 0; m < A.mt; m++) {
            int mvam  = plasma_tile_mview(A, m);
            int ldam = plasma_tile_mmain(A, m);
            for (int n = 0; n < A.nt; n++) {
                int nvan = plasma_tile_nview(A, n);
                core_omp_slascl(
                    PlasmaGeneral,
                    cfrom, cto,
                    mvam, nvan,
                    A(m, n), ldam,
                    sequence, request);
            }
        }
    }
}
