/*
 * ridge.h are functions that enables ridge regression
 * whether centralized or distributed.
 */

#ifndef RIDGE_H
#define RIDGE_H

#include <stdint.h>
#include <stddef.h>

/*
 * library test function. to check if everything is OK
 */
void ridge_test();

/*
 * This is to test the ARGOUT capability of swig numpy
 */
void ridge_range( double *alpha, int asize );

/*
 * perform Ridge Regression on the input Dmat
 * and Tvct. they're both 1D representation
 * of the double array, thus, please specify also
 * their dimensions. To perform a partial solve (i.e assumes that
 * Dmat is already the kernel, set param kernel to > 0
 * @params Dmat - input Data matrix to perform ridge regression on
 * @params Tvct - input Target vector to train on
 * @dsize and tsize are numpy intrinsics, just pass the np array it will absorb 2 params
 * @params xrowsz, xcolsz - the rowsize, colsize of Dmat
 * @params yelmsz, ydimsz - the rowsize, colsize of Tvct
 * @params lambda - the hyperparameter for ridge regression
 * @params kernel - if > 0, then assumes Dmat is the kernel and do not perform rank update
 * @params alpha, int asize is actually a numpy Argout array
 *
 * note, when using this in C, dsize, tsize and asize can be replaced with 0
 * when using this in Python, Dmat, dsize is actually a numpy array
 * similarly for Tvct, tsize and alpha, asize
 * this function is now stable. please refrain from editing 10th May 19 - ToraNova
 */
void ridge_solve(
	double *Dmat, int dsize, size_t xrowsz, size_t xcolsz,
	double *Tvct, int tsize, size_t yelmsz, size_t ydimsz,
	double lambda, int kernel,
	double *alpha, int asize
);

#endif
