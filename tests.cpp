//#include <gtest/gtest.h>
#include <fstream>
#include "/usr/include/gtest/gtest.h"
#include "Probing.h"  // includes all other PReach files

/*
// create function that builds graph structures
ListDigraph& buildGraph(char graphFile[], char sourcesFile[], char targetsFile[]){
    // create graph structure
    ListDigraph g;
    NameToNode nodeMap; // mapping from names to nodes in the graph
    ListDigraph::Node source; // start node
    ListDigraph::Node target; // target node
    WeightMap wMap(g); // keeps track of weights
    NodeNames nNames(g); // node names
    ArcIntMap arcIdMap(g); // mapping from the arcs to their ids in the original graph
    CreateGraph(graphFile, g, nNames, nodeMap, wMap, arcIdMap);
}
 */

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
		      //cout << "left to right" << endl;
		      return false;  // an edge crossed the cut, so it is invalid (no more computation needed)
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
		      //cout << "right to left" << endl;
		      //cout << origin_id << "-->" << node_id << endl;  // for debugging
		      return false;  // no more computation needed

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
	    //cout << "valid cut" << endl;
    }
    // if the function reaches this point, then the test has been passed
    return true;
}



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

// tests the path finding function
bool testPaths(char graphFile[], char sourcesFile[], char targetsFile[]){
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

    bool success = true;
    for (int i = 1; i < cuts.size(); i++){
        for (int startNode: cvtBitset(cuts[i-1].getMiddle())){
            vector<int> base = {startNode};
            vector< vector<int> > paths = PathsFromPointTesting(gOrig, startNode, cvtBitset(cuts[i-1].getMiddle()), cvtBitset(cuts[i].getMiddle()), base);
            // validate paths
            for (vector<int> path: paths){
                /*
                cout << "nodes: ";
                for (int node: path){ cout << node << ", ";}
                cout << endl;
                */
                for (int j = 0; j < path.size(); j++) {
                    int nodeId = path[j];
                    ListDigraph::Node currentNode = gOrig.nodeFromId(nodeId);
                    if (j > 0){
                        bool nodePresent = false;
                        int prevNodeId = path[j-1];
                        for (ListDigraph::InArcIt a(gOrig, currentNode); a != INVALID; ++a){
                            if (gOrig.id(gOrig.source(a)) == prevNodeId){
                                nodePresent = true;
                                break;
                            }
                        }
                        if (!nodePresent){
                            return false;
                        }
                    }
                }
            }
        }
    }
    return true;
}

// test edge ordering heuristic

