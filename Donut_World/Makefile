all: testunit

vector.o: vector.cpp

	g++ -Wall -c -o vector.o vector.cpp

matrix.o: matrix.cpp
	g++ -Wall -c -o matrix.o matrix.cpp
	
main.o: main.cpp
	g++ -Wall -c -o main.o main.cpp

testunit: vector.o matrix.o main.o

	g++ vector.o matrix.o main.o -o testunit

clean: 
	rm -f *.o testunit
