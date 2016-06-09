#include <stdio.h>
#define DEBUG
#ifdef DEBUG
bool print_cuts = true;
#define dprintf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
bool print_cuts = false;
#define dprintf(fmt, ...)
#endif

#include "Cut.h"

string output_path = "/home/erol/Documents/preach_sampling/data.txt";  // path of output file

////////////////////////////////////////////////////////////////////////////////////////////////////// main function

int main(int argc, char** argv) {
    ListDigraph gOrig;
    NameToNode nodeMap; // mapping from names to nodes in the graph
    ListDigraph::Node sourceOrig; // start node
    ListDigraph::Node targetOrig; // target node

    try {
        char* graph_file = argv[1];  // path of graph file
        char* sourcesFile = argv[2]; // path of sources file
        char* targetsFile = argv[3];  // path of targets file

        WeightMap wMapOrig(gOrig); // keeps track of weights
        NodeNames nNames(gOrig); // node names
        ArcIntMap arcIdMapOrig(gOrig); // mapping from the arcs to their ids in the original graph

        CreateGraph(graph_file, gOrig, nNames, nodeMap, wMapOrig, arcIdMapOrig);
        UnifyTerminals(gOrig, wMapOrig, nNames, nodeMap, arcIdMapOrig, sourcesFile, targetsFile);
        sourceOrig = FindNode(SOURCE, gOrig, nNames, nodeMap);
        targetOrig = FindNode(SINK, gOrig, nNames, nodeMap);
    } catch (runtime_error) {
        cout << "ERROR: setup failed";
        exit(1);
    }

    vector<Cut> cuts;
    // assigns a range of cuts to each node
    vector<pair<int, vector<int> > > output;
    vector<pair<vector<int>, int> > edgeSubsets;
    FindSomeGoodCuts(gOrig, sourceOrig, targetOrig, cuts, edgeSubsets);
    // iterate over every node
    for (ListDigraph::NodeIt n(gOrig); n != INVALID; ++n) {
        int current_node = gOrig.id(n);  // index must be shifted
        vector<int> cut_range;  // cuts that the node is a part of
        //iterate over every cut
        for (int i = 0; i < cuts.size(); i++) {
            vector<int> middle = cvtBitset(cuts[i].getMiddle());  // nodes on current cut
            // iterate over every node on the cut
            for (int j = 0; j < middle.size(); j++) {
                // check if current node is on the cut
                if (current_node == middle[j]) {
                    cut_range.push_back(i+1);
                }
            }
        }
        output.push_back(make_pair(current_node, cut_range));
    }

    // print out cuts
    cout << cuts.size() << endl;
    if (print_cuts) {
        for (int i = 0; i < cuts.size(); i++) {
            cout << (i+1) << ": ";  // convert to 1-based indexing
            vector<int> middle = cvtBitset(cuts[i].getMiddle());
            for (int j = 0; j < middle.size(); ++j) {
                cout << middle[j] << " ";
            }
            cout << endl;
        }
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
                    data_file << ',' << output[i].second[j];
                }
                data_file << endl;
            }
        }
    }
    data_file.close();

}
