.PHONY: all configure build test format watch

all: configure build test format

configure:
	mkdir -p build
	(cd build && cmake ..)

build:
	mkdir -p build
	(cd build && make)

test:
	./build/bin/unit_tests --gtest_shuffle

format:
	find include test -iname '*.h' -o -iname '*.cpp' | xargs clang-format -i

# NOTE: Intended to be run on host machine (requires entr command)
watch:
	while sleep 1; do \
		find include test -iname '*.h' -o -iname '*.cpp' | \
			entr -cd make build test \
	; done
