all: node

node: Node.o Common.o
	g++ Node.o Common.o -o node

Node.o: Common.hpp Node.cpp  
	g++ -c Node.cpp -o Node.o

Common.o: Common.hpp Common.cpp
	g++ -c Common.cpp -o Common.o

clean:
	rm -rf *.o node