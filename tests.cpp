<<<<<<< HEAD
#include <gtest/gtest.h>
#include <fstream>
=======
#include "/home/erol/Documents/googletest/include/gtest/gtest.h"
>>>>>>> 1b214c50b126cf3272ce1ef8589f958efaa945d9
#include "Probing.h"  // includes all other PReach files

// verify that graph was properly created

bool createGraphTest(char graph_file[]){
    // create graph structure
    ListDigraph g;
    NameToNode nodeMap; // mapping from names to nodes in the graph
    ListDigraph::Node source; // start node
    ListDigraph::Node target; // target node
    WeightMap wMap(g); // keeps track of weights
    NodeNames nNames(g); // node names
    ArcIntMap arcIdMap(g); // mapping from the arcs to their ids in the original graph
    CreateGraph(graph_file, g, nNames, nodeMap, wMap, arcIdMap);
    // check if all edges are present
    vector<int> expected_nodes;
    fstream file_obj(graph_file);
    while (!file_obj.eof()){
        bool flag;
        int start = 0;
        int end = 0;
        double prob;
        file_obj >> start >> end >> prob;
        if (start == 0){ continue; }
        start--;
        end--;
        // iterate through all outgoing edges to determine if the edge from the file is present
        flag = false;
        for (ListDigraph::OutArcIt o(g, g.nodeFromId(start)); o != INVALID; ++o){
            int target = g.id(g.target(o));
            int expected_target = g.id(g.nodeFromId(end));  // not ideal, I know
            if (target == expected_target){
                flag = true;
                break;
                // edge is present
            }
        }
        // exit and return false if edge was not found
        if (!flag){
            cout << "Edge not present: " << start << " to " << end << endl;
            return false;
        }
    }
    // if this point is reached, then the graph object is correct
    return true;
}

// verify that cuts are valid

