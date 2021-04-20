.PHONY: all clean

all: testprogram

clean:
	rm -f testprogram testprogram.o

testprogram: testprogram.cpp makefile
	c++ -O2 -o testprogram.o -I../stublib/include -c testprogram.cpp
	c++ -o testprogram testprogram.o ../stublib/stublib.a
