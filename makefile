test: main.o
	g++ -Wall -ggdb main.o -o test
main.o: main.cpp
	g++ -Wall -ggdb -c main.cpp