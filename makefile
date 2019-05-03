# Makefile to build the wrapping objects

.phony: all
all:
	python3 src/setup.py build_ext --inplace

# manual method as follow
# swig -python $(idir)/pyplasma.i
# gcc -c -fpic irr.c pyplasma_wrap.c -I/usr/include/python3.6
# gcc -shared irr.o pyplasma_wrap.o -o _pyplasma.so

.phony: clean
clean:
	rm -rf build
	rm -rf __pycache__
	rm -f ./*.o ./*.so ./*.pyc 
	rm -f ./*.c ./*.py
