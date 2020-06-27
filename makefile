SRC=$(wildcard src/*.cpp)
CFLAGS=$(shell pkg-config --cflags --libs /usr/lib/pkgconfig/opencv4.pc)

.PHONY:build

build: $(SRC)
	mkdir -p build
	g++ src/main.cpp $(CFLAGS) -g -o build/mvrbow

release: $(SRC)
	mkdir -p build
	g++ src/main.cpp $(CFLAGS) -O2 -o build/mvrbow

run:
	build/mvrbow $(HOME)/Downloads/movie.mp4

image:
	pdflatex -output-directory=out template.tex