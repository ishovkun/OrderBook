all: tests

COMPILER = g++
FLAGS = -std=c++2a  -Wall -Wextra -Werror -pedantic -O0 -fsanitize=address

tests: tests/*
	$(COMPILER) $(FLAGS) tests/run_tests.cpp -o run_tests
	./run_tests
