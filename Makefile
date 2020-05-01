.DEFAULT_GOAL := help

################################################################################
#> Development
################################################################################

define HELP
print('\nUsage: make \033[36m[target]\033[0m\n')
with open('Makefile') as f:
	for line in f.readlines():
		if 'elif' in line or 'print' in line: continue
		if line.startswith('#>'):
			print(line.strip()[3:])
		elif ': ##> ' in line:
			print('    \033[36m{}\033[0m - {}'.format(*line.strip().split(': ##> ')))
print('')
endef
export HELP

.PHONY: help
help: ##> prints this message
	@python -c "$$HELP"

.PHONY: watch
watch: ##> builds and runs the tests on file save
	while sleep 1; do \
		find include test -iname '*.h' -o -iname '*.cpp' | \
			entr -cd make build test \
	; done

################################################################################
#> Build
################################################################################

.PHONY: configure
configure: ##> configures cmake
	mkdir -p build
	(cd build && cmake -DCMAKE_BUILD_TYPE=Debug ..)

.PHONY: build
build: ##> builds the unit tests
	mkdir -p build
	(cd build && make)

.PHONY: clean
clean: ##> removes the build files
	mkdir -p build
	(cd build && make clean)

################################################################################
#> Test
################################################################################

.PHONY: test
test: ##> runs the unit tests
	./build/bin/unit_tests --gtest_shuffle

################################################################################
#> Static Analysis
################################################################################

.PHONY: format
format: ##> formats the code using clang-format
	find include test -iname '*.h' -o -iname '*.cpp' | \
		xargs clang-format -i

.PHONY: cppcheck
cppcheck: ##> runs cppcheck on the code
	find include -iname '*.h' -o -iname '*.cpp' | \
	xargs cppcheck \
		--enable=warning,style,performance,portability,information,missingInclude \
		--inconclusive \
		--inline-suppr \
		--std=c++11 \
		--language=c++ \
		--error-exitcode=1

.PHONY: tidy
tidy: ##> runs clang-tidy on the code
	find include -iname '*.h' -o -iname '*.cpp' | \
		xargs -I{} clang-tidy -extra-arg-before=-xc++ {} -- -std=c++11 -Iinclude

.PHONY: cyclomatic-complexity
cyclomatic-complexity: ##> runs a cyclomatic complexity on the code
	python -m lizard -l cpp include/ -C 10 -L 50 -a 5

################################################################################
#> Docker
################################################################################

.PHONY: docker-build
docker-build: ##> builds the docker container for development
	docker build --rm -f ".devcontainer/Dockerfile" -t diydi:latest .

.PHONY: docker-run
docker-run: ##> runs the docker container for development
	docker run --rm -it -w /diydi -v $(shell pwd):/diydi/ diydi:latest $(cmd)
