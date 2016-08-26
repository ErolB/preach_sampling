//
// Created by erol on 5/9/16.
//

#include "Cut.h"

Nodes_T& Cut::getMiddle(){
    return this->middle;
}
Nodes_T& Cut::getRight(){
    return this->right;
}
Nodes_T& Cut::getLeft(){
    return this->left;
}
int Cut::size(){
    return this->middle.count();
}
Edges_T& Cut::getCoveredEdges(){
    return coveredEdges;
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

/*Just for debugging - ignore*/
//bool compareCuts(Cut cut1, Cut cut2){
//    return (cut1.getMiddle().count() < cut2.getMiddle().count());

/*Removes cuts that are obsoleted by cut
A cut is obsolete if its middle set overlaps with the left set of cut*/
void RemoveObsoleteCuts(vector<Cut>& cuts, Cut& cut){
    for (size_t i=0; i<cuts.size(); i++){
        Cut currentCut = cuts.at(i);
        if ((currentCut.getMiddle() & cut.getLeft()).any()){ // currentCut is obsolete
            cuts.erase(cuts.begin() + i);
            i--;
        }
    }
}

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

/*creates first level cut: nodes adjacent to source*/
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

/*Finds *SOME* good cuts: steps from a cut to the next by
replacing every node by all of its neighbors.

This function generates a set of consecutive cuts (which may share nodes with neighboring cuts). Starting with the
 initial cut (given by the function createFirstCut), it iterates over every node in the current cut. For each of the
 node's outgoing edges. If an edge connects to a node on the right side of the cut, its target node is added to the
 next cut. The next cut is then processed to ensure that it fits a set of requirements. It must be a genuine cut and not
 have any backward edges.
*/
void FindSomeGoodCuts(ListDigraph& g, ListDigraph::Node source, ListDigraph::Node target, vector<Cut>& cuts, vector< pair< vector<int>, int > > & edgeSubsets){
    //start by forming the first cut: adjacent to source
    Cut firstCut = createFirstCut(g, source, target);
    if (firstCut.getMiddle().none()){ // That was a dummy returned cut, i.e. no cuts available
        return;
    }
    Nodes_T currentMiddle = firstCut.getMiddle();
    Nodes_T currentLeft = firstCut.getLeft();
    Nodes_T currentRight = firstCut.getRight();
    Edges_T currentCovered = firstCut.getCoveredEdges();
    cuts.push_back(firstCut);
    bool added = true;
    while (added){ // repeat until nothing new is added
        //
        Nodes_T middle = currentMiddle;
        Nodes_T left = currentLeft;
        Nodes_T right = currentRight;
        Edges_T covered = currentCovered;
        added = false;
        // iterate over the node in the current cut
        FOREACH_BS(nodeId, currentMiddle){
            ListDigraph::Node node = g.nodeFromId(nodeId);
            vector<int> nextNodes;
            vector<int> nextArcs;
            // iterate over the node's outgoing edges
            for (ListDigraph::OutArcIt arc(g, node); arc != INVALID; ++arc){
                ListDigraph::Node next = g.target(arc);
                int nextId = g.id(next);
                if (nextId == g.id(target)){ // node connected to target, ignore all of its neighbors
                    nextNodes.clear();
                    nextArcs.clear();
                    break;
                }else if (right[nextId]){
                    // If the edge's target is to the right of the current node, it is eligible for moving from the
                    // right to the middle.
                    nextNodes.push_back(nextId);
                    nextArcs.push_back(g.id(arc));
                }
            }
            if (nextNodes.size() > 0){ // There are nodes to move from right to left
                added = true;
                // adjust the bitsets to represent the new cut
                FOREACH_STL(nextId, nextNodes){
                        right.reset(nextId);
                        middle.set(nextId);
                    }END_FOREACH;
                FOREACH_STL(nextId, nextArcs){
                        covered.set(nextId);
                    }END_FOREACH;
                middle.reset(nodeId);
                left.set(nodeId);
            }
        }
        if (added){  // if a new cut will be added
            // mark as covered: all edges going from the middle not to the right
            FOREACH_BS(nodeId, middle){
                ListDigraph::Node middleNode = g.nodeFromId(nodeId);
                for (ListDigraph::OutArcIt arc(g, middleNode); arc != INVALID; ++arc){
                    if (!right[g.id(g.target(arc))]){
                        covered.set(g.id(arc));
                    }
                }
            }
            // create a new cut object and add it to the vector
            Cut newCut(left, middle, right, covered);
            cuts.push_back(newCut);
            // the new cut becomes the current cut
            currentMiddle = middle;
            currentLeft = left;
            currentRight = right;
            currentCovered = covered;
        }
    }

    FillEdgeSubsets(g, cuts, edgeSubsets);
    RefineCuts(cuts, g, target, edgeSubsets);

    cout << "original cuts: " << cuts.size() << endl;
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

/*removes cuts that are masked by smaller cuts*/
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

/*Does the needed preprocessing of the graph:
 * adding source & sink
 * remove isolated nodes
 * collapse elementary paths
 * see comments of each function for details
 * */
void Preprocess(ListDigraph& g,
                WeightMap& wMap,
                NodeNames& nMap,
                NameToNode& nodeMap,
                ArcIntMap& arcIdMap,
                string sourcesFile,
                string targetsFile,
                string pre){
    UnifyTerminals(g, wMap, nMap, nodeMap, arcIdMap, sourcesFile, targetsFile);
    if (pre == PRE_YES){
        RemoveIsolatedNodes(g, nodeMap[SOURCE], nodeMap[SINK]);
        CollapseELementaryPaths(g, wMap, arcIdMap, nodeMap[SOURCE], nodeMap[SINK]);
        RemoveSelfCycles(g);
    }
    //EXTRA STEP: make sure source and sink are not directly connected
    ListDigraph::Node source = nodeMap[SOURCE];
    ListDigraph::Node sink = nodeMap[SINK];
    for (ListDigraph::OutArcIt arc(g, source); arc != INVALID; ++arc){
        if (g.id(g.target(arc)) == g.id(sink)){ // direct source-sink connection - isolate with a middle node
            ListDigraph::Node isolator = g.addNode();
            nodeMap["ISOLATOR"] = isolator;
            nMap[isolator] = "ISOLATOR";
            ListDigraph::Arc head = g.addArc(isolator, sink);
            wMap[head] = wMap[arc];
            arcIdMap[head] = g.id(head);
            ListDigraph::Arc tail = g.addArc(source, isolator);
            arcIdMap[tail] = g.id(tail);
            wMap[tail] = SURE;
            g.erase(arc);
            break;
        }
    }
}

/*prints cuts*/
void PrintCuts(vector<Cut>& cuts, ListDigraph& g){
    FOREACH_STL(cut, cuts){
            PrintCut(cut, g);
        }END_FOREACH;
}

void HorizontalPaths(vector<int> edges_covered, ListDigraph::Node start_node, Cut end_cut, ListDigraph& g){
    cout << "finding horizontal cuts " << endl;
    for (ListDigraph::OutArcIt edge(g,start_node); edge != INVALID; ++edge){
        int target_id = g.id(g.target(edge));
        if (!end_cut.getMiddle()[target_id]) {  // if the edge is not on the target cut
            edges_covered.push_back(g.id(edge));
            HorizontalPaths(edges_covered, start_node, end_cut, g);
        }
    }
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

// version for edge bitset
vector<int> cvtEdgeBitset(Edges_T input){
    vector<int> positions;
    for (int i = 0; i < input.size(); i++){
        if (input[i]){
            positions.push_back(i);
        }
    }
    return positions;
}

vector< vector<int> > PathsFromPointTesting(ListDigraph& g, int startNode, vector<int> startCut, vector<int> endCut, vector<int> base){
    vector< vector<int> > paths;
    for(ListDigraph::OutArcIt o(g, g.nodeFromId(startNode)); o != INVALID; ++o) {
        int current_id = g.id(g.target(o));
        // skip if node is on start cut
        bool on_start = false;
        for (int nodeId: startCut){
            if (current_id == nodeId){ on_start = true; }
        }
        if (on_start){ continue; }
        // check if node is already on the path
        bool on_path = false;
        for (int nodeId: base){
            if (current_id == nodeId) { on_path = true; }
        }
        if (on_path){ continue; }
        // add current node
        vector<int> new_path = base;
        new_path.push_back(current_id);
        // check if current node completes path
        bool completed = false;
        for (int nodeId: endCut){
            if (nodeId == current_id) {
                completed = true;
                paths.push_back(new_path);
                break;
            }
        }
        // add path
        if (!completed){
            vector< vector<int> > new_paths = PathsFromPointTesting(g, current_id, startCut, endCut, new_path);
            for(vector<int> path: new_paths){
                paths.push_back(path);
            }
        }
    }
    return paths;
}

vector< vector<int> > PathsFromCutTesting(ListDigraph& g, vector<int> startCut, vector<int> endCut){
    vector< vector<int> > all_paths;
    for (int startNode: startCut){
        vector<int> base;
        base.push_back(startNode);
        vector< vector<int> > new_paths = PathsFromPointTesting(g, startNode, startCut, endCut, base);
        for (vector<int> path: new_paths){
            all_paths.push_back(path);
        }
    }
    return all_paths;
}

void ConsumeSausage(ListDigraph& g, WeightMap& wMap, Polynomial& poly, Edges_T& sausage, Nodes_T& endNodes, vector< vector<int> > paths){
    // Build a dictionary of edgeId -> source and target node ids
    // Will need it with each collapsation operation within this sausage
    map< int, vector<int> > edgeTerminals;
    map< pair< int, int >, int > reverseTerminalMap;

    FOREACH_BS(edgeId, sausage){
        vector<int> terminals;
        ListDigraph::Arc arc = g.arcFromId(edgeId);
        terminals.push_back(g.id(g.source(arc)));
        terminals.push_back(g.id(g.target(arc)));
        edgeTerminals[edgeId] = terminals;
        reverseTerminalMap[make_pair(g.id(g.source(arc)), g.id(g.target(arc)))] = edgeId;
    }
    // build vectors from sausage
    vector<int> edgeIdList;
    for (int edgeId = 0; edgeId < sausage.size(); edgeId++) {
        if (sausage[edgeId]) {
            edgeIdList.push_back(edgeId);  // edgeId indexing is zero-based
        }
    }
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
                // if the score map contains an entry for that distance from completion
                if (scoreMap.count(counter)) { scoreMap[counter]++; }
                else { scoreMap[counter] = 1; }
                //cout << "edge: " << edgeId << ", score: " << counter << endl;
            }
        }
        edgeScores[edgeId] = scoreMap;
    }
    /*
    //randomly rearrange edge IDs
    vector<int> temp;
    int size = edgeIdList.size();
    srand((int)getCPUTime());  // seed random generator
    while(temp.size() != size){
        int random = (int) rand() / 1000;
        int index = (random % edgeIdList.size());
        temp.push_back(edgeIdList[index]);
        edgeIdList.erase(edgeIdList.begin() + index);
    }
    edgeIdList = temp;

    // order edges based on scores
    */
    int stop_count = 10;
    vector<int> tempEdgeList = edgeIdList;
    edgeIdList.clear();
    while (!tempEdgeList.empty()){
        // find ideal edge ("greedy" approach)
        vector<int> bestEdges;
        for (int i = 1; i < stop_count; i++) { // "i" is how close a path is to completion
            int maxScore = 0;
            for (int edgeId: tempEdgeList) {
                map<int, int> scoreMap = edgeScores[edgeId];
                if (!scoreMap.count(i)){ break; } // break out of loop if score is not available
                if (scoreMap[i] > maxScore){ maxScore = scoreMap[i]; }
            }
            // now that the maximum score is known, find the ideal edges
            bestEdges.clear();  // start over
            for (int edgeId: tempEdgeList) {
                if (edgeScores[edgeId][i] == maxScore){ bestEdges.push_back(edgeId); }
            }
        }
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
                    cout << "edge: " << edgeId << ", score: " << counter << endl;
                }
            }
            edgeScores[edgeId] = scoreMap;
        }
    }

    //start adding the edges in the current sausage
    //here we collapse after each addition (arbitrary)
    double start = getCPUTime();
    for (int edgeId: edgeIdList) {
        cout << edgeId << " ";
        if (sausage[edgeId]) {
            //cout << "Adding edge " << edgeCounter;
            poly.addEdge(edgeId, wMap[g.arcFromId(edgeId)]);
            //cout << ", Collapsing!" << endl;
            poly.collapse(sausage, edgeTerminals, endNodes);
        }
    }
    cout << "***" << (getCPUTime() - start) << "***" << endl;

    //Advance the polynomial: make it ready for next sausage
    poly.advance();
}

