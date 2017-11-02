all: node

node: Node.o
	g++ Node.o -o node

Node.o: Common.hpp Node.cpp  
	g++ -c Node.cpp -o Node.o

clean:
	rm -rf *.o node