/** Define debugging macro */

#include <stdio.h>
#ifdef DEBUG
#define dprintf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define dprintf(fmt, ...)
#endif

/*
#include "preach.h"
#include "Graph.h"
#include "Cut.h"
#include "Sampling.h"
 */
#include "Sampling.h"

string edgesToReferenceString(ListDigraph& g, WeightMap& wMap, NodeNames& nNames){
    vector<string> edges;
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        string arcString = arcToString(g, wMap, nNames, arc);
        edges.push_back(arcString);
    }
    stringstream ss;
    ss << "\"";
    ss << joinString(edges, "#");
    ss << "\"";
    return ss.str();
}

int main(int argc, char** argv)
{
    if (argc < 9) {
		// arg1: network file
		// arg2: sources file
		// arg3: targets file
		// arg4: sampled portion
		// arg5: Number of samples
		// arg6: sampling method
		// arg7: probe size (if any)
		// arg8: probe repeats (if any)
		cout << "ERROR - Usage: preach graph-file sources-file targets-file sampled-portion sample-size sampling-method" << endl;
		return -1;
	}

	// initialize srand so we get random graphs
    //timeval time;
    //gettimeofday(&time,NULL);
    //srand48((time.tv_sec * 1000) + (time.tv_usec / 1000));
    initRand();

    ListDigraph gOrig;
	WeightMap wMapOrig(gOrig); // keeps track of weights
	NodeNames nNames(gOrig); // node names
	NameToNode nodeMap; // mapping from names to nodes in the graph
	ArcIntMap arcIdMapOrig(gOrig); // mapping from the arcs to their ids in the original graph

	CreateGraph(argv[1], gOrig, nNames, nodeMap, wMapOrig, arcIdMapOrig);
	int numNodes = countNodes(gOrig);
	int numEdges = countArcs(gOrig);
	cout << "#Original graph size: " << numNodes << " nodes, " << numEdges << " edges" << endl;
	//cout << numNodes << "  " << numEdges << "  ";

	// Read sources and targets and preprocess
	Preprocess(gOrig, wMapOrig, nNames, nodeMap, arcIdMapOrig, argv[2], argv[3], PRE_YES);

	numNodes = countNodes(gOrig);
	numEdges = countArcs(gOrig);
	cout << "#Modified graph size: " << numNodes << " nodes, " << numEdges << " edges" << endl;
	//cout << numNodes << "  " << numEdges << "  ";

	if (numEdges == 0){ // empty graph - source and target unreachable
	    //cout << ">>0.0" << endl;
	    cout << "#>>result = 0.0" << endl;
        cout << "@@@";
	    return 0;
    }

	ListDigraph::Node sourceOrig = FindNode(SOURCE, gOrig, nNames, nodeMap);
	ListDigraph::Node targetOrig = FindNode(SINK, gOrig, nNames, nodeMap);

    Edges_T sampleEdges;
    double samplingProb = atof(argv[4]);
    bool fixedSample = false;
    bool weighted = false;
    vector<EdgeSubset> chances;
    if (argv[6] == SAMPLING_FIXED_RANDOM || argv[6] == SAMPLING_FIXED_WEIGHTED_RANDOM) {
        int probeSize = atoi(argv[7]);
        int probeRepeats = atoi(argv[8]);
        fixedSample = true;
        weighted = (argv[6] == SAMPLING_FIXED_WEIGHTED_RANDOM);
        cout << "#Probing for " << probeSize << " times";
        cout.flush();
        //ProbeRandom(gOrig, wMapOrig, arcIdMapOrig, sourceOrig, targetOrig, samplingProb, sampleEdges, probeSize,
        //            probeRepeats, weighted);
        cout << endl;
        //cout << "#Fixed sample = " << sampleEdges.to_string<char,std::string::traits_type,std::string::allocator_type>() << endl;
    }


    double total = 0.0;
    int sampleSize = atoi(argv[5]);
    cout << "#V\tE\tP\tT" << endl;

    dprintf("Starting sampling loop\n");
    for (int i=0; i<sampleSize; i++){
        double startCPUTime = getCPUTime();
        double prob = iteration(gOrig, wMapOrig, arcIdMapOrig, sourceOrig, targetOrig, fixedSample, samplingProb, sampleEdges, true, weighted, chances);
        double duration = getCPUTime() - startCPUTime;
        cout << prob << "\t" << duration << endl;
        total += prob;
	dprintf("Finished iteration %d\n",i);
    }
    cout << "#>>result = " << total/sampleSize << endl;
    cout << "@@@" << endl;  // indicates that the program has completed
    return 0;
}