double Solve(ListDigraph& g, WeightMap& wMap, ListDigraph::Node& source, ListDigraph::Node& target, vector<Cut>& cuts){
    //FOR DEBUGGING - REMOVE
    //sort(cuts.begin(), cuts.end(), compareCuts);
    // set up the source term and start the polynomial
    Nodes_T zSource, wSource;
    zSource.set(g.id(source));
    vector<Term> sourceTerm;
    sourceTerm.push_back(Term(zSource, wSource, 1.0));
    Polynomial poly(sourceTerm);
    vector< vector<int> > paths;

    Edges_T covered; // This will hold the set of covered edges so far
    Edges_T sausage; // This will hold the current sausage: edges being considered for addition

    // repeat until no cuts left
    cout << "---" << endl;
    while(cuts.size() > 0){
        dprintf("beginning iteration\n");
        //select a cut: here we just select the first one (arbitrary)
        Cut nextCut = cuts.front();
        //cout << "Available " << cuts.size() << " cuts, Using cut with size " << nextCut.size();
        //cout << nextCut.size() << "  ";
        cuts.erase(cuts.begin());
        // Identify the sausage: The current set of edges in question
        sausage = nextCut.getCoveredEdges() & ~covered;
        //cout << ", Sausage size: " << sausage.count() << endl;
        //cout << sausage.count() << "  ";
        //Consume the current sausage
        try{
            vector<int> base = {g.id(source)};
            vector<int> start_cut = base;
            vector< vector<int> > paths = PathsFromPointTesting(g, g.id(source), start_cut, cvtBitset(nextCut.getMiddle()), base);
            ConsumeSausage(g, wMap, poly, sausage, nextCut.getMiddle(), paths);
        }catch(exception& e){
            cout << endl << "EXCEPTION: " << e.what() << ": " << typeid(e).name() << endl;
            exit(3);
        }
        //mark the sausage as covered
        covered |= sausage;
        //remove obsolete cuts
        RemoveObsoleteCuts(cuts, nextCut);
    }
    cout << "---" << endl;

    // Last: add the edges between the last cut and the target node
    Edges_T allEdges; // set of all edges in the network
    EdgesAsBitset(g, allEdges);
    sausage = allEdges & ~covered; // the last sausage is all edges that are not yet covered
    Nodes_T targetSet; // The last stop
    targetSet.set(g.id(target));
    //cout << "Last step, Sausage size: " << sausage.count() << endl;
    //cout << "1  " << sausage.count() << "  ";
    double start = getCPUTime();
    paths.clear();
    vector<int> finalEdges = cvtEdgeBitset(sausage);
    for (int edge: finalEdges){
        paths.push_back({edge});
    }
    ConsumeSausage(g, wMap, poly, sausage, targetSet, paths);
    cout << (getCPUTime() - start) << " milliseconds" << endl;

    //RESULT
    return poly.getResult();
    //return -1.0;
}