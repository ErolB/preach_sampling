#include <stdio.h>
#ifdef DEBUG
#define dprintf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define dprintf(fmt, ...)
#endif

#include "Cut.h"

string output_path = "/home/erol/Documents/preach_sampling/data.txt";  // path of output file
char graph_file[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_4_85_2.txt";  // path of graph file
char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_4_85_2.sources"; // path of sources file
char targetsFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_4_85_2.targets";  // path of targets file

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
    // assigns a range of cuts to each node
    vector< pair< int, vector< int > > > output;
    vector< pair< vector< int >, int > > edgeSubsets;
    FindSomeGoodCuts(gOrig, sourceOrig, targetOrig, cuts, edgeSubsets);
    // iterate over every node
    for (ListDigraph::NodeIt n(gOrig); n != INVALID; ++n) {
        int current_node = gOrig.id(n);
        vector<int> cut_range;  // cuts that the node is a part of
        //iterate over every cut
        for (int i = 1; i <= cuts.size(); ++i) {
            vector<int> middle = cvtBitset(cuts[i - 1].getMiddle());  // nodes on current cut
            // iterate over every node on the cut
            for (int j = 0; j <= middle.size(); ++j) {
                // check if current node is on the cut
                if (current_node == middle[j]) {
                    cut_range.push_back(i);
                }
            }
        }
        output.push_back(make_pair(current_node, cut_range));
    }

    // write data to file
    // values are comma separated
    ofstream data_file;
    data_file.open(output_path);
    if (data_file.is_open()) {
        for (int i = 0; i < output.size(); i++){
            if (output[i].second.size() > 0) {
                data_file << output[i].first;
                for (int j = 0; j < output[i].second.size(); j++) {
                    data_file << ',' << output[i].second[j] + 1;
                }
                data_file << endl;
            }
        }
    }
    data_file.close();

}