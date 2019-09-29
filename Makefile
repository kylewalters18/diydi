.PHONY: all env run build test format watch .build .test .format

all:
	make env run cmd="make .build .test .format"

env:
	docker build --rm -f "Dockerfile" -t diydi:latest .

run:
	docker run --rm -it -v $(shell pwd):/diydi diydi:latest $(cmd)

build:
	make run cmd="make .build"

test:
	make run cmd="make .test"

format:
	make run cmd="make .format"

# NOTE: Intended to be run on host machine (requires entr command)
watch:
	find include test -iname '*.h' -o -iname '*.cpp' | \
		entr make run cmd="make .build .test"


###############################################################################
########################### Command implementations ###########################
###############################################################################

.build:
	mkdir -p build
	(cd build && cmake .. && make)

.test:
	./build/bin/unit_tests --gtest_shuffle

.format:
	find include test -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i
