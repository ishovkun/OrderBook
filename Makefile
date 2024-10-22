all: main

COMPILER = g++
FLAGS = -std=c++23  -Wall -Wextra -Werror -pedantic -g -O0 -fsanitize=address

tests: tests/* Makefile
	$(COMPILER) $(FLAGS) tests/run_tests.cpp -o run_tests
	./run_tests

main: ./*.cpp ./*.hpp Makefile
	$(COMPILER) $(FLAGS) app.cpp -o main
