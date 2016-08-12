CC = g++

EXC_FLAGS = --std=c++11 -o
OBJ_FLAGS = --std=c++11 -c

all: preachRandom preachOptimized

util.o: util.cpp util.h
	$(CC) $(OBJ_FLAGS) util.cpp

Graph.o: Graph.cpp Graph.h util.o
	$(CC) $(OBJ_FLAGS) Graph.cpp

CutRandom.o: CutRandom.cpp Cut.h Graph.o
	$(CC) $(OBJ_FLAGS) CutRandom.cpp

SamplingRandom.o: Sampling.cpp Sampling.h CutRandom.o
	$(CC) $(EXC_FLAGS) SamplingRandom.o $(OBJ_FLAGS) Sampling.cpp CutRandom.o

preachRandom.o: preach.cc preach.h SamplingRandom.o
	$(CC) $(EXC_FLAGS) preachRandom.o $(OBJ_FLAGS) preach.cc

preachRandom: preachRandom.o
	$(CC) $(EXC_FLAGS) preachRandom preachRandom.o CutRandom.o Graph.o SamplingRandom.o util.o

CutOptimized.o: CutOptimized.cpp Cut.h Graph.o
	$(CC) $(OBJ_FLAGS) CutOptimized.cpp

SamplingOptimized.o: Sampling.cpp Sampling.h CutOptimized.o
	$(CC) $(EXC_FLAGS) SamplingOptimized.o $(OBJ_FLAGS) Sampling.cpp CutOptimized.o

preachOptimized.o: preach.cc preach.h SamplingOptimized.o
	$(CC) $(EXC_FLAGS) preachOptimized.o $(OBJ_FLAGS) preach.cc SamplingOptimized.o

preachOptimized: preachOptimized.o
	$(CC) $(EXC_FLAGS) preachOptimized preachOptimized.o CutOptimized.o Graph.o SamplingOptimized.o util.o

clean:
	-rm -f preachOptimized
	-rm -f preachRandom
	-rm -f util.o
	-rm -f Graph.o
	-rm -f CutRandom.o
	-rm -f SamplingRandom.o
	-rm -f preachRandom.o
	-rm -f CutOptimized.o
	-rm -f SamplingOptimized.o
	-rm -f preachOptimized.o