vector<int> orderEdges(ListDigraph& g, Cut cut1, Cut cut2){
    // find paths
    vector<int> startNodes = cvtBitset(cut1.getMiddle());
    vector<int> endNodes = cvtBitset(cut2.getMiddle());
    vector<int> edgeIdList = cvtEdgeBitset(cut2.getCoveredEdges() & ~cut1.getCoveredEdges());
    vector< vector<int> > paths = PathsFromCutTesting(g, startNodes, endNodes);
    cout << "found paths" << endl;

    // Build a dictionary of edgeId -> source and target node ids
    // Will need it with each collapsation operation within this sausage
    map< int, vector<int> > edgeTerminals;
    map< pair< int, int >, int > reverseTerminalMap;
    // construct map structures representing paths
    vector< map< int, bool > > path_maps;
    for (vector<int> path: paths){
        map< int, bool > current_map;
        for (int edgeId: edgeIdList){
            current_map[edgeId] = false;
        }
        for (int i = 1; i < path.size(); i++){
            int current_id = reverseTerminalMap[make_pair(path[i-1], path[i])];
            current_map[current_id] = true;
        }
        path_maps.push_back(current_map);
    }
    cout << "built path structure" << endl;
    // create data structure to store scoring information for each edge
    // each edge is assigned a map which represents each path it is in, and separates the paths by how close they are
    //     to being completed
    map< int, map< int, int > > edgeScores;
    for (int edgeId: edgeIdList) {
        map< int, int > scoreMap;
        // for each path
        for (map < int, bool > path_map: path_maps){
            // if current edge on current path
            if (path_map[edgeId]){
                // count number of edges left until path is completed
                int counter = 0;
                for (int current_edge: edgeIdList){
                    if (path_map[current_edge]){ counter++; }
                }
                if (counter == 0) { continue; }
                // if the score map contains an entry for that distance from completion
                if (scoreMap.count(counter)) { scoreMap[counter]++; }
                else { scoreMap[counter] = 1; }
                //cout << "edge: " << edgeId << ", score: " << counter << endl;
            }
        }
        edgeScores[edgeId] = scoreMap;
    }
    cout << "edges scored" << endl;
    int stop_count = 10;
    vector<int> tempEdgeList = edgeIdList;
    edgeIdList.clear();
    while (!tempEdgeList.empty()){
        // find ideal edge ("greedy" approach)
        vector<int> bestEdges;
        for (int i = 1; i < stop_count; i++) { // "i" is how close a path is to completion
            int maxScore = -1;
            for (int edgeId: tempEdgeList) {
                // create blank entries in score map where none exist
                map<int, int> scoreMap = edgeScores[edgeId];
                if (!scoreMap.count(i)) {
                    edgeScores[edgeId][i] = 0;
                    scoreMap[i] = 0;
                }
                //if (!scoreMap.count(i)) { continue; } // continue if score is not available
                if (scoreMap[i] > maxScore) { maxScore = scoreMap[i]; }
            }
            if (maxScore == -1) { continue; }  // new
            // now that the maximum score is known, find the ideal edges
            bestEdges.clear();  // start over
            for (int edgeId: tempEdgeList) {
                if (!edgeScores[edgeId].count(i)) { continue; }  // prevents creation of blank entries
                if (edgeScores[edgeId][i] == maxScore){ bestEdges.push_back(edgeId); }
            }
        }
        cout << "found ideal edges" << endl;
        cout << tempEdgeList.size() << endl;
        int bestEdge = bestEdges[0];  // pick an ideal edge (arbitrary)
        // add edge to list and remove from temporary list
        edgeIdList.push_back(bestEdge);
        for (int i = 0; i < tempEdgeList.size(); i++){
            if (tempEdgeList[i] == bestEdge){
                tempEdgeList.erase(tempEdgeList.begin()+i);
                break;
            }
        }
        // update path map
        for (map< int, bool > path: path_maps){
            path[bestEdge] = false;
        }
        // update score map (make a separate function)
        for (int edgeId: edgeIdList) {
            map< int, int > scoreMap;
            // for each path
            for (map < int, bool > path_map: path_maps){
                // if current edge on current path
                if (path_map[edgeId]){
                    // count number of edges left until path is completed
                    int counter = 0;
                    for (int current_edge: edgeIdList){
                        if (path_map[current_edge]){ counter++; }
                    }
                    // if the score map contains an entry for that distance from completion
                    if (scoreMap.count(counter) != 0) { scoreMap[counter]++; }
                    else { scoreMap[counter] = 1; }
                }
            }
            edgeScores[edgeId] = scoreMap;
        }
    }
    return edgeIdList;
}

bool testHeuristic(char graphFile[], char sourcesFile[], char targetsFile[]){
    // create graph structure
    ListDigraph g;
    NameToNode nodeMap; // mapping from names to nodes in the graph
    ListDigraph::Node source; // start node
    ListDigraph::Node target; // target node
    WeightMap wMap(g); // keeps track of weights
    NodeNames nNames(g); // node names
    ArcIntMap arcIdMap(g); // mapping from the arcs to their ids in the original graph
    CreateGraph(graphFile, g, nNames, nodeMap, wMap, arcIdMap);
    UnifyTerminals(g, wMap, nNames, nodeMap, arcIdMap, sourcesFile, targetsFile);
    source = FindNode(SOURCE, g, nNames, nodeMap);
    target = FindNode(SINK, g, nNames, nodeMap);
    vector<pair<vector<int>, int> > edgeSubsets;
    vector<Cut> cuts;
    FindSomeGoodCuts(g, source, target, cuts, edgeSubsets);
    for (int node: cvtBitset(cuts[1].getMiddle())){
        cout << node << endl;
    }
    vector<int> optimized = orderEdges(g, cuts[1], cuts[2]);
    for (int node: optimized){
        cout << node << endl;
    }
}

