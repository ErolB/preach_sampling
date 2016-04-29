/** Define debugging macro */

#include <stdio.h>
#ifdef DEBUG
#define dprintf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define dprintf(fmt, ...)
#endif

#include "preach.h"
#include "preprocess.h"
#include "probing.h"
#include "graph_calc.h"

using namespace std;
using lemon::ListDigraph;
using lemon::INVALID;
using lemon::Bfs;

typedef ListDigraph::ArcMap<double> WeightMap;
typedef ListDigraph::NodeMap<string> NodeNames;
typedef map<string, ListDigraph::Node> NameToNode;
typedef ListDigraph::ArcMap<int> ArcIntMap;

const string SOURCE = "SOURCE";
const string SINK = "SINK";
const string PRE_YES = "pre";
const string PRE_NO = "nopre";

const string SAMPLING_RANDOM = "rand";
const string SAMPLING_FIXED_RANDOM = "fixrand";
const string SAMPLING_FIXED_WEIGHTED_RANDOM = "fixwrand";



/*Prints the graph in node IDs*/
void PrintGraph(ListDigraph& g){
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        cout << g.id(g.source(arc)) << " " << g.id(g.target(arc)) << endl;
    }
}

/*Starting from a vertex cut, recursively finds all other cuts to the right*/
void FindAllCuts(Cut& currentCut, vector<Cut>& cuts,  ListDigraph& g, ListDigraph::Node target){
    Nodes_T currentMiddle = currentCut.getMiddle();
    Nodes_T currentLeft = currentCut.getLeft();
    Nodes_T currentRight = currentCut.getRight();
    Edges_T currentCovered = currentCut.getCoveredEdges();

    // Go through all nodes in the cut, try to replace it with neighbors
    FOREACH_BS(nodeId, currentMiddle){
        Nodes_T middle = currentMiddle;
        Nodes_T left = currentLeft;
        Nodes_T right = currentRight;
        Edges_T covered = currentCovered;
        ListDigraph::Node node = g.nodeFromId(nodeId);
		// Loop over all neighbors of the node
		bool added = false;
        for (ListDigraph::OutArcIt arc(g, node); arc != INVALID; ++arc){
            ListDigraph::Node next = g.target(arc);
            int nextId = g.id(next);
            if (nextId == g.id(target)){ // cut connected to target, STOP HERE
                added = false;
                break;
            }else if (right[nextId]){ // add from right to middle
                added = true;
                right.reset(nextId);
                middle.set(nextId);
                covered.set(g.id(arc));
            }
        }
        if (added){ // added at least one node to the cut
            middle.reset(nodeId);
            left.set(nodeId);
            // mark as covered: all edges going from the middle not to the right
            FOREACH_BS(nodeId, middle){
                ListDigraph::Node middleNode = g.nodeFromId(nodeId);
                for (ListDigraph::OutArcIt arc(g, middleNode); arc != INVALID; ++arc){
                    if (!right[g.id(g.target(arc))]){
                        covered.set(g.id(arc));
                    }
                }
            }
            Cut newCut(left, middle, right, covered);
            cuts.push_back(newCut);
            FindAllCuts(newCut, cuts, g, target);
        }
    }
}

/*prints a cut*/
void PrintCut(Cut& cut, ListDigraph& g){
    Nodes_T nodes = cut.getMiddle();
    FOREACH_BS(id, nodes){
        cout << id << " ";
    }
    cout << " : ";
    Edges_T covered = cut.getCoveredEdges();
    FOREACH_BS(id, covered){
        cout << g.id(g.source(g.arcFromId(id))) << "-" << g.id(g.target(g.arcFromId(id))) << " ";
    }
    cout << endl;
}

/*prints cuts*/
void PrintCuts(vector<Cut>& cuts, ListDigraph& g){
    FOREACH_STL(cut, cuts){
        PrintCut(cut, g);
    }END_FOREACH;
}

/*Gets all edges as a bitset*/
void EdgesAsBitset(ListDigraph& g, Edges_T& edges){
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        edges.set(g.id(arc));
    }
}

/*Just for debugging - ignore*/
bool compareCuts(Cut cut1, Cut cut2){
    return (cut1.getMiddle().count() < cut2.getMiddle().count());
}

string joinString(vector<string>& parts, string delim){
    stringstream ss;
    for(size_t i = 0; i < parts.size(); ++i)
    {
      if(i != 0)
        ss << delim;
      ss << parts[i];
    }
    return ss.str();
}

void splitString(string str, vector<string>& result, char delim){
    istringstream stream(str);
    while (!stream.eof()){
      string part;
      getline(stream, part, delim);
      result.push_back(part);
    }
}

string arcToString(ListDigraph& g, WeightMap& wMap, NodeNames& nNames, ListDigraph::Arc& arc){
    stringstream ss;
    ss << nNames[g.source(arc)];
    ss << ">";
    ss << nNames[g.target(arc)];
    ss << ">";
    stringstream ws;
    ws << wMap[arc];
    string wString = ws.str();
    if (wString.length() > 6){
        wString.resize(6);
    }
    ss << wString;
    return ss.str();
}

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

bool CheckProcessedReference(ListDigraph& g, WeightMap& wMap, NodeNames& nNames, string reference){
    vector<string> edges;
    // The reference string has to have leading and trailing quotes
    //reference.erase(0,1);
    //reference.erase(reference.length()-1, 1);
    splitString(reference, edges, '#');
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        string arcString = arcToString(g, wMap, nNames, arc);
        vector<string>::iterator it = find(edges.begin(), edges.end(), arcString);
        if (it == edges.end()){
            return false;
        } else{
            edges.erase(it);
        }
    }
    if (edges.size() == 0)
        return true;
    else{
        return false;
    }
}

void minimizeGraph(ListDigraph& g, WeightMap& wMap, ArcIntMap& arcIdMap, ListDigraph::Node& source, ListDigraph::Node& target){
    RemoveIsolatedNodes(g, source, target);
    CollapseELementaryPaths(g, wMap, arcIdMap, source, target);
    RemoveSelfCycles(g);
}

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
	    return 0;
    }

	ListDigraph::Node sourceOrig = FindNode(SOURCE, gOrig, nNames, nodeMap);
	ListDigraph::Node targetOrig = FindNode(SINK, gOrig, nNames, nodeMap);

    Edges_T sampleEdges;
    double samplingProb = atof(argv[4]);
    bool fixedSample = false;
    bool weighted = false;
    vector<EdgeSubset> chances;
    if (argv[6] == SAMPLING_FIXED_RANDOM || argv[6] == SAMPLING_FIXED_WEIGHTED_RANDOM){
        int probeSize = atoi(argv[7]);
        int probeRepeats = atoi(argv[8]);
        fixedSample = true;
        weighted = (argv[6] == SAMPLING_FIXED_WEIGHTED_RANDOM);
        cout << "#Probing for " << probeSize << " times";
        cout.flush();
        ProbeRandom(gOrig, wMapOrig, arcIdMapOrig, sourceOrig, targetOrig, samplingProb, sampleEdges, probeSize, probeRepeats, weighted);
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
    return 0;
}
