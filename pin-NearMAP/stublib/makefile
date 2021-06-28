.PHONY: all clean python

all: stublib.a python

clean:
	rm -f stublib.o stublib.a

stublib.a: stublib.c makefile
	cc -DNO_PYTHON -O2 -o stublib.o -c stublib.c
	ar rcs stublib.a stublib.o

python: stublib.a
	python setup.py build --build-lib ./pybuild
