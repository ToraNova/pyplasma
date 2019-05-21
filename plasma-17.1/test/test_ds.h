/**
 *
 * @file
 *
 *  PLASMA is a software package provided by:
 *  University of Tennessee, US,
 *  University of Manchester, UK.
 *
 * @generated from test/test_zc.h, mixed zc -> ds, Tue Apr 16 14:33:18 2019
 *
 **/
#ifndef TEST_DS_H
#define TEST_DS_H

#include "test.h"

//==============================================================================
// test routines
//==============================================================================
void test_dsgesv(param_value_t param[], char *info);
void test_dsposv(param_value_t param[], char *info);
void test_dlag2s(param_value_t param[], char *info);
void test_slag2d(param_value_t param[], char *info);

#endif // TEST_DS_H
