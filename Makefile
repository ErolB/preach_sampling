GTEST_DIR = /usr/include/gtest
GTEST_SRC_DIR = /usr/src/gtest
CC = g++

EXC_FLAGS = --std=c++11 -o
OBJ_FLAGS = --std=c++11 -c

GTEST_HEADERS = $(GTEST_DIR)/*.h $(GTEST_DIR)/internal/*.h

GTEST_SRCS_ = $(GTEST_SRC_DIR)/src/*.cc $(GTEST_SRC_DIR)/src/*.h $(GTEST_HEADERS)

CPPFLAGS += -isystem $(GTEST_DIR)

all: preach

util.o: util.cpp util.h
	$(CC) $(OBJ_FLAGS) util.cpp

Graph.o: Graph.cpp Graph.h util.o
	$(CC) $(OBJ_FLAGS) Graph.cpp

Cut.o: Cut.cpp Cut.h Graph.o
	$(CC) $(OBJ_FLAGS) Cut.cpp

Sampling.o: Sampling.cpp Sampling.h Cut.o
	$(CC) $(OBJ_FLAGS) Sampling.cpp

Probing.o: Probing.cpp Probing.h Sampling.o
	$(CC) $(OBJ_FLAGS) Probing.cpp

preach.o: preach.cc preach.h Probing.o
	$(CC) $(OBJ_FLAGS) preach.cc

preach: preach.o
	$(CC) $(EXC_FLAGS) preach preach.o Cut.o Graph.o Sampling.o Probing.o util.o

classify_nodes.o: classify_nodes.cpp
	$(CC) $(OBJ_FLAGS) classify_nodes.cpp

classify: classify_nodes.o Cut.o Graph.o preach.o util.o
	$(CC) $(EXC_FLAGS) classify classify_nodes.o Cut.o Graph.o util.o

tests.o: tests.cpp
	$(CC) $(OBJ_FLAGS) tests.cpp

tests: tests.o Probing.o Sampling.o Graph.o Cut.o util.o libgtest.a
	$(CC) --std=c++11 -o tests $^ -lpthread

clean:
	-rm -f preach
	-rm -f util.o
	-rm -f Graph.o
	-rm -f Cut.o
	-rm -f Sampling.o
	-rm -f Probing.o
	-rm -f classify
