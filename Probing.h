//
// Created by erol on 5/9/16.
//

#ifndef PREACH_SAMPLING_PROBING_H
#define PREACH_SAMPLING_PROBING_H

// function signatures

void ProbeRandom(ListDigraph& gOrig, WeightMap& wMapOrig, ArcIntMap& arcIdMap,
                 ListDigraph::Node& sourceOrig, ListDigraph::Node& targetOrig,
                 double samplingProb, Edges_T& sampleEdges, int probeSize, int probeRepeats, bool weighted);

#endif //PREACH_SAMPLING_PROBING_H
