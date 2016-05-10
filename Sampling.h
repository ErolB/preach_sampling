//
// Created by erol on 5/9/16.
//

#ifndef PREACH_SAMPLING_SAMPLING_H
#define PREACH_SAMPLING_SAMPLING_H

// function signatures

void SampleRandom(ListDigraph& g, WeightMap& wMap, ListDigraph::Node& source, ListDigraph::Node& target,
                  double samplingProb, Edges_T& sampleEdges, ArcIntMap& arcIdMap);

void SampleFixed(ListDigraph& g, WeightMap& wMap, ListDigraph::Node& source, ListDigraph::Node& target,
                 Edges_T& sampleEdges, ArcIntMap& arcIdMap);

void SampleWeightedRandom(ListDigraph& g, WeightMap& wMap, ListDigraph::Node& source, ListDigraph::Node& target,
                          double samplingProb, Edges_T& sampleEdges, ArcIntMap& arcIdMap, vector<EdgeSubset>& chances);

double iteration(ListDigraph& gOrig, WeightMap& wMapOrig, ArcIntMap& arcIdMapOrig, ListDigraph::Node& sourceOrig,
                 ListDigraph::Node& targetOrig, bool fixed, double samplingProb, Edges_T& sampleEdges, bool print,
                 bool weighted, vector<EdgeSubset>& chances);


#endif //PREACH_SAMPLING_SAMPLING_H
