.PHONY: all clean

all: stublib.a

clean:
	rm -f stublib.o stublib.a

stublib.a: stublib.c makefile
	cc -O2 -o stublib.o -c stublib.c
	ar rcs stublib.a stublib.o

