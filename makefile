################################################################################
# Makefile to build the wrapping objects
# PyPlasma Makefile
# ToraNova 2019
# chia_jason96@live.com
# May 8th 2019
################################################################################

# project name
PRONAME  := pyplasma
# Using plasma lib version
PLIBV := plasma-17.1

# TODO:
# configure python version, either 3.4, 3,5 or 3.6 (please use 3.6 if possible)
PYTHV ?= python3.6

# Directory specifications
SRCDIR   := src
SOURCES  := $(wildcard $(SRCDIR)/*.c)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=%.o) 

# specify the library paths (the root dir of the lib projects)
PRTROOT  ?= /home/cjason/library/prodtools
MKLROOT  ?= /opt/intel/mkl
PLSROOT  ?= $(PWD)/$(PLIBV)

# rmb to export this in your env to run the program
LD_LIBRARY_PATH ?= $(MKLROOT)/lib/intel64:$(PRTROOT)/lib

################################################################################
# Compilation Flags and include/library specification
################################################################################
CC       = gcc
CFLAGS   = -std=c99 -O3 -DPLASMA_WITH_MKL -DMKL_Complex16="double _Complex" -DMKL_Complex8="float _Complex" 
LDFLAGS  = -fopenmp -fPIC

# Target specific vars
debug: 	CFLAGS  += -DEDEBUG -Wall -Wno-unused-function -Wno-unsued-variable 
ctest:  LDFLAGS += -DEDEBUG -Wall -Wno-unused-function -Wno-unsued-variable 

################################################################################
# Library definitions
# In order of lib dir specification and linkages
# PLASMA/Coreblas
# Intel Math Kernel Library
# Prodtools
# Standards
# THE ORDER IS VERY IMPORTANT. DO NOT MESS IT UP
# Error for unable to load avx2.so and def.so fix
# https://software.intel.com/en-us/forums/intel-distribution-for-python/topic/637997
################################################################################
LIBS      := \
	-L$(PLSROOT)/lib \
	-lplasma -lcoreblas \
	-L$(MKLROOT)/lib/intel64 \
	-lmkl_intel_lp64 -lmkl_core -lmkl_sequential -lmkl_rt -lmkl_def -lm \
	-L$(PRTROOT)/lib \
	-lprodtools_shared \
	-lstdc++ 

################################################################################
# Include declarations
# In order of lib dir specification
# Intel Math Kernel Library
# PLASMA/Coreblas
# Prodtools
# Standards
################################################################################
INC      := \
	-I$(MKLROOT)/include \
	-I$(PLSROOT)/include \
	-I$(PRTROOT)/include \

################################################################################
# Phony targets
################################################################################
.phony: all sharedlib wrapper debug clean distclean runtest iclplasma

# Default build
# to suit deployment use cases
all: sharedlib 

# Working target
# manual target, builds manually
sharedlib: $(OBJECTS) $(PRONAME)_wrap.o iclplasma
	$(CC) $(LDFLAGS) -shared $(OBJECTS) $(PRONAME)_wrap.o -o _$(PRONAME).so $(LIBS)

# wrapper target, builds the python wrapped version
# Warn: not used at the moment
wrapper:
	python3 src/setup.py build_ext --inplace

# debugger for the makefile
debug: $(OBJECTS) $(PRONAME)_wrap.o
	@echo "Echoing make vars"
	@echo $(SRCDIR)
	@echo $(SOURCES)
	@echo $(OBJECTS)
	$(CC) $(LDFLAGS) -shared $(OBJECTS) $(PRONAME)_wrap.o -o _$(PRONAME).so $(LIBS)

# Cleanup
clean:
	rm -rf __pycache__
	rm -f ./*.o ./*.so ./*.pyc 
	rm -f ./*.c ./*.cxx ./$(PRONAME).py
	rm -f ctest

distclean: clean
	cd $(PLSROOT) && $(MAKE) distclean

runtest: 
	rm -f ctest
	$(MAKE) ctest
	./ctest
	python3 test/p_run.py

iclplasma :
	cd $(PLSROOT) && $(MAKE)

################################################################################
# Build targets
################################################################################

$(OBJECTS): %.o : $(SRCDIR)/%.c
	$(CC) -c $(LDFLAGS) $(CFLAGS) -o $@ $(INC) $<

$(PRONAME)_wrap.o: iswig
	$(CC) -c $(LDFLAGS) $(CFLAGS) -o $@ -I/usr/include/$(PYTHV) $(INC) $(PRONAME)_wrap.c

iswig:
	swig -python $(PRONAME).i

################################################################################
# C Test routines
# doesn't require any swig bullshit. just pure C to see if it really works
################################################################################
ctest: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) test/c_run.c -o $@ $(LIBS) $(INC) 

