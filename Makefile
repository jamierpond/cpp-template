
.PHONY: build test compile

compile:
	cmake --build build

build:
	cmake -G Ninja -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
		-DCMAKE_C_COMPILER=clang \
		-DCMAKE_CXX_COMPILER=clang++


test:
	make build
	make compile
	cd build && ctest --output-on-failure
