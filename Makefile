all: main

main: main.cpp monad.h
	g++ -std=c++20 main.cpp -o main
