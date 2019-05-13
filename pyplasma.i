/* This is the interface file */
/* For our wrapper test*/

%module pyplasma 
%{      
        #define SWIG_FILE_WITH_INIT
        #include "include/irr.h"
        #include "include/pputil.h"
        #include "include/ridge.h"
%}

/* explicit list of functions to be interfaced */
/*void wrapper_test(int arg1);*/

/* uses numpy.i typemap*/
%include "numpy.i"

%init
%{
        import_array();
%}

/* applying numpy typemaps */
%apply (double *IN_ARRAY1, int DIM1) {(double *Dmat, int dsize)} 
%apply (double *IN_ARRAY1, int DIM1) {(double *Tvct, int tsize)} 
%apply (double *ARGOUT_ARRAY1, int DIM1) {(double *alpha, int asize)}

/* or just */
%include "include/irr.h"
%include "include/pputil.h"
%include "include/ridge.h"
