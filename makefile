SRC=$(wildcard src/*.cpp)
CFLAGS=$(shell pkg-config --cflags --libs /usr/lib/pkgconfig/opencv4.pc)

.PHONY:build

build: $(SRC)
	mkdir -p build
	g++ src/main.cpp $(CFLAGS) -o build/mvrbow

run: build
	@ build/mvrbow $(HOME)/Downloads/movie.mkv