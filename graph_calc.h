//
// Contains functions for performing calculations on graphs.
//

#include "preach.h"

double SuccessProb(vector<int> subset, ListDigraph& gOrig, WeightMap& wMapOrig){
    double result = 1.0;
    FOREACH_STL(arcId, subset){
        result *= (1.0 - wMapOrig[gOrig.arcFromId(arcId)]);
    }END_FOREACH;
    return result + 0.001;
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

/*Finds reachability probability given the vertex cuts
    Starts from the source to the first cut
    Then removes all the obsolete cuts and pick a next cut
    An obsolete cut is a cut whose middle set intersects with
    the left set of the current cut*/
double Solve(ListDigraph& g, WeightMap& wMap, ListDigraph::Node& source, ListDigraph::Node& target, vector<Cut>& cuts){
    //FOR DEBUGGING - REMOVE
    //sort(cuts.begin(), cuts.end(), compareCuts);
    // set up the source term and start the polynomial
    Nodes_T zSource, wSource;
    zSource.set(g.id(source));
    vector<Term> sourceTerm;
    sourceTerm.push_back(Term(zSource, wSource, 1.0));
    Polynomial poly(sourceTerm);

    Edges_T covered; // This will hold the set of covered edges so far
    Edges_T sausage; // This will hold the current sausage: edges being considered for addition

    // repeat until no cuts left
    dprintf("starting loop\n");
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
            ConsumeSausage(g, wMap, poly, sausage, nextCut.getMiddle());
        }catch(exception& e){
            cout << endl << "EXCEPTION: " << e.what() << ": " << typeid(e).name() << endl;
            exit(3);
        }
        //mark the sausage as covered
        covered |= sausage;
        //remove obsolete cuts
        RemoveObsoleteCuts(cuts, nextCut);
    }
    dprintf("Loop finished\n");

    // Last: add the edges between the last cut and the target node
    Edges_T allEdges; // set of all edges in the network
    EdgesAsBitset(g, allEdges);
    sausage = allEdges & ~covered; // the last sausage is all edges that are not yet covered
    Nodes_T targetSet; // The last stop
    targetSet.set(g.id(target));
    //cout << "Last step, Sausage size: " << sausage.count() << endl;
    //cout << "1  " << sausage.count() << "  ";
    ConsumeSausage(g, wMap, poly, sausage, targetSet);

    //RESULT
    return poly.getResult();
    //return -1.0;
}

/*Finds *SOME* good cuts: steps from a cut to the next by
replacing every node by all of its neighbors*/
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
        Nodes_T middle = currentMiddle;
        Nodes_T left = currentLeft;
        Nodes_T right = currentRight;
        Edges_T covered = currentCovered;
        added = false;
        FOREACH_BS(nodeId, currentMiddle){
            ListDigraph::Node node = g.nodeFromId(nodeId);
            vector<int> nextNodes;
            vector<int> nextArcs;
            for (ListDigraph::OutArcIt arc(g, node); arc != INVALID; ++arc){
                ListDigraph::Node next = g.target(arc);
                int nextId = g.id(next);
                if (nextId == g.id(target)){ // node connected to target, ignore all of its neighbors
                    nextNodes.clear();
                    nextArcs.clear();
                    break;
                }else if (right[nextId]){ // eligible for moving from right to middle
                    nextNodes.push_back(nextId);
                    nextArcs.push_back(g.id(arc));
                }
            }
            if (nextNodes.size() > 0){ // There are nodes to move from right to left
                added = true;
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
        if (added){
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
            currentMiddle = middle;
            currentLeft = left;
            currentRight = right;
            currentCovered = covered;
        }
    }

    FillEdgeSubsets(g, cuts, edgeSubsets);
    RefineCuts(cuts, g, target, edgeSubsets);
}
