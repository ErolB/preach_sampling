//
// Contains functions for working with graphs
//

#ifndef PREACH_H
#include "preach.h"
#define PREACH_H
#endif

// map names to nodes
ListDigraph::Node FindNode(string name, ListDigraph& g,
                           NodeNames& nMap,
                           NameToNode& nodeMap
){

    // look in map
    if (nodeMap.find(name) == nodeMap.end()){ // not found, add
        ListDigraph::Node node = g.addNode();
        nodeMap[name]=node;
    }

    ListDigraph::Node node = nodeMap[name];
    nMap[node]=name;

    return node;
}

bool EdgeExists(ListDigraph& g, ListDigraph::Node& source, ListDigraph::Node& target){
    for (ListDigraph::OutArcIt arc(g, source); arc != INVALID; ++arc){
        if (g.id(g.target(arc)) == g.id(target))
            return true;
    }
    return false;
}

// Create the graph from the file
void CreateGraph(char* filename, ListDigraph& g,
                 ListDigraph::NodeMap<string>& nMap,
                 NameToNode& nodeMap,
                 WeightMap& wMap, ArcIntMap& arcIdMap){
    fstream in(filename);

    while (!in.eof()){
        string start;
        string stop;
        double weight = -1.0;
        in >> start >> stop >> weight;

        if (weight==-1.0)
            continue;

        ListDigraph::Node sN = FindNode(start, g, nMap, nodeMap );
        ListDigraph::Node tN = FindNode(stop, g, nMap, nodeMap);
        if (!EdgeExists(g, sN, tN)){
            ListDigraph::Arc a = g.addArc(sN, tN);
            wMap[a] = weight;
            arcIdMap[a] = g.id(a);
        }
    }
    in.close();
}

void ReadList(string filename, vector<string>& list) {
    fstream in(filename.data());
    string item;
    while (!in.eof()) {
        item = "";
        in >> item;

        if (item.empty())
            continue;
        list.push_back(item);
    }
    in.close();
}

/*Reads sources and targets and adds a unified source and unified sink to the graph
 * HOW: adds a new SOURCE node to the graph and a 1.0-weight edge to all sources
 * same with SINK and all targets*/
void UnifyTerminals(ListDigraph& g,
                    WeightMap& wMap,
                    NodeNames& nMap,
                    NameToNode& nodeMap,
                    ArcIntMap& arcIdMap,
                    string sourcesFile,
                    string targetsFile){
    vector<string> sources;
    vector<string> targets;

    //read sources and targets
    ReadList(sourcesFile, sources);
    ReadList(targetsFile, targets);

    //create unified source and sink nodes
    ListDigraph::Node source = FindNode(SOURCE, g, nMap, nodeMap);
    ListDigraph::Node sink = FindNode(SINK, g, nMap, nodeMap);

    // add an edge from the new source to all sources
    FOREACH_STL(nodeName, sources){
            ListDigraph::Node node = FindNode(nodeName, g, nMap, nodeMap);
            if (!EdgeExists(g, source, node)){
                ListDigraph::Arc arc = g.addArc(source, node);
                wMap[arc] = SURE;
                arcIdMap[arc] = g.id(arc);
            }
        }END_FOREACH;

    // add an edge from all targets to the new sink
    FOREACH_STL(nodeName, targets){
            ListDigraph::Node node = FindNode(nodeName, g, nMap, nodeMap);
            if (!EdgeExists(g, node, sink)){
                ListDigraph::Arc arc = g.addArc(node, sink);
                wMap[arc] = SURE;
                arcIdMap[arc] = g.id(arc);
            }
        }END_FOREACH;
}

/*Gets the In-degree of a node*/
int getNodeInDegree(ListDigraph& g, ListDigraph::Node& node){
    int count = 0;
    for (ListDigraph::InArcIt arc(g, node); arc != INVALID; ++arc)
        count++;
    return count;
}

/*Gets the Out-degree of a node*/
int getNodeOutDegree(ListDigraph& g, ListDigraph::Node& node){
    int count = 0;
    for (ListDigraph::OutArcIt arc(g, node); arc != INVALID; ++arc)
        count++;
    return count;
}
