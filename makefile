################################################################################
# Makefile to build the wrapping objects
# PyPlasma Makefile
# ToraNova 2019
# chia_jason96@live.com
# May 8th 2019
################################################################################

# project name
PRONAME  := pyplasma
# TODO: please create pyplasma.i swig interface file to
# allow this project to compile

# Directory specifications
SRCDIR   := src
SOURCES  := $(wildcard $(SRCDIR)/*.c)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=%.o) 

# specify the library paths (the root dir of the lib projects)
PRTROOT  ?= /home/cjason/library/prodtools
MKLROOT  ?= /opt/intel/mkl
PLSROOT  ?= /home/cjason/github/plasma-17.1

# rmb to export this in your env to run the program
LD_LIBRARY_PATH ?= $(MKLROOT)/lib/intel64:$(PRTROOT)/lib

################################################################################
# Compilation Flags and include/library specification
################################################################################
CC       = gcc
CFLAGS   = -std=c99 -O3  
LDFLAGS  = -fopenmp -fPIC

# Target specific vars
debug: 	CFLAGS += -DEDEBUG -Wall -Wno-unused-function -Wno-unsued-variable 
re_ctest: CFLAGS += -DEDEBUG -Wall -Wno-unused-function -Wno-unsued-variable 

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

# Default build
# to suit deployment use cases
.phony: all
all: sharedlib 

# Working target
# manual target, builds manually
.phony: sharedlib 
sharedlib: objs
	$(CC) $(LDFLAGS) -shared $(OBJECTS) $(PRONAME)_wrap.o -o _$(PRONAME).so $(LIBS)

# wrapper target, builds the python wrapped version
# Warn: not used at the moment
.phony: wrapper
wrapper:
	python3 src/setup.py build_ext --inplace

# debugger for the makefile
.phony: debug
debug: objs
	@echo "Echoing make vars"
	@echo $(SRCDIR)
	@echo $(SOURCES)
	@echo $(OBJECTS)
	$(CC) $(LDFLAGS) -shared $(OBJECTS) $(PRONAME)_wrap.o -o _$(PRONAME).so $(LIBS)

# Cleanup
.phony: clean
clean:
	rm -rf __pycache__
	rm -f ./*.o ./*.so ./*.pyc 
	rm -f ./*.c ./*.cxx ./$(PRONAME).py
	rm -f ctest

################################################################################
# Build targets
################################################################################

objs: iswig
	$(CC) -c -fpic $(SOURCES) $(CFLAGS) $(PRONAME)_wrap.c -I/usr/include/python3.6 $(INC)

iswig:
	swig -python $(PRONAME).i

################################################################################
# C Test routines
# doesn't require any swig bullshit. just pure C to see if it really works
################################################################################
ctest:
	@echo "Compiling Tests in C"
	$(CC) $(LDFLAGS) -o $@ $(SOURCES) $(CFLAGS) $(LIBS) $(INC) 

# retest
.phony: re_ctest
re_ctest: clean ctest


