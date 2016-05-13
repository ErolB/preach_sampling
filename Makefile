EXC_FLAGS = --std=c++11 -o
OBJ_FLAGS = --std=c++11 -c

all: preach

preach: preach.o cut.o graph.o sampling.o probing.o
	g++ $(EXC_FLAGS) preach preach.o cut.o graph.o sampling.o probing.o

classify: classify_nodes.o cut.o graph.o sampling.o probing.o
	g++ $(EXC_FLAGS) classify classify_nodes.o cut.o graph.o sampling.o probing.o

preach.o: preach.cc preach.h
	g++ $(OBJ_FLAGS) preach.cc

classify_nodes.o: classify_nodes.cpp
	g++ $(OBJ_FLAGS) classify_nodes.cpp

cut.o: Cut.cc Cut.h
	g++ $(OBJ_FLAGS) Cut.cc

graph.o: Graph.cc Graph.h cut.o
	g++ $(OBJ_FLAGS) Graph.cc

sampling.o: Sampling.cc Sampling.h
	g++ $(OBJ_FLAGS) Sampling.cc

probing.o: Probing.cc Probing.h
	g++ $(OBJ_FLAGS) Probing.cc

clean:
	-rm preach
	-rm classify
