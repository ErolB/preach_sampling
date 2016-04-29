all: preach

preach: preach.cc
	g++ -o preach preach.cc --std=c++11

classify_nodes: classify_nodes.cpp
	g++ -o classify classify_nodes.cpp --std=c++11

clean:
	-rm preach
	-rm classify