bool verifyCuts(vector<Cut> cuts, ListDigraph& g){
    cout << cuts.size();
    for(Cut cut: cuts){
        // generate vectors representing left, middle, and right nodes
        vector<int> left = cvtBitset(cut.getLeft());
        vector<int> middle = cvtBitset(cut.getMiddle());
        vector<int> right = cvtBitset(cut.getRight());
    	// display cut details
    	//cout << "CUT DETAILS" << endl;
    	//cout << "left: ";
    	//for (int node: left){
    	  //cout << node << ",";
    	//}
    	//cout << endl << "middle: " << endl;
    	//for (int node: middle){
    	//  cout << node << ",";
    	//}
    	//cout << endl << "right: " << endl;
    	//for (int node: right){
    	//  cout << node << ",";
    	//}
    	//cout << endl;
        // verify that no edges cross the cut
        for(int node_id: left) {  // iterate through nodes on the left
            // check if outgoing edges lead to nodes on the right
            for (ListDigraph::OutArcIt o(g, g.nodeFromId(node_id)); o != INVALID; ++o) {
                int target_id = g.id(g.target(o));  // finds the ID number of the target node
                // check if node is on the right
                for (int test_node: right) {
                    if (target_id == test_node) {
<<<<<<< HEAD
		      //cout << "left to right" << endl;
		      return false;  // an edge crossed the cut, so it is invalid (no more computation needed)
=======
                        cout << "1" << endl;
                        return false;  // an edge crossed the cut, so it is invalid (no more computation needed)
>>>>>>> 1b214c50b126cf3272ce1ef8589f958efaa945d9
                    }
                }
            }
            // check if incoming edges come from nodes on the right
            for (ListDigraph::InArcIt i(g, g.nodeFromId(node_id)); i != INVALID; ++i) {
                int origin_id = g.id(g.source(i));
                // check if node is on the right
                for (int test_node: right) {
                    if (origin_id == test_node){
<<<<<<< HEAD
		      //cout << "right to left" << endl;
		      //cout << origin_id << "-->" << node_id << endl;  // for debugging
		      return false;  // no more computation needed
=======
                        cout << origin_id << " " << node_id << endl;
                        return false;  // no more computation needed
>>>>>>> 1b214c50b126cf3272ce1ef8589f958efaa945d9
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
	    //cout << "valid cut" << endl;
    }
    // if the function reaches this point, then the test has been passed
    return true;
}

<<<<<<< HEAD
=======

TEST(CutValididtyTest, standard){
    char graph_file[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.txt";
    char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.sources";
    char targetsFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.targets";
>>>>>>> 1b214c50b126cf3272ce1ef8589f958efaa945d9

bool testCuts(char graphFile[], char sourcesFile[], char targetsFile[]){
    ListDigraph gOrig;
    NameToNode nodeMap; // mapping from names to nodes in the graph
    ListDigraph::Node sourceOrig; // start node
    ListDigraph::Node targetOrig; // target node
    WeightMap wMapOrig(gOrig); // keeps track of weights
    NodeNames nNames(gOrig); // node names
    ArcIntMap arcIdMapOrig(gOrig); // mapping from the arcs to their ids in the original graph

    CreateGraph(graphFile, gOrig, nNames, nodeMap, wMapOrig, arcIdMapOrig);
    UnifyTerminals(gOrig, wMapOrig, nNames, nodeMap, arcIdMapOrig, sourcesFile, targetsFile);
    sourceOrig = FindNode(SOURCE, gOrig, nNames, nodeMap);
    targetOrig = FindNode(SINK, gOrig, nNames, nodeMap);

    vector<Cut> cuts;
    // assigns a range of cuts to each node
    vector<pair<int, vector<int> > > output;
    vector<pair<vector<int>, int> > edgeSubsets;
    FindSomeGoodCuts(gOrig, sourceOrig, targetOrig, cuts, edgeSubsets);

    // print out cuts (for debugging)
    //for(int i = 0; i < cuts.size(); i++){
    //  cout << cuts[i].getMiddle() << endl;
    //}

    return verifyCuts(cuts, gOrig);
}

// test solve function
bool testSolve(char graphFile[], char sourcesFile[], char targetsFile[]){
    ListDigraph gOrig;
    NameToNode nodeMap; // mapping from names to nodes in the graph
    ListDigraph::Node sourceOrig; // start node
    ListDigraph::Node targetOrig; // target node
    WeightMap wMapOrig(gOrig); // keeps track of weights
    NodeNames nNames(gOrig); // node names
    ArcIntMap arcIdMapOrig(gOrig); // mapping from the arcs to their ids in the original graph

    CreateGraph(graphFile, gOrig, nNames, nodeMap, wMapOrig, arcIdMapOrig);
    UnifyTerminals(gOrig, wMapOrig, nNames, nodeMap, arcIdMapOrig, sourcesFile, targetsFile);
    sourceOrig = FindNode(SOURCE, gOrig, nNames, nodeMap);
    targetOrig = FindNode(SINK, gOrig, nNames, nodeMap);

    vector<Cut> cuts;
    vector<pair<vector<int>, int> > edgeSubsets;
    FindSomeGoodCuts(gOrig, sourceOrig, targetOrig, cuts, edgeSubsets);

    double probability = Solve(gOrig, wMapOrig, sourceOrig, targetOrig, cuts);
    //cout << "Probability: " << probability << endl;
    return probability;
}

bool testCollapsation(char graph_file[], vector<int> present_nodes, vector<int> end_nodes){
    Term test_term;
    Nodes_T z, w, ends, placeholder;  // placeholder bitstring is required for collapsation function, but serves no purpose in this test
    Edges_T edges;
    map <int, vector<int> > edge_terminals;
    // set up graph
    ListDigraph g;
    NameToNode nodeMap; // mapping from names to nodes in the graph
    ListDigraph::Node source; // start node
    ListDigraph::Node target; // target node
    WeightMap wMap(g); // keeps track of weights
    NodeNames nNames(g); // node names
    ArcIntMap arcIdMap(g); // mapping from the arcs to their ids in the original graph
    CreateGraph(graph_file, g, nNames, nodeMap, wMap, arcIdMap);
    // set up term object
    int node_counter = 0;
    for (ListDigraph::NodeIt a(g); a != INVALID; ++a){ // iterate through the edges in the graph
        bool present = false;
        for (int node: present_nodes){ // check which edges are present in the subgraph represented by the term
            if (node == node_counter){
                present = true;
                break;
            }
        }
        if (present){  // record the presence and absence of 
            z.set(node_counter);
        } else{
            w.set(node_counter);
        }
    }
    test_term = Term(z, w, 1.0);
    // set up terminal map
    for (ListDigraph::ArcIt a(g); a != INVALID; ++a){
        vector<int> terminals;
        terminals.push_back(g.id(g.source(a)));
        terminals.push_back(g.id(g.target(a)));
        edge_terminals[g.id(a)] = terminals;
        edges.set(g.id(a));
    }
    // set up bitstring from end_nodes
    for (int node: end_nodes){
        ends.set(node);
    }
    // test collapsation function
    return test_term.collapse(edges, edge_terminals, ends, placeholder);
}

bool testIteration(char graphFile[], char sourcesFile[], char targetsFile[]){
    // setup
    ListDigraph gOrig;
    NameToNode nodeMap; // mapping from names to nodes in the graph
    ListDigraph::Node sourceOrig; // start node
    ListDigraph::Node targetOrig; // target node
    WeightMap wMapOrig(gOrig); // keeps track of weights
    NodeNames nNames(gOrig); // node names
    ArcIntMap arcIdMapOrig(gOrig); // mapping from the arcs to their ids in the original graph
    CreateGraph(graphFile, gOrig, nNames, nodeMap, wMapOrig, arcIdMapOrig);
    UnifyTerminals(gOrig, wMapOrig, nNames, nodeMap, arcIdMapOrig, sourcesFile, targetsFile);
    sourceOrig = FindNode(SOURCE, gOrig, nNames, nodeMap);
    targetOrig = FindNode(SINK, gOrig, nNames, nodeMap);
    // test function
    Edges_T sampleEdges;
    vector<EdgeSubset> chances;
    double result = iteration(gOrig, wMapOrig, arcIdMapOrig, sourceOrig, targetOrig, true, 1, sampleEdges, false, false, chances);
    if (abs(result - 0.75) < 0.1){
        return true;
    } else {
        return false;
    }
}

TEST(GraphTest, small){
    char graph_file[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.txt";
    EXPECT_TRUE(createGraphTest(graph_file));
}

TEST(GraphTest, medium){
    char graph_file[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_10_4.txt";
    EXPECT_TRUE(createGraphTest(graph_file));
}

TEST(GraphTest, large){
    char graph_file[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_4_35_2.txt";
    EXPECT_TRUE(createGraphTest(graph_file));
}

TEST(CutTest, small){
    char graphFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.txt";
    char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.sources";
    char targetsFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.targets";
    EXPECT_TRUE(testCuts(graphFile, sourcesFile, targetsFile));
}

TEST(CutTest, medium1){
    char graphFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_10_4.txt";
    char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_10_4.sources";
    char targetsFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_10_4.targets";
    EXPECT_TRUE(testCuts(graphFile, sourcesFile, targetsFile));
}

TEST(CutTest, medium2){
    char graphFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_3_30_3.txt";
    char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_3_30_3.sources";
    char targetsFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_3_30_3.targets";
    EXPECT_TRUE(testCuts(graphFile, sourcesFile, targetsFile));
}

TEST(CutTest, large){
    char graphFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_4_35_2.txt";
    char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_4_35_2.sources";
    char targetsFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_4_35_2.targets";
    EXPECT_TRUE(testCuts(graphFile, sourcesFile, targetsFile));
}

TEST(SolveTest, small){
    char graphFile[] = "/home/erol/Documents/preach_sampling/test_graphs/1.txt";
    char sourcesFile[] = "/home/erol/Documents/preach_sampling/test_graphs/1.sources";
    char targetsFile[] = "/home/erol/Documents/preach_sampling/test_graphs/1.targets";
    double result = testSolve(graphFile, sourcesFile, targetsFile);
    bool correct;
    if ((0.875 - result) < 0.01){
        correct = true;
    } else {
        correct = false;
    }
    EXPECT_TRUE(correct);
}

TEST(CollapseTest, should_collapse){
    char graph_file[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_4.txt";
    vector<int> present = {0, 1, 2, 3, 4, 5};
    vector<int> ends = {3, 4};
    bool result = testCollapsation(graph_file, present, ends);
    EXPECT_TRUE(result);
}

TEST(IterationTest, test){
    char graphFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.txt";
    char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.sources";
    char targetsFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.targets";
    EXPECT_TRUE(testIteration(graphFile, sourcesFile, targetsFile));
}

int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
<<<<<<< HEAD
=======

>>>>>>> 1b214c50b126cf3272ce1ef8589f958efaa945d9
}
