#include "preach.h"

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


// map names to nodes
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

bool EdgeExists(ListDigraph& g, ListDigraph::Node& source, ListDigraph::Node& target){
	for (ListDigraph::OutArcIt arc(g, source); arc != INVALID; ++arc){
		if (g.id(g.target(arc)) == g.id(target))
			return true;
	}
	return false;
}

// Create the graph from the file
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

/*Gets the In-degree of a node*/
int getNodeInDegree(ListDigraph& g, ListDigraph::Node& node){
	int count = 0;
	for (ListDigraph::InArcIt arc(g, node); arc != INVALID; ++arc)
		count++;
	return count;
}

/*Gets the Out-degree of a node*/
int getNodeOutDegree(ListDigraph& g, ListDigraph::Node& node){
	int count = 0;
	for (ListDigraph::OutArcIt arc(g, node); arc != INVALID; ++arc)
		count++;
	return count;
}


/*Reverses the graph: replaces each edge by its reverse edge*/
void reverseGraph(ListDigraph& g){
    // Collect a list of all edges
    vector<ListDigraph::Arc> arcs;
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        arcs.push_back(arc);
    }
    FOREACH_STL(arc, arcs){
        g.reverseArc(arc);
    }END_FOREACH;
}

/*Removes "Isolated" nodes from the graph
 * An isolated node is the nodes that are not reachable from source or
 * can't reach to sink*/
void RemoveIsolatedNodes(ListDigraph& g, ListDigraph::Node& source, ListDigraph::Node& target){
    // First: Make a forward traversal and mark the reachable nodes from source
    Nodes_T forward;
    Bfs<ListDigraph> bfs(g);
    bfs.run(source);
    for (ListDigraph::NodeIt node(g); node != INVALID; ++node){
        if (bfs.reached(node)){
            forward.set(g.id(node));
        }
    }
    // Second: reverse the graph and make a backward traversal
    // and mark the reachable nodes from the sink
    Nodes_T backward;
    reverseGraph(g);
    bfs = Bfs<ListDigraph>(g);
    bfs.run(target);
    for (ListDigraph::NodeIt node(g); node != INVALID; ++node){
        if (bfs.reached(node)){
            backward.set(g.id(node));
        }
    }
    // reverse the graph again to return it to original state
    reverseGraph(g);

    //collect bad nodes
    vector<ListDigraph::Node> badNodes;
    for (ListDigraph::NodeIt node(g); node != INVALID; ++node){
        if (g.id(node) != g.id(source) && g.id(node) != g.id(target) && !(forward[g.id(node)] && backward[g.id(node)]))
            badNodes.push_back(node);
    }

    // Erase all bad nodes
    FOREACH_STL(node, badNodes){
        g.erase(node);
    }END_FOREACH;
}


/*
Removes edges that are self cycles
*/
void RemoveSelfCycles(ListDigraph& g){
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        if (g.source(arc) == g.target(arc)){
            g.erase(arc);
        }
    }
}


/*Collapses all elementary paths
 * An elementary path: a --> x --> b , with x not connected to anything else
 * we delete x and create a new link a --> b with weight w = weight(a-->x)*weight(x-->b)
 * if an edge a --> b already exists before with weight w', we merge the old edge with the new one with
 * a weight = 1-(1-w)(1-w')
 * */
