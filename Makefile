
.PHONY: all clean run test
SHELL := /bin/bash

run:
	cppc c
	./build/MyProject

test:
	cppc c
	cd ./build
	ctest
	cd ..

