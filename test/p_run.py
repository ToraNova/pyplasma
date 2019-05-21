#!/usr/bin/python3

import sys
sys.path.append('..')

import pyplasma.pyplasma as pyplasma

if __name__ == "__main__":

    print(pyplasma)
    pyplasma.test()
    pyplasma.argtest(0)
    pyplasma.pputil_test()
    pyplasma.ridge_test()

