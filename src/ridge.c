/*
 * PyPLASMA utility functions
 * For utility such as file loading/reading
 * and other auxiliaries
 */

/*
 * include the irrelevant header
 */
#include "../include/ridge.h"
#include "../include/pputil.h"
#include "../include/irr.h"

/*
 * standard libraries
 */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <omp.h>

/*
 *	PRODTOOLS includes
 */
#include <prodtools/arrayutil/doubles.h>
#include <prodtools/math/linear.h>
#include <prodtools/support/timing.h>
#include <prodtools/support/suprint.h>

/*
 *	PLASMA CORE BLAS includes
 */
//#include "core_lapack.h"
#include "core_blas.h"
#include "plasma.h"

#define REAL

/*
 * PLASMA library docs from doxygen
 * http://icl.bitbucket.io/plasma/
 */

/*
 * library test function. to check if everything is OK
 */
void ridge_test(){
	printf("ridge libtest OK\n");
	//test if arrayutil is actually working
	arrayutil_doubles_libtest();
	linear_libtest();
	return;
}

/*
 * This is to test the ARGOUT capability of swig numpy
 * SAMPLE USE : in python
 * k = ridge_range(10)
 * print(k) -> [0,1,2,3,...9]
 */
void ridge_range( double *alpha, int asize ){
	int i;
	for(i=0;i<asize;i++){
		alpha[i] = i;
	}
}
	

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
 * **** If please supply Kernel + I
 * @params alpha, int asize is actually a numpy Argout array
 *
 * note, when using this in C, dsize, tsize and asize can be replaced with 0
 * when using this in Python, Dmat, dsize is actually a numpy array
 * similarly for Tvct, tsize and alpha, asize
 * this function is now stable. please refrain from editing 10th May 19 - ToraNova
 *
 * ALTHOUGH this function returns void. in python the numpy typemap
 * causes it to return alpha instead. thus, sample call include
 * alpha = ridge_solve( args.. , size of alpha )
 * refer to ridge_range for a detailed example
 */
void ridge_solve(
	double *Dmat, int dsize, size_t xrowsz, size_t xcolsz,
	double *Tvct, int tsize, size_t yelmsz, size_t ydimsz,
	double lambda, int kernel,
	double *alpha, int asize
){
	//var declares
	double *symmetricL; //intermediaries
	int rc; //used to store result codes

	//current assumption, ydimsz == 1
	if(ydimsz > 1)log_err("Warning: untested code for target dim > 1");
	debug("PyNP ndsize D/T/A %d %d %d",dsize,tsize,asize);
	
	//obtain unity matrix
	symmetricL = linear_obtainUnityST( yelmsz );
	debug("Unity obtained (size:%ld)",yelmsz);

#ifdef EDEBUG
	//declares the weight array (for debugging only)
	double *weight;
	weight = (double *)calloc( xcolsz* ydimsz, sizeof(double));
	debug("weight array allocated (%ld/%ld)",ydimsz,xcolsz);
	//timing mechanism
	timing_SimpleWallTimer *stimer = constr_SimpleWallTimer();
	SimpleWallTimer_begin(stimer);
#endif

	//begin plasma
	plasma_init();
	debug("Starting ridge_solve kernel(%d) with lambda %1.3f",kernel,lambda);

	if(kernel <= 0){
		debug("Non Kernel Mode");

		//this performs XtX on row major data
		rc = plasma_dsyrk(
			PlasmaLower, PlasmaTrans,
			yelmsz, xcolsz,
			1.0, Dmat, xcolsz,
			lambda, symmetricL, yelmsz);
		debug("plasma_dsyrk routine complete. %d",rc);

		//perform cholesky's decomposition
		rc = plasma_dpotrf(PlasmaLower, yelmsz, symmetricL, yelmsz);
		debug("plasma_dpotrf routine complete. %d",rc);

		//now perform the inversion
		rc = plasma_dpotri(PlasmaLower, yelmsz, symmetricL, yelmsz);
		debug("plasma_dpotri routine complete. %d",rc);

		/*
		 * ASSERTION - inversion is OK
		 */
		
		//recover the alpha to prepare for redistribution
		rc = plasma_dsymm(
			PlasmaLeft, PlasmaLower,
			xrowsz, ydimsz, lambda,
			symmetricL, yelmsz,
			Tvct, xrowsz,
			0.0, alpha, xrowsz);

		debug("plasma_dsymm routine complete. %d",rc);
		//these steps are not actually required in actual practice
		//only used for debugging
#ifdef EDEBUG
		//prints the alpha out
		//not really necessary since alphas can be very large
		//suprint_doublearrST(alpha, ydimsz, xrowsz, 14, 5);

		//this step is not actually required in actual practice
		//only used for debuggin
		rc = plasma_dgemm(
			PlasmaNoTrans, PlasmaNoTrans,
			xcolsz, ydimsz, xrowsz,
			(1.0/lambda), Dmat, xcolsz,
			     alpha, xrowsz,
			0.0, weight, xcolsz);
		debug("plasma_dgemm routine complete. %d",rc);
		debug("Test weights obtained.");
		//prints the weights out
		suprint_doublearrST(weight, ydimsz, xcolsz, 14, 5);
		free(weight);
#endif

	}else{
		debug("Kernel Mode");
		rc = plasma_dgemm(
			PlasmaNoTrans, PlasmaNoTrans,
			yelmsz, yelmsz, yelmsz,
			1.0, Dmat, yelmsz,
				symmetricL, yelmsz,
			lambda, symmetricL, yelmsz);
		debug("plasma_dgemm routine complete. %d",rc);
			
		//perform cholesky's decomposition, except on Dmat == symmetricL
		rc = plasma_dpotrf(PlasmaLower, yelmsz, symmetricL, yelmsz);
		debug("plasma_dpotrf routine complete. %d",rc);

		//now perform the inversion, except on Dmat == symmetricL
		rc = plasma_dpotri(PlasmaLower, yelmsz, symmetricL, yelmsz);
		debug("plasma_dpotri routine complete. %d",rc);

		//recover the alpha to prepare for redistribution, except on Dmat
		// Dmat == symmetricL
		rc = plasma_dsymm(
			PlasmaLeft, PlasmaLower,
			xrowsz, ydimsz, lambda,
			symmetricL, yelmsz,
			Tvct, xrowsz,
			0.0, alpha, xrowsz);

		debug("plasma_dsymm routine complete. %d",rc);

		//there is no way to compute W since Dmat is the kernel. the true Dmat
		//is not available
	}

#ifdef EDEBUG
	//timer and array cleanup
	SimpleWallTimer_end(stimer);
	SimpleWallTimer_print(stimer);
	destr_SimpleWallTimer(stimer);
#endif

	plasma_finalize();
	free(symmetricL);
}


	
