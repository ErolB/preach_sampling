#include <stdio.h>
#ifdef DEBUG
#define dprintf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define dprintf(fmt, ...)
#endif

#ifndef PREACH_SAMPLING_CUT_H
#include "preach.h"

const string SOURCE = "SOURCE";
const string SINK = "SINK";

using namespace std;
using lemon::ListDigraph;
using lemon::INVALID;
using lemon::Bfs;

typedef ListDigraph::ArcMap<double> WeightMap;
typedef ListDigraph::NodeMap<string> NodeNames;
typedef map<string, ListDigraph::Node> NameToNode;
typedef ListDigraph::ArcMap<int> ArcIntMap;

string output_path = "/home/erol/Documents/preach_sampling/data.txt";  // path of output file
char graph_file[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_4_85_2.txt";  // path of graph file
char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_4_85_2.sources"; // path of sources file
char targetsFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_4_85_2.targets";  // path of targets file

////////////////////////////////////////////////////////////////////////// read a list from a file

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

///////////////////////////////////////////////////////////////////////// check if edge exists

bool EdgeExists(ListDigraph& g, ListDigraph::Node& source, ListDigraph::Node& target){
    for (ListDigraph::OutArcIt arc(g, source); arc != INVALID; ++arc){
        if (g.id(g.target(arc)) == g.id(target))
            return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////// map names to nodes

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

///////////////////////////////////////////////////////////// Create the graph from the file

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
//////////////////////////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////// removes cuts that are masked by smaller cuts

void RemoveRedundantCuts(vector<Cut>& cuts){
    for (size_t i=0; i<cuts.size(); i++){
        Cut currenti = cuts.at(i);
        for (size_t j=i+1; j<cuts.size(); j++){
            Cut currentj = cuts.at(j);
            // see if one of them is contained in the other
            if ((~currenti.getMiddle() & currentj.getMiddle()).none()){
                // j contained in i: delete i and break
                cuts.erase(cuts.begin() + i);
                i--;
                break;
            }
            if ((~currentj.getMiddle() & currenti.getMiddle()).none()){
                //i contained in j: delete j
                cuts.erase(cuts.begin() + j);
                j--;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
/*Minimizes the cuts, then makes sure they are "Good"
 It also returns edge subsets which are added together to a cut to make it "good",
 along with the size of that cut
 This is done for the purpose of weighted random probing
*/
void RefineCuts(vector<Cut>& cuts, ListDigraph& g, ListDigraph::Node& target, vector< pair< vector<int>, int> > & edgeSubsets){
    // check for non-minimality: containment of cuts in other cuts
    //RemoveRedundantCuts(cuts);

    // grow each cut (if necessary) into a good cut
    vector<Cut> goodCuts;
    FOREACH_STL(cut, cuts){
            Nodes_T right = cut.getRight();
            Nodes_T middle = cut.getMiddle();
            Edges_T covered = cut.getCoveredEdges();
            int cutsize = middle.count();
            // repeat until nothing changes
            while(true){
                // for each node on the right, make sure its outgoing neighbors are all on the right also
                vector<int> toAdd;
                FOREACH_BS(nodeId, right){
                    ListDigraph::Node node = g.nodeFromId(nodeId);
                    vector<int> backedges;
                    for (ListDigraph::OutArcIt arc(g, node); arc != INVALID; ++arc){
                        if (!right[g.id(g.target(arc))]){ // back edge
                            backedges.push_back(g.id(arc));
                        }
                    }
                    if (backedges.size() > 0){ // some backedges are there
                        toAdd.push_back(nodeId);
                        pair<vector<int>, int> subset = make_pair(backedges, cutsize);
                        edgeSubsets.push_back(subset);
                    }
                }
                if (toAdd.size() == 0)
                    break;
                FOREACH_STL(nodeId, toAdd){
                        right.reset(nodeId);
                        middle.set(nodeId);
                    }END_FOREACH;
            }
            //Now some new edges can be covered due to moving nodes to the middle
            //mark these edges as covered
            FOREACH_BS(nodeId, middle){
                ListDigraph::Node middleNode = g.nodeFromId(nodeId);
                for (ListDigraph::OutArcIt arc(g, middleNode); arc != INVALID; ++arc){
                    if (!right[g.id(g.target(arc))]){
                        covered.set(g.id(arc));
                    }
                }
            }
            //add the new good cut
            goodCuts.push_back(Cut(cut.getLeft(), middle, right, covered));
        }END_FOREACH;
    cuts = goodCuts;

    // Minimize good cuts:
    // If a node in the middle group has no outgoing edges to the right group
    // Then move it to the left group
    vector<Cut> bestCuts;
    FOREACH_STL(cut, cuts){
            Nodes_T middle = cut.getMiddle();
            Nodes_T left = cut.getLeft();
            Nodes_T right = cut.getRight();
            vector<int> toMove;
            FOREACH_BS(nodeId, middle){
                // make sure it has at least one edge to the right
                bool hasRight = false;
                for (ListDigraph::OutArcIt arc(g, g.nodeFromId(nodeId)); arc != INVALID; ++arc){
                    if (right[g.id(g.target(arc))]){
                        hasRight = true;
                        break;
                    }
                }
                if (!hasRight){
                    toMove.push_back(nodeId);
                }
            }
            FOREACH_STL(nodeId, toMove){
                    middle.reset(nodeId);
                    left.set(nodeId);
                }END_FOREACH;
            bestCuts.push_back(Cut(left, middle, right, cut.getCoveredEdges()));
        }END_FOREACH;
    cuts = bestCuts;

    // Last: remove redundant cuts again
    RemoveRedundantCuts(cuts);
}

/////////////////////////////////////////////////// creates first level cut: nodes adjacent to source

Cut createFirstCut(ListDigraph& g, ListDigraph::Node source, ListDigraph::Node target){
    Nodes_T left;
    Nodes_T middle;
    Nodes_T right;
    Edges_T covered;
    // initially all nodes are on the right
    for (ListDigraph::NodeIt node(g); node != INVALID; ++node){
        right.set(g.id(node));
    }
    //move source from right to left
    left.set(g.id(source));
    right.reset(g.id(source));
    //move nodes adjacent to source from right to middle
    //and mark covered edges in the process
    for (ListDigraph::OutArcIt arc(g, source); arc != INVALID; ++arc){
        int endId = g.id(g.target(arc));
        if (endId == g.id(target)){
            return Cut();
        }
        covered.set(g.id(arc));
        middle.set(endId);
        right.reset(endId);
    }
    // mark as covered: the edges from the middle not going to the right
    FOREACH_BS(nodeId, middle){
        ListDigraph::Node node = g.nodeFromId(nodeId);
        for (ListDigraph::OutArcIt arc(g, node); arc != INVALID; ++arc){
            if (!right[g.id(g.target(arc))]){
                covered.set(g.id(arc));
            }
        }
    }

    // create the cut and recurse
    return Cut(left, middle, right, covered);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Collects edge subsets from cuts
   An edge subset is all edges coming into a cut member from the left
*/
void FillEdgeSubsets(ListDigraph& g, vector<Cut>& cuts, vector< pair< vector<int>, int > > & edgeSubsets){
    FOREACH_STL(cut, cuts){
            Nodes_T middle = cut.getMiddle();
            Nodes_T left = cut.getLeft();
            FOREACH_BS(nodeId, middle){
                ListDigraph::Node node = g.nodeFromId(nodeId);
                vector<int> incoming;
                for (ListDigraph::InArcIt arc(g, node); arc != INVALID; ++arc){
                    if (left.test(g.id(g.source(arc))))
                        incoming.push_back(g.id(arc));
                }
                if (incoming.size() > 0){
                    pair< vector<int>, int > subset = make_pair(incoming, middle.count());
                    edgeSubsets.push_back(subset);
                }
            }
        }END_FOREACH;
}

///////////////////////////////////////////////////////////////////////////////////////// finds node separators

void FindSomeGoodCuts(ListDigraph& g, ListDigraph::Node source, ListDigraph::Node target, vector<Cut>& cuts, vector< pair< vector<int>, int > > & edgeSubsets) {
    //start by forming the first cut: adjacent to source
    Cut firstCut = createFirstCut(g, source, target);
    if (firstCut.getMiddle().none()) { // That was a dummy returned cut, i.e. no cuts available
        return;
    }
    Nodes_T currentMiddle = firstCut.getMiddle();
    Nodes_T currentLeft = firstCut.getLeft();
    Nodes_T currentRight = firstCut.getRight();
    Edges_T currentCovered = firstCut.getCoveredEdges();
    cuts.push_back(firstCut);
    bool added = true;
    while (added) { // repeat until nothing new is added
        Nodes_T middle = currentMiddle;
        Nodes_T left = currentLeft;
        Nodes_T right = currentRight;
        Edges_T covered = currentCovered;
        added = false;
        FOREACH_BS(nodeId, currentMiddle) {
            ListDigraph::Node node = g.nodeFromId(nodeId);
            vector<int> nextNodes;
            vector<int> nextArcs;
            for (ListDigraph::OutArcIt arc(g, node); arc != INVALID; ++arc) {
                ListDigraph::Node next = g.target(arc);
                int nextId = g.id(next);
                if (nextId == g.id(target)) { // node connected to target, ignore all of its neighbors
                    nextNodes.clear();
                    nextArcs.clear();
                    break;
                } else if (right[nextId]) { // eligible for moving from right to middle
                    nextNodes.push_back(nextId);
                    nextArcs.push_back(g.id(arc));
                }
            }
            if (nextNodes.size() > 0) { // There are nodes to move from right to left
                added = true;
                FOREACH_STL(nextId, nextNodes)
                    {
                        right.reset(nextId);
                        middle.set(nextId);
                    }
                END_FOREACH;
                FOREACH_STL(nextId, nextArcs)
                    {
                        covered.set(nextId);
                    }
                END_FOREACH;
                middle.reset(nodeId);
                left.set(nodeId);
            }
        }
        if (added) {
            // mark as covered: all edges going from the middle not to the right
            FOREACH_BS(nodeId, middle) {
                ListDigraph::Node middleNode = g.nodeFromId(nodeId);
                for (ListDigraph::OutArcIt arc(g, middleNode); arc != INVALID; ++arc) {
                    if (!right[g.id(g.target(arc))]) {
                        covered.set(g.id(arc));
                    }
                }
            }
            Cut newCut(left, middle, right, covered);
            cuts.push_back(newCut);
            currentMiddle = middle;
            currentLeft = left;
            currentRight = right;
            currentCovered = covered;
        }
    }

    FillEdgeSubsets(g, cuts, edgeSubsets);
    RefineCuts(cuts, g, target, edgeSubsets);
}

// returns a vector of the indicies of all ones
vector<int> cvtBitset(Nodes_T input){
    vector<int> positions;
    for (int i = 0; i < input.size(); i++){
        if (input[i]){
            positions.push_back(i);
        }
    }
    return positions;
}

////////////////////////////////////////////////////////////////////////////////////////////////////// main function

int main(){
    ListDigraph gOrig;
    WeightMap wMapOrig(gOrig); // keeps track of weights
    NodeNames nNames(gOrig); // node names
    NameToNode nodeMap; // mapping from names to nodes in the graph
    ArcIntMap arcIdMapOrig(gOrig); // mapping from the arcs to their ids in the original graph

    CreateGraph(graph_file, gOrig, nNames, nodeMap, wMapOrig, arcIdMapOrig);
    UnifyTerminals(gOrig, wMapOrig, nNames, nodeMap, arcIdMapOrig, sourcesFile, targetsFile);
    ListDigraph::Node sourceOrig = FindNode(SOURCE, gOrig, nNames, nodeMap);
    ListDigraph::Node targetOrig = FindNode(SINK, gOrig, nNames, nodeMap);

    vector<Cut> cuts;
    // assigns whole numbers to nodes on cuts and fractional values to those between cuts
    vector< pair< vector<int>, double > > output;
    vector< pair< vector<int>, int > > edgeSubsets;
    FindSomeGoodCuts(gOrig, sourceOrig, targetOrig, cuts, edgeSubsets);
    for (int i = 1; i <= cuts.size(); i++){
        vector<int> middle = cvtBitset(cuts[i-1].getMiddle());
        output.push_back(make_pair(middle, i));
        vector<int> next = cvtBitset(cuts[i-1].getRight() & cuts[i].getLeft());
        output.push_back(make_pair(next, (double) i + 0.5));
    }

    // write data to file
    // values are comma separated
    ofstream data_file;
    data_file.open(output_path);
    if (data_file.is_open()) {
        for (int i = 0; i < output.size(); i++){
            for (int j = 0; j < output[i].first.size(); j++) {
                data_file << output[i].first[j]+1 << "," << output[i].second << endl;
            }
        }
    }
    data_file.close();

}