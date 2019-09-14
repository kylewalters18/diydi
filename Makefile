.PHONY: test format

test:
	mkdir -p build/test
	(cd build/test && cmake ../.. && make -j4 && ./unit_tests --gtest_shuffle)

format:
	find include test -iname *.h -o -iname *.cpp | xargs clang-format -i
