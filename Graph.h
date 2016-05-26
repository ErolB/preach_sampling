//
// Created by erol on 5/9/16.
//

#include "preach.h"

#ifndef PREACH_SAMPLING_GRAPH_H
#define PREACH_SAMPLING_GRAPH_H

// classes for polynomial caluclations

/*Represents an edge subset, with its sampling score*/
class EdgeSubset{
public:
    int id;
    vector<int> subset;
    int cutsize;
    double successProb;

    double score(){
        return successProb * cutsize / subset.size();
    }
};

class Term{
    Nodes_T z; // Reachable nodes
    Nodes_T w; // Unreachable nodes
    Edges_T x; // Present edges
    Edges_T y; // Absent edges
    double coeff; // Duh!

public:
    Term(){}

    //Term(Nodes_T& _z, Nodes_T& _w, Edges_T& _x, Edges_T& _y, double _coeff):
    //    z(_z), w(_w), x(_x), y(_y), coeff(_coeff){}

    Term(Nodes_T& _z, Nodes_T& _w, double _coeff):
            z(_z), w(_w), coeff(_coeff){}

    Term(Nodes_T& _z, Nodes_T& _w):
            z(_z), w(_w){
        coeff = 0.0;
    }

    string toString();

    /*Multiply by a new edge term*/
    void multiply(int subscript, double p, bool inverse);

    double getCoeff();

    void addToCoeff(double increment);

    bool hasZ();

    /*Checks for collapsing, returns the set of nodes it collapses to in newZ
    returns true if collapses*/
    bool collapse(Edges_T& midEdges, map< int, vector<int> >& edgeTerminals, Nodes_T& endNodes, Nodes_T& newZ);
};

class Polynomial{
    vector<Term> terms; // regular terms with start nodes and middle edges
    map<string, Term> endTerms; // black holes with end nodes, the key is z nodes as ulong

public:
    Polynomial(vector<Term>& _terms): terms(_terms){}

    /*Adds and edge: mutiplies the whole polynomial by
    pX_subscript + (1-p)Y_subscript
    DOES NOT COLLAPSE AUTOMATICALLY*/
    void addEdge(int subscript, double p);

    /*Advances the polynomial: prepares it for the next cut.
    transfers endTerms to terms, and reinitializes endTerms*/
    void advance(){
        //SANITY CHECK
        assert(terms.size() == 0);
        //copy endTerms to terms
        terms = vector<Term>();
        double totalCoeff = 0.0;
        for (map<string, Term>::iterator iter = endTerms.begin(); iter != endTerms.end(); ++iter){
            totalCoeff += iter->second.getCoeff();
            terms.push_back(iter->second);
        }
        //reinitialize endTerms
        endTerms = map<string, Term>();

        //SANITY CHECK
        assert(totalCoeff < 1.01 && totalCoeff > 0.99);
    }

    double getResult();

    /*Collapses the polynomial: iterates over each term
    to collapse it if possible.
    midEdges are all edges currently considered between
    the past cut and the next one.
    edgeTerminals is a hash from edge id to its source and target ids
    endNodes are the nodes in the next cut*/
    void collapse(Edges_T& midEdges, map< int, vector<int> >& edgeTerminals, Nodes_T& endNodes);
};

// function signatures

void PrintGraph(ListDigraph& g);

void EdgesAsBitset(ListDigraph& g, Edges_T& edges);

bool EdgeExists(ListDigraph& g, ListDigraph::Node& source, ListDigraph::Node& target);

ListDigraph::Node FindNode(string name, ListDigraph& g, NodeNames& nMap, NameToNode& nodeMap);

int getNodeInDegree(ListDigraph& g, ListDigraph::Node& node);

int getNodeOutDegree(ListDigraph& g, ListDigraph::Node& node);

void CreateGraph(char* filename, ListDigraph& g, ListDigraph::NodeMap<string>& nMap, NameToNode& nodeMap,
                 WeightMap& wMap, ArcIntMap& arcIdMap);

void minimizeGraph(ListDigraph& g, WeightMap& wMap, ArcIntMap& arcIdMap, ListDigraph::Node& source, ListDigraph::Node& target);

bool CheckProcessedReference(ListDigraph& g, WeightMap& wMap, NodeNames& nNames, string reference);

void reverseGraph(ListDigraph& g);

void UnifyTerminals(ListDigraph& g, WeightMap& wMap, NodeNames& nMap, NameToNode& nodeMap, ArcIntMap& arcIdMap,
                    string sourcesFile, string targetsFile);

void CollapseELementaryPaths(ListDigraph& g, WeightMap& wMap, ArcIntMap& arcIdMap, ListDigraph::Node& source, ListDigraph::Node& target);

void RemoveIsolatedNodes(ListDigraph& g, ListDigraph::Node& source, ListDigraph::Node& target);

void RemoveSelfCycles(ListDigraph& g);

#endif
