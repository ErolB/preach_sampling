#ifndef PREACH_H
#define PREACH_H

#include <vector>
#include <bitset>
#include <iostream>
#include <assert.h>
#include <typeinfo>
#include <sstream>
#include <string>
#include <numeric>
#include <math.h>

#include <lemon/list_graph.h>
#include <lemon/bfs.h>

#include <fstream>
#include <map>
#include <ctime>
#include <sys/time.h>

#include <sys/time.h> // for gettimeofday
#include <stdlib.h> // drand48

#include "Stl.h"
#include "util.h"

// for debugging
#include <stdio.h>
#ifdef DEBUG
#define dprintf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define dprintf(fmt, ...)
#endif

#define MAX_NODES 256
#define MAX_EDGES 1024
#define SURE 1.0

using namespace std;
using lemon::ListDigraph;
using lemon::INVALID;
using lemon::Bfs;

typedef ListDigraph::ArcMap<double> WeightMap;
typedef ListDigraph::NodeMap<string> NodeNames;
typedef map<string, ListDigraph::Node> NameToNode;
typedef ListDigraph::ArcMap<int> ArcIntMap;

typedef bitset<MAX_NODES> Nodes_T;
typedef bitset<MAX_EDGES> Edges_T;
typedef unsigned long ulong;

const string SOURCE = "SOURCE";
const string SINK = "SINK";
const string PRE_YES = "pre";
const string PRE_NO = "nopre";

const string SAMPLING_RANDOM = "rand";
const string SAMPLING_FIXED_RANDOM = "fixrand";
const string SAMPLING_FIXED_WEIGHTED_RANDOM = "fixwrand";

/////////////////////////////////////////////////////////////////////////////////////////////////////

// macro to smoth up the use of bitsets
#define FOREACH_BS(v, vSet)	  \
	for (size_t v=vSet._Find_first(); v!=vSet.size(); v=vSet._Find_next(v))

void ReadList(string filename, vector<string>& list);

void splitString(string str, vector<string>& result, char delim);

string arcToString(ListDigraph& g, WeightMap& wMap, NodeNames& nNames, ListDigraph::Arc& arc);

string edgesToReferenceString(ListDigraph& g, WeightMap& wMap, NodeNames& nNames);

#endif




