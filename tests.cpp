#include "/home/erol/Documents/googletest/include/gtest/gtest.h"
#include "Probing.h"  // includes all other PReach files

bool verifyCuts(vector<Cut> cuts, ListDigraph& g){
    for(Cut cut: cuts){
        // generate vectors representing left, middle, and right nodes
        vector<int> left = cvtBitset(cut.getLeft());
        vector<int> middle = cvtBitset(cut.getMiddle());
        vector<int> right = cvtBitset(cut.getRight());
        // verify that no edges cross the cut
        for(int node_id: left) {  // iterate through nodes on the left
            // check if outgoing edges lead to nodes on the right
            for (ListDigraph::OutArcIt o(g, g.nodeFromId(node_id)); o != INVALID; ++o) {
                int target_id = g.id(g.target(o));  // finds the ID number of the target node
                // check if node is on the right
                for (int test_node: right) {
                    if (target_id == test_node) {
                        cout << "1" << endl;
                        return false;  // an edge crossed the cut, so it is invalid (no more computation needed)
                    }
                }
            }
            // check if incoming edges come from nodes on the right
            for (ListDigraph::InArcIt i(g, g.nodeFromId(node_id)); i != INVALID; ++i) {
                int origin_id = g.id(g.source(i));
                // check if node is on the right
                for (int test_node: right) {
                    if (origin_id == test_node){
                        cout << origin_id << " " << node_id << endl;
                        return false;  // no more computation needed
                    }
                }
            }
        }
        // check for edges from the right to the middle
        for (int node_id: middle){
            for (ListDigraph::InArcIt i(g, g.nodeFromId(node_id)); i != INVALID; ++i){
                int origin_id = g.id(g.source(i));
                // check if node is on the right
                for (int test_node: right){
                    if (origin_id == test_node){
                        cout << "3" << endl;
                        return false;
                    }
                }
            }
        }
    }
    // if the function reaches this point, then the test has been passed
    return true;
}


TEST(CutValididtyTest, standard){
    char graph_file[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.txt";
    char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.sources";
    char targetsFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.targets";

    ListDigraph gOrig;
    NameToNode nodeMap; // mapping from names to nodes in the graph
    ListDigraph::Node sourceOrig; // start node
    ListDigraph::Node targetOrig; // target node
    WeightMap wMapOrig(gOrig); // keeps track of weights
    NodeNames nNames(gOrig); // node names
    ArcIntMap arcIdMapOrig(gOrig); // mapping from the arcs to their ids in the original graph

    CreateGraph(graph_file, gOrig, nNames, nodeMap, wMapOrig, arcIdMapOrig);
    UnifyTerminals(gOrig, wMapOrig, nNames, nodeMap, arcIdMapOrig, sourcesFile, targetsFile);
    sourceOrig = FindNode(SOURCE, gOrig, nNames, nodeMap);
    targetOrig = FindNode(SINK, gOrig, nNames, nodeMap);

    vector<Cut> cuts;
    // assigns a range of cuts to each node
    vector<pair<int, vector<int> > > output;
    vector<pair<vector<int>, int> > edgeSubsets;
    FindSomeGoodCuts(gOrig, sourceOrig, targetOrig, cuts, edgeSubsets);

    EXPECT_TRUE(verifyCuts(cuts, gOrig));
}

int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

}
