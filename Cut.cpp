//
// Created by erol on 5/9/16.
//

#include "preach.h"
#include "Cut.h"

Cut::getMiddle(){
    return this->middle;
}
Cut::getRight(){
    return this->right;
}
Cut::getLeft(){
    return this->left;
}
Cut::size(){
    return this->middle.count();
}
Cut::getCoveredEdges(){
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

void ConsumeSausage(ListDigraph& g, WeightMap& wMap, Polynomial& poly, Edges_T& sausage, Nodes_T& endNodes){
    // Build a dictionary of edgeId -> source and target node ids
    // Will need it with each collapsation operation within this sausage
    map< int, vector<int> > edgeTerminals;

    FOREACH_BS(edgeId, sausage){
        vector<int> terminals;
        ListDigraph::Arc arc = g.arcFromId(edgeId);
        terminals.push_back(g.id(g.source(arc)));
        terminals.push_back(g.id(g.target(arc)));
        edgeTerminals[edgeId] = terminals;
    }

    vector<int> edgeIdList;
    for (int edgeId = 0; edgeId < sausage.size(); edgeId++) {
        if (sausage[edgeId]) {
            edgeIdList.push_back(edgeId);  // edgeId indexing is zero-based
        }
    }
/*
    //randomly rearrange edge IDs
    vector<int> temp;
    int size = edgeIdList.size();
    while(temp.size() != size){
        int random = (int) rand() / 1000;
        int index = (random % edgeIdList.size());
        temp.push_back(edgeIdList[index]);
        edgeIdList.erase(edgeIdList.begin() + index);
    }
    edgeIdList = temp;
*/

    //start adding the edges in the current sausage
    //here we collapse after each addition (arbitrary)
    int edgeCounter = 0;
    for (int edgeId: edgeIdList) {
        if (sausage[edgeId]) {
            //cout << "Adding edge " << edgeCounter;
            poly.addEdge(edgeId, wMap[g.arcFromId(edgeId)]);
            //cout << ", Collapsing!" << endl;
            poly.collapse(sausage, edgeTerminals, endNodes);
        }
    }

    //Advance the polynomial: make it ready for next sausage
    poly.advance();
}