all: node controlProg

node: Node.o Common.o
	g++ -pthread Node.o Common.o -o node

controlProg: ControlProg.o Common.o
	g++ ControlProg.o Common.o -o controlProg

Node.o: Common.hpp Node.cpp  
	g++ -c -g -std=c++11 Node.cpp -o Node.o

ControlProg.o: Common.hpp ControlProg.cpp
	g++ -c -g -std=c++11 ControlProg.cpp -o ControlProg.o

Common.o: Common.hpp Common.cpp
	g++ -c -std=c++11 Common.cpp -o Common.o

clean:
	rm -rf *.o node controlProg
