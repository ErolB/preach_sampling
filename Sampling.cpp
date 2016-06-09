//
// Created by erol on 5/9/16.
//

#include "Sampling.h"

double SuccessProb(vector<int> subset, ListDigraph& gOrig, WeightMap& wMapOrig){
    double result = 1.0;
    FOREACH_STL(arcId, subset){
            result *= (1.0 - wMapOrig[gOrig.arcFromId(arcId)]);
        }END_FOREACH;
    return result + 0.001;
}

/* This function samples the graph. The edges to sample are randomly selected.
** For every edge (excpet from SOURCE or to TARGET), it tosses a coin with success prob = samplingProb
** If success, the edge is sampled using its edge probability
** Edges that result in 0 are erased from g.
** Also sampleEdges will change to reflect the resulting sampled set of edges.
*/
void SampleRandom(ListDigraph& g, WeightMap& wMap,
                  ListDigraph::Node& source, ListDigraph::Node& target,
                  double samplingProb, Edges_T& sampleEdges, ArcIntMap& arcIdMap){

    dprintf("Start SampleRandom\n");
    vector<ListDigraph::Arc> toDelete;
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        if (g.source(arc) == source || g.target(arc) == target)
            continue;
        if (nextRand() <= samplingProb){ // generic coin toss
            sampleEdges.set(arcIdMap[arc]);
            if (nextRand() <= wMap[arc]){ // sampling coin toss
                wMap[arc] = 1.0;
            } else {
                wMap[arc] = 0.0;
                toDelete.push_back(arc);
                //g.erase(arc);
            }
        }
    }
    // now delete all the edges that need deletion
    for (ListDigraph::Arc &edge: toDelete) {// foreach arc in toDelete
        g.erase(edge);
    }

}

/* This function samples the edges in sampleEdges */
void SampleFixed(ListDigraph& g, WeightMap& wMap,
                 ListDigraph::Node& source, ListDigraph::Node& target,
                 Edges_T& sampleEdges, ArcIntMap& arcIdMap){
    dprintf("Start SampleFixed");
    vector<ListDigraph::Arc> toDelete;
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        if (sampleEdges.test(arcIdMap[arc])){ // edge is in sampleEdges
            if (nextRand() <= wMap[arc]){ // sampling coin toss
                wMap[arc] = 1.0;
            } else {
                wMap[arc] = 0.0;
                //g.erase(arc);
                toDelete.push_back(arc);
            }
        }
    }
    for (ListDigraph::Arc &edge: toDelete) { // foreach arc in toDelete
        g.erase(edge);
    }
}

/* This method samples the graph using weighted random sampling through the chances vector.
   It randomly selects an edge subset from chances vector, samples its members using their probability
   Until the budget is met, which is derived from the samplingProb and the number of edges
*/
void SampleWeightedRandom(ListDigraph& g, WeightMap& wMap,
                          ListDigraph::Node& source, ListDigraph::Node& target,
                          double samplingProb, Edges_T& sampleEdges, ArcIntMap& arcIdMap, vector<EdgeSubset>& chances){
    dprintf("Starting SampleWeightedRandom");
    // Our sampling budget
    int budget = (int) ceil(countArcs(g) * samplingProb);
    // The total number of edges
    int edgesCount = countArcs(g);

    // inverted list of original arc ids to arcs in g
    map<int, ListDigraph::Arc> idToArc;
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        idToArc[arcIdMap[arc]] = arc;
    }

    vector<ListDigraph::Arc> toDelete;
    while (budget > 0 && chances.size() > 0){
        int index = (int) floor(nextRand() * chances.size());
        EdgeSubset es = chances[index];
        FOREACH_STL(arcId, es.subset){
            if (!sampleEdges.test(arcId)){
                budget --;
                sampleEdges.set(arcId);
                ListDigraph::Arc arc = idToArc[arcId];
                if (nextRand() <= wMap[arc]){ // sampling coin toss
                    wMap[arc] = 1.0;
                } else {
                    wMap[arc] = 0.0;
                    toDelete.push_back(arc);
                    //g.erase(arc);
                }
                if (budget <= 0)
                    break;
            }
        }END_FOREACH;
        // Here we go forward and backwards from index to remove the selected entries from chances
        int id = chances[index].id;
        int start, end;
        for (start=index; start>=0 && chances[start].id==id; --start);
        start ++;
        for (end=index; end<chances.size() && chances[end].id==id; ++end);
        chances.erase(chances.begin()+start, chances.begin()+end);
    }

    for (ListDigraph::Arc &edge: toDelete) { // for each arc in toDelete
        g.erase(edge);
    }

    if (budget > 0){
        vector<int> remainingEdges;
        for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
            if (!sampleEdges.test(arcIdMap[arc]))
                remainingEdges.push_back(g.id(arc));
        }
        while (budget > 0){
            int index = (int) floor(nextRand() * remainingEdges.size());
            int edge = remainingEdges[index];
            ListDigraph::Arc arc = g.arcFromId(edge);
            int arcId = arcIdMap[arc];
            if (nextRand() <= wMap[arc]){ // sampling coin toss
                wMap[arc] = 1.0;
            } else {
                wMap[arc] = 0.0;
                g.erase(arc);
            }
            sampleEdges.set(arcId);
            remainingEdges.erase(remainingEdges.begin()+index);
            budget --;
        }
    }
}

/*This function performs one sampling iteration, returns the reachability result
** sampleEdges will be used if fixed is set to true
** sampleEdges will be changed to the chosen random set if fixed is false
*/
double iteration(ListDigraph& gOrig, WeightMap& wMapOrig, ArcIntMap& arcIdMapOrig,
                 ListDigraph::Node& sourceOrig, ListDigraph::Node& targetOrig,
                 bool fixed, double samplingProb, Edges_T& sampleEdges, bool print,
                 bool weighted, vector<EdgeSubset>& chances){
    //Initialize a new graph from the original
    ListDigraph g;
    WeightMap wMap(g);
    ArcIntMap arcIdMap(g);
    ListDigraph::Node source;
    ListDigraph::Node target;
    dprintf("Create graph\n");
    digraphCopy(gOrig, g).node(sourceOrig, source).node(targetOrig, target).arcMap(wMapOrig, wMap).arcMap(arcIdMapOrig, arcIdMap).run();

    dprintf("Sampling the graph\n");
    // sample the graph
    if (fixed){
        SampleFixed(g, wMap, source, target, sampleEdges, arcIdMap);
    } else {
        if (weighted){
            SampleWeightedRandom(g, wMap, source, target, samplingProb, sampleEdges, arcIdMap, chances);
        } else {
            SampleRandom(g, wMap, source, target, samplingProb, sampleEdges, arcIdMap);
        }
    }

    dprintf("Minimize graph\n");
    // post-sampling minimization
    minimizeGraph(g, wMap, arcIdMap, source, target);

    int numNodes = countNodes(g);
    int numEdges = countArcs(g);
    if (print) cout << numNodes << "\t" << numEdges << "\t";
    if (numEdges == 0){ // empty graph - source and target unreachable
        return 0.0;
    }

    dprintf("Find good cuts\n");
    vector<Cut> cuts;
    vector< pair< vector<int>, int > > edgeSubsets;
    FindSomeGoodCuts(g, source, target, cuts, edgeSubsets);
    vector<int> edges;
    HorizontalPaths(edges, sourceOrig, cuts[1], gOrig)

    dprintf("Solve graph\n");
    double prob = Solve(g, wMap, source, target, cuts);
    return prob;
}
