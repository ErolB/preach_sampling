//
// Created by erol on 5/9/16.
//

#include "preach.h"

/* This function does random probing.
** Does multiple random sampling and selects the edge set that makes the least runtime on average
    if weighted = true: weighted by their sizes, disappearance probability and invloved cut size
** The resulting edges should be in sampleEdges
*/
void ProbeRandom(ListDigraph& gOrig, WeightMap& wMapOrig, ArcIntMap& arcIdMap,
                 ListDigraph::Node& sourceOrig, ListDigraph::Node& targetOrig,
                 double samplingProb, Edges_T& sampleEdges, int probeSize, int probeRepeats, bool weighted){

    vector<EdgeSubset> chancesOrig;
    if (weighted){
        vector<Cut> cuts;
        vector< pair< vector<int>, int > > edgeSubsets;
        FindSomeGoodCuts(gOrig, sourceOrig, targetOrig, cuts, edgeSubsets);

        //Compile a vector of EdgeSubset objects
        vector<EdgeSubset> ess;
        double minScore = 1000000000.0;
        double maxScore = -1.0;
        int idCounter = 0;
        FOREACH_STL(subset, edgeSubsets){
            idCounter ++;
            EdgeSubset es;
            es.id = idCounter;
            es.subset = subset.first;
            es.cutsize = subset.second;
            es.successProb = SuccessProb(subset.first, gOrig, wMapOrig);
//            bool jump = false;
//            FOREACH_STL(arcId, es.subset){
//                ListDigraph::Arc arc = gOrig.arcFromId(arcId);
//                jump = jump || gOrig.source(arc) == sourceOrig || gOrig.target(arc) == targetOrig;
//            }END_FOREACH;
//            if (jump) continue; // we don't want any subset that is involved with wource or target
            ess.push_back(es);
            if (es.score() < minScore)
                minScore = es.score();
            if (es.score() > maxScore)
                maxScore = es.score();
        }END_FOREACH;

        // form a chances vector
        FOREACH_STL(es, ess){
            double range = maxScore - minScore;
            double esScore = es.score();
            double esRatio = (esScore - minScore) * 100.0 / range; //normalization
            int esChances = (int) ceil(esRatio);
            for (int i=0; i<esChances; i++)
                chancesOrig.push_back(es);
        }END_FOREACH;
    }

    map< string, vector<double> > durations;
    map<string, Edges_T> samples;
    for (int i=0; i<probeSize; i++){
        Edges_T sample;
        vector<EdgeSubset> chances = chancesOrig;
        iteration(gOrig, wMapOrig, arcIdMap, sourceOrig, targetOrig, false, samplingProb, sample, false, weighted, chances);
        string sampleString = sample.to_string<char,std::string::traits_type,std::string::allocator_type>();
        samples[sampleString] = sample;
        for (int j=0; j<probeRepeats; j++){
            cout << ".";
            cout.flush();
            double startCPUTime = getCPUTime();
            iteration(gOrig, wMapOrig, arcIdMap, sourceOrig, targetOrig, true, samplingProb, sample, false, weighted, chances); //last two parameters don't matter here
            double duration = getCPUTime() - startCPUTime;
            durations[sampleString].push_back(duration);
        }
    }
    // get the min average time sample
    Edges_T minSample;
    double minAvg = 1000000000;
    for (map< string, vector<double> >::iterator it=durations.begin(); it != durations.end(); ++it){
        double avg = std::accumulate(it->second.begin(), it->second.end(), 0.0) / it->second.size();
        if (avg < minAvg){
            minAvg = avg;
            minSample = samples[it->first];
        }
    }
    sampleEdges = minSample;
}