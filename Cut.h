//
// Created by erol on 5/9/16.
//

#include "preach.h"

#ifndef PREACH_SAMPLING_CUT_H
#define PREACH_SAMPLING_CUT_H

#include "Graph.h"

// class definitions

/*A class modeling a vertex cut*/
class Cut{
    Nodes_T middle; // The nodes in the
    Nodes_T left; // Set of nodes on the left
    Nodes_T right; // Set of nodes on the right
    Edges_T coveredEdges; // Set of edges covered by this cut (left and middle)

public:
    Cut(){}

    // consturctor for a specified cut
    Cut(Nodes_T& _left, Nodes_T& _middle, Nodes_T& _right, Edges_T& _covered):
            left(_left), middle(_middle), right(_right), coveredEdges(_covered){}

    Nodes_T& getMiddle();
    Nodes_T& getRight();
    Nodes_T& getLeft();
    Edges_T& getCoveredEdges();
    int size();
};

// function signatures

void PrintCut(Cut& cut, ListDigraph& g);

void RemoveObsoleteCuts(vector<Cut>& cuts, Cut& cut);

void RefineCuts(vector<Cut>& cuts, ListDigraph& g, ListDigraph::Node& target, vector< pair< vector< int >, int> > & edgeSubsets);

Cut createFirstCut(ListDigraph& g, ListDigraph::Node source, ListDigraph::Node target);

void FillEdgeSubsets(ListDigraph& g, vector<Cut>& cuts, vector< pair< vector<int>, int > > & edgeSubsets);

void FindSomeGoodCuts(ListDigraph& g, ListDigraph::Node source, ListDigraph::Node target, vector<Cut>& cuts,
                      vector< pair< vector<int>, int > > & edgeSubsets);

void FindAllCuts(Cut& currentCut, vector<Cut>& cuts,  ListDigraph& g, ListDigraph::Node target);

void ConsumeSausage(ListDigraph& g, WeightMap& wMap, Polynomial& poly, Edges_T& sausage, Nodes_T& endNodes);

void RemoveRedundantCuts(vector<Cut>& cuts);

void Preprocess(ListDigraph& g, WeightMap& wMap, NodeNames& nMap, NameToNode& nodeMap, ArcIntMap& arcIdMap, string sourcesFile,
                string targetsFile, string pre);

double Solve(ListDigraph& g, WeightMap& wMap, ListDigraph::Node& source, ListDigraph::Node& target, vector<Cut>& cuts);

void PrintCuts(vector<Cut>& cuts, ListDigraph& g);

#endif //PREACH_SAMPLING_CUT_H