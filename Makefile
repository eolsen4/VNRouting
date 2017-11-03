all: node

node: Node.o Common.o
	g++ -pthread Node.o Common.o -o node

Node.o: Common.hpp Node.cpp  
	g++ -c -std=c++11 -DDEBUG Node.cpp -o Node.o

Common.o: Common.hpp Common.cpp
	g++ -c -std=c++11 Common.cpp -o Common.o

clean:
	rm -rf *.o node