void CollapseELementaryPaths(ListDigraph& g, WeightMap& wMap, ArcIntMap& arcIdMap, ListDigraph::Node& source, ListDigraph::Node& target){
	// repeat until nothing changes
	bool changing = true;
	while(changing){
		changing = false;
		RemoveSelfCycles(g);
		vector<ListDigraph::Node> elementaryNodes;
		for (ListDigraph::NodeIt node(g); node != INVALID; ++node){
			if (node == source || node == target)
				continue;
            if (getNodeInDegree(g, node) == 1 && getNodeOutDegree(g, node) == 1){
				// elementary path, mark node to be removed
				elementaryNodes.push_back(node);
			}
		}
		// handle marked nodes: remove their edges and delete them
		FOREACH_STL(node, elementaryNodes){
			//link before with after
			for (ListDigraph::OutArcIt outArc(g, node); outArc != INVALID; ++outArc){
				for (ListDigraph::InArcIt inArc(g, node); inArc != INVALID; ++inArc){
					bool found = false;
					//Find existing arc between before and after
					for (ListDigraph::OutArcIt arc(g, g.source(inArc)); arc != INVALID; ++arc){
						if (g.target(arc) == g.target(outArc)){
							// a link already exists
							wMap[arc] = 1 - (1 - wMap[arc]) * (1 - wMap[inArc]*wMap[outArc]);
							found = true;
							break;
						}
					}
					if (!found){ // no existing link.. add one
						ListDigraph::Arc newArc = g.addArc(g.source(inArc), g.target(outArc));
						wMap[newArc] = wMap[inArc]*wMap[outArc];
						arcIdMap[newArc] = g.id(newArc);
					}
				}
			}
			g.erase(node);
			changing = true;
		}END_FOREACH;
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
                pair< vector<int>, int > subset = make_pair(incoming, middle.count()-1);
                edgeSubsets.push_back(subset);
            }
        }
    }END_FOREACH;
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

/*Gets all edges as a bitset*/
void EdgesAsBitset(ListDigraph& g, Edges_T& edges){
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        edges.set(g.id(arc));
    }
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

    //start adding the edges in the current sausage
    //here we collapse after each addition (arbitrary)
    int edgeCounter = 0;
    FOREACH_BS(edgeId, sausage){
        edgeCounter ++;
        //cout << "Adding edge " << edgeCounter;
        poly.addEdge(edgeId, wMap[g.arcFromId(edgeId)]);
        //cout << ", Collapsing!" << endl;
        poly.collapse(sausage, edgeTerminals, endNodes);
    }

    //Advance the polynomial: make it ready for next sausage
    poly.advance();
}