bool testHorizontalCuts(char graphFile[], char sourcesFile[], char targetsFile[]){
    // create graph structure
    ListDigraph g;
    NameToNode nodeMap; // mapping from names to nodes in the graph
    ListDigraph::Node source; // start node
    ListDigraph::Node target; // target node
    WeightMap wMap(g); // keeps track of weights
    NodeNames nNames(g); // node names
    ArcIntMap arcIdMap(g); // mapping from the arcs to their ids in the original graph
    CreateGraph(graphFile, g, nNames, nodeMap, wMap, arcIdMap);
    UnifyTerminals(g, wMap, nNames, nodeMap, arcIdMap, sourcesFile, targetsFile);
    source = FindNode(SOURCE, g, nNames, nodeMap);
    target = FindNode(SINK, g, nNames, nodeMap);
    vector<pair<vector<int>, int> > edgeSubsets;
    vector<Cut> cuts;
    FindSomeGoodCuts(g, source, target, cuts, edgeSubsets);
    cout << "cuts: " << endl;
    for (Cut current_cut: cuts){
        for (int edge: cvtBitset(current_cut.getMiddle())){
            cout << edge << " ";
        }
        cout << endl;
    }
    cout << endl;
    map< int, set<int> > result = HorizontalCuts(cuts[1], cuts[2], g);
    for (auto item: result){
        cout << item.first << endl;
        for (int edge: item.second){
            ListDigraph::Arc edge_obj = g.arcFromId(edge);
            cout << g.id(g.source(edge_obj)) << " -> " << g.id(g.target(edge_obj)) << endl;
        }
    }
    cout << endl;
    return true;
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
    char graphFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_4_65_3.txt";
    char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_4_65_3.sources";
    char targetsFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_4_65_3.targets";
    EXPECT_TRUE(testCuts(graphFile, sourcesFile, targetsFile));
}
/*
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
 */

TEST(IterationTest, test){
    char graphFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.txt";
    char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.sources";
    char targetsFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_5_2.targets";
    EXPECT_TRUE(testIteration(graphFile, sourcesFile, targetsFile));
}

TEST(PathTest, test){
    char graphFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_10_4.txt";
    char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_10_4.sources";
    char targetsFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_10_4.targets";
    EXPECT_TRUE(testPaths(graphFile, sourcesFile, targetsFile));
}

TEST(HeuristicTest, test){
    char graphFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_10_4.txt";
    char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_10_4.sources";
    char targetsFile[] = "/home/erol/Documents/preach_sampling/data/synthetic/BA_2_10_4.targets";
    testHeuristic(graphFile, sourcesFile, targetsFile);
}

TEST(HorizontalCutsTest, test){
    /*
    char graphFile[] = "/home/erol/Documents/preach_sampling/data/unit_testing/graph1";
    char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/unit_testing/sources1";
    char targetsFile[] = "/home/erol/Documents/preach_sampling/data/unit_testing/targets1";
    */
    
    char graphFile[] = "/home/erol/Documents/preach_sampling/data/kegg/cellcycle/Renal.net";
    char sourcesFile[] = "/home/erol/Documents/preach_sampling/data/kegg/cellcycle/sources.txt";
    char targetsFile[] = "/home/erol/Documents/preach_sampling/data/kegg/cellcycle/targets.txt";
    
    testHorizontalCuts(graphFile, sourcesFile, targetsFile);
}

int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
