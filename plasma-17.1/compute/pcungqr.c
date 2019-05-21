/**
 *
 * @file
 *
 *  PLASMA is a software package provided by:
 *  University of Tennessee, US,
 *  University of Manchester, UK.
 *
 * @generated from compute/pzungqr.c, normal z -> c, Tue Apr 16 14:34:12 2019
 *
 **/

#include "plasma_async.h"
#include "plasma_context.h"
#include "plasma_descriptor.h"
#include "plasma_internal.h"
#include "plasma_types.h"
#include "plasma_workspace.h"
#include "core_blas.h"

#define A(m, n) (plasma_complex32_t*)plasma_tile_addr(A, m, n)
#define T(m, n) (plasma_complex32_t*)plasma_tile_addr(T, m, n)
#define Q(m, n) (plasma_complex32_t*)plasma_tile_addr(Q, m, n)

/***************************************************************************//**
 *  Parallel construction of Q using tile V (application to identity)
 **/
void plasma_pcungqr(plasma_desc_t A, plasma_desc_t T, plasma_desc_t Q,
                    plasma_workspace_t work,
                    plasma_sequence_t *sequence, plasma_request_t *request)
{
    // Return if failed sequence.
    if (sequence->status != PlasmaSuccess)
        return;

    // Set inner blocking from the T tile row-dimension.
    int ib = T.mb;

    for (int k = imin(A.mt, A.nt)-1; k >= 0; k--) {
        int mvak  = plasma_tile_mview(A, k);
        int nvak  = plasma_tile_nview(A, k);
        int mvqk  = plasma_tile_mview(Q, k);
        int ldak  = plasma_tile_mmain(A, k);
        int ldqk  = plasma_tile_mmain(Q, k);
        for (int m = Q.mt - 1; m > k; m--) {
            int mvqm = plasma_tile_mview(Q, m);
            int ldam = plasma_tile_mmain(A, m);
            int ldqm = plasma_tile_mmain(Q, m);
            for (int n = k; n < Q.nt; n++) {
                int nvqn = plasma_tile_nview(Q, n);
                core_omp_ctsmqr(
                    PlasmaLeft, PlasmaNoTrans,
                    Q.mb, nvqn, mvqm, nvqn, nvak, ib,
                    Q(k, n), ldqk,
                    Q(m, n), ldqm,
                    A(m, k), ldam,
                    T(m, k), T.mb,
                    work,
                    sequence, request);
            }
        }
        for (int n = k; n < Q.nt; n++) {
            int nvqn = plasma_tile_nview(Q, n);
            core_omp_cunmqr(
                PlasmaLeft, PlasmaNoTrans,
                mvqk, nvqn, imin(nvak, mvak), ib,
                A(k, k), ldak,
                T(k, k), T.mb,
                Q(k, n), ldqk,
                work,
                sequence, request);
        }
    }
}