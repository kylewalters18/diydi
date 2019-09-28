.PHONY: up ssh build test format gdb

env:
	docker build --rm -f "Dockerfile" -t diydi:latest .

run:
	docker run --rm -it -v $(shell pwd):/diydi diydi:latest $(cmd)

build:
	mkdir -p build
	(cd build && cmake .. && make)

test:
	./build/bin/unit_tests --gtest_shuffle

format:
	find include test -iname *.h -o -iname *.cpp | xargs clang-format -i