/*Just for debugging - ignore*/
bool compareCuts(Cut cut1, Cut cut2){
    return (cut1.getMiddle().count() < cut2.getMiddle().count());
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
    while(cuts.size() > 0){
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

/* This function samples the edges in sampleEdges */
void SampleFixed(ListDigraph& g, WeightMap& wMap,
                 ListDigraph::Node& source, ListDigraph::Node& target,
                 Edges_T& sampleEdges, ArcIntMap& arcIdMap){
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        if (sampleEdges.test(arcIdMap[arc])){ // edge is in sampleEdges
            if (drand48() <= wMap[arc]){ // sampling coin toss
                wMap[arc] = 1.0;
            } else {
                wMap[arc] = 0.0;
                g.erase(arc);
            }
        }
    }
}

/* This function samples the graph. The edges to sample are randomly selected.
** For every edge (excpet from SOURCE or to TARGET), it tosses a coin with success prob = samplingProb
** If success, the edge is sampled using its edge probability
** Edges that result in 0 are erased from g.
** Also sampleEdges will change to reflect the resulting sampled set of edges.
*/
void SampleRandom(ListDigraph& g, WeightMap& wMap,
                  ListDigraph::Node& source, ListDigraph::Node& target,
                  double samplingProb, Edges_T& sampleEdges, ArcIntMap& arcIdMap){
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        if (g.source(arc) == source || g.target(arc) == target)
            continue;
        if (drand48() <= samplingProb){ // generic coin toss
            sampleEdges.set(arcIdMap[arc]);
            if (drand48() <= wMap[arc]){ // sampling coin toss
                wMap[arc] = 1.0;
            } else {
                wMap[arc] = 0.0;
                g.erase(arc);
            }
        }
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


/* This method samples the graph using weighted random sampling through the chances vector.
   It randomly selects an edge subset from chances vector, samples its members using their probability
   Until the budget is met, which is derived from the samplingProb and the number of edges
*/
void SampleWeightedRandom(ListDigraph& g, WeightMap& wMap,
                  ListDigraph::Node& source, ListDigraph::Node& target,
                  double samplingProb, Edges_T& sampleEdges, ArcIntMap& arcIdMap, vector<EdgeSubset>& chances){
    // Our sampling budget
    int budget = (int) ceil(countArcs(g) * samplingProb);
    // The total number of edges
    int edgesCount = countArcs(g);

    // inverted list of original arc ids to arcs in g
    map<int, ListDigraph::Arc> idToArc;
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        idToArc[arcIdMap[arc]] = arc;
    }

    while (budget > 0){
        int chancesSize = chances.size();
        if (chancesSize > 0){
            int index = (int) floor(drand48() * chances.size());
            EdgeSubset es = chances[index];
            FOREACH_STL(arcId, es.subset){
                if (!sampleEdges.test(arcId)){
                    budget --;
                    sampleEdges.set(arcId);
                    ListDigraph::Arc arc = idToArc[arcId];
                    if (drand48() <= wMap[arc]){ // sampling coin toss
                        wMap[arc] = 1.0;
                    } else {
                        wMap[arc] = 0.0;
                        g.erase(arc);
                    }
                    if (budget <= 0)
                        break;
                }
            }END_FOREACH;
            // Here we go forward and backwards from index to remove the selected entries from chances
            int id = chances[index].id;
            int start, end;
            for (start=index; start>=0 && chances[start].id==id; --start);
            start ++;
            for (end=index; end<chances.size() && chances[end].id==id; ++end);
            chances.erase(chances.begin()+start, chances.begin()+end);
        } else {
            int edge = (int) floor(drand48() * edgesCount);
            while (sampleEdges.test(edge))
                edge = (int) floor(drand48() * edgesCount);
            sampleEdges.set(edge);
            budget --;
        }
    }
}

/*This function performs one sampling iteration, returns the reachability result
** sampleEdges will be used if fixed is set to true
** sampleEdges will be changed to the chosen random set if fixed is false
*/
double iteration(ListDigraph& gOrig, WeightMap& wMapOrig, ArcIntMap& arcIdMapOrig,
                 ListDigraph::Node& sourceOrig, ListDigraph::Node& targetOrig,
                 bool fixed, double samplingProb, Edges_T& sampleEdges, bool print,
                 bool weighted, vector<EdgeSubset>& chances){
    //Initialize a new graph from the original
    ListDigraph g;
    WeightMap wMap(g);
    ArcIntMap arcIdMap(g);
    ListDigraph::Node source;
    ListDigraph::Node target;
    digraphCopy(gOrig, g).node(sourceOrig, source).node(targetOrig, target).arcMap(wMapOrig, wMap).arcMap(arcIdMapOrig, arcIdMap).run();

    // sample the graph
    if (fixed){
        SampleFixed(g, wMap, source, target, sampleEdges, arcIdMap);
    } else {
        if (weighted){
            SampleWeightedRandom(g, wMap, source, target, samplingProb, sampleEdges, arcIdMap, chances);
        } else {
            SampleRandom(g, wMap, source, target, samplingProb, sampleEdges, arcIdMap);
        }
    }

    // post-sampling minimization
    minimizeGraph(g, wMap, arcIdMap, source, target);

    int numNodes = countNodes(g);
    int numEdges = countArcs(g);
    if (print) cout << numNodes << "\t" << numEdges << "\t";
    if (numEdges == 0){ // empty graph - source and target unreachable
        return 0.0;
    }

    vector<Cut> cuts;
    vector< pair< vector<int>, int > > edgeSubsets;
    FindSomeGoodCuts(g, source, target, cuts, edgeSubsets);

    double prob = Solve(g, wMap, source, target, cuts);
    return prob;
}

double SuccessProb(vector<int> subset, ListDigraph& gOrig, WeightMap& wMapOrig){
    double result = 1.0;
    FOREACH_STL(arcId, subset){
        result *= (1.0 - wMapOrig[gOrig.arcFromId(arcId)]);
    }END_FOREACH;
    return result + 0.001;
}

/* This function does random probing.
** Does multiple random sampling and selects the edge set that makes the least runtime on average
    if weighted = true: weighted by their sizes, disappearance probability and invloved cut size
** The resulting edges should be in sampleEdges
*/
void ProbeRandom(ListDigraph& gOrig, WeightMap& wMapOrig, ArcIntMap& arcIdMap,
                 ListDigraph::Node& sourceOrig, ListDigraph::Node& targetOrig,
                 double samplingProb, Edges_T& sampleEdges, int probeSize, int probeRepeats, bool weighted){

    vector<EdgeSubset> chancesOrig;
    if (weighted){
        vector<Cut> cuts;
        vector< pair< vector<int>, int > > edgeSubsets;
        FindSomeGoodCuts(gOrig, sourceOrig, targetOrig, cuts, edgeSubsets);

        //Compile a vector of EdgeSubset objects
        vector<EdgeSubset> ess;
        double minScore = 1000000000.0;
        int idCounter = 0;
        FOREACH_STL(subset, edgeSubsets){
            idCounter ++;
            EdgeSubset es;
            es.id = idCounter;
            es.subset = subset.first;
            es.cutsize = subset.second;
            es.successProb = SuccessProb(subset.first, gOrig, wMapOrig);
//            bool jump = false;
//            FOREACH_STL(arcId, es.subset){
//                ListDigraph::Arc arc = gOrig.arcFromId(arcId);
//                jump = jump || gOrig.source(arc) == sourceOrig || gOrig.target(arc) == targetOrig;
//            }END_FOREACH;
//            if (jump) continue; // we don't want any subset that is involved with wource or target
            ess.push_back(es);
            if (es.score() < minScore)
                minScore = es.score();
        }END_FOREACH;

        // form a chances vector
        FOREACH_STL(es, ess){
            int esChances = (int) ceil(es.score() / minScore);
            for (int i=0; i<esChances; i++)
                chancesOrig.push_back(es);
        }END_FOREACH;
    }

    map< string, vector<double> > durations;
    map<string, Edges_T> samples;
    for (int i=0; i<probeSize; i++){
        Edges_T sample;
        vector<EdgeSubset> chances = chancesOrig;
        iteration(gOrig, wMapOrig, arcIdMap, sourceOrig, targetOrig, false, samplingProb, sample, false, weighted, chances);
        string sampleString = sample.to_string<char,std::string::traits_type,std::string::allocator_type>();
        samples[sampleString] = sample;
        for (int j=0; j<probeRepeats; j++){
            cout << "." << sample.count();
            cout.flush();
            double startCPUTime = getCPUTime();
            iteration(gOrig, wMapOrig, arcIdMap, sourceOrig, targetOrig, true, samplingProb, sample, false, weighted, chances); //last two parameters don't matter here
            double duration = getCPUTime() - startCPUTime;
            durations[sampleString].push_back(duration);
        }
    }
    // get the min average time sample
    Edges_T minSample;
    double minAvg = 1000000000;
    for (map< string, vector<double> >::iterator it=durations.begin(); it != durations.end(); ++it){
        double avg = std::accumulate(it->second.begin(), it->second.end(), 0.0) / it->second.size();
        if (avg < minAvg){
            minAvg = avg;
            minSample = samples[it->first];
        }
    }
    sampleEdges = minSample;
}

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
    timeval time;
    gettimeofday(&time,NULL);
    srand48((time.tv_sec * 1000) + (time.tv_usec / 1000));

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

    for (int i=0; i<sampleSize; i++){
        double startCPUTime = getCPUTime();
        double prob = iteration(gOrig, wMapOrig, arcIdMapOrig, sourceOrig, targetOrig, fixedSample, samplingProb, sampleEdges, true, weighted, chances);
        double duration = getCPUTime() - startCPUTime;
        cout << prob << "\t" << duration << endl;
        total += prob;
    }
    cout << "#>>result = " << total/sampleSize << endl;
    return 0;
}
