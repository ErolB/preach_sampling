GTEST_DIR = /home/erol/Documents/googletest

EXC_FLAGS = -std=c++11 -o
OBJ_FLAGS = -std=c++11 -c

GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h $(GTEST_DIR)/include/gtest/internal/*.h

GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

all: preach

util.o: util.cpp util.h
	g++ $(OBJ_FLAGS) util.cpp

Graph.o: Graph.cpp Graph.h util.o
	g++ $(OBJ_FLAGS) Graph.cpp

Cut.o: Cut.cpp Cut.h Graph.o
	g++ $(OBJ_FLAGS) Cut.cpp

Sampling.o: Sampling.cpp Sampling.h Cut.o
	g++ $(OBJ_FLAGS) Sampling.cpp

Probing.o: Probing.cpp Probing.h Sampling.o
	g++ $(OBJ_FLAGS) Probing.cpp

preach.o: preach.cc preach.h Probing.o
	g++ $(OBJ_FLAGS) preach.cc

preach: preach.o
	g++ -g $(EXC_FLAGS) preach preach.o Cut.o Graph.o Sampling.o Probing.o util.o

classify_nodes.o: classify_nodes.cpp
	g++ $(OBJ_FLAGS) classify_nodes.cpp

classify: classify_nodes.o Cut.o Graph.o preach.o util.o
	g++ $(EXC_FLAGS) classify classify_nodes.o Cut.o Graph.o util.o

manual_tests.o: manual_tests.cpp
	g++ $(OBJ_FLAGS) manual_tests.cpp

tests: manual_tests.o Probing.o Sampling.o Graph.o Cut.o util.o libgtest.a
	g++ -g -o tests $^ -lpthread

clean:
	-rm -f preach
	-rm -f util.o
	-rm -f Graph.o
	-rm -f Cut.o
	-rm -f Sampling.o
	-rm -f Probing.o
	-rm -f classify
	-rm -f tests.o
	-rm -f gtest-all.o
	-rm -f manual_tests.o
	-rm -f tests
