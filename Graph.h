//
// Created by erol on 5/9/16.
//

#ifndef PREACH_SAMPLING_GRAPH_H
#define PREACH_SAMPLING_GRAPH_H

//function signatures

void PrintGraph(ListDigraph&);

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

void RemoveRedundantCuts(vector<Cut>& cuts);

void Preprocess(ListDigraph& g, WeightMap& wMap, NodeNames& nMap, NameToNode& nodeMap, ArcIntMap& arcIdMap, string sourcesFile,
                string targetsFile, string pre);

double Solve(ListDigraph& g, WeightMap& wMap, ListDigraph::Node& source, ListDigraph::Node& target, vector<Cut>& cuts);

#endif
