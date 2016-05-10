//
// Created by erol on 5/9/16.
//

#include "preach.h"
#include "Graph.h"
#include "Cut.h"

/*Prints the graph in node IDs*/
void PrintGraph(ListDigraph& g){
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        cout << g.id(g.source(arc)) << " " << g.id(g.target(arc)) << endl;
    }
}

//verifies the existence of an edge
bool EdgeExists(ListDigraph& g, ListDigraph::Node& source, ListDigraph::Node& target){
    for (ListDigraph::OutArcIt arc(g, source); arc != INVALID; ++arc){
        if (g.id(g.target(arc)) == g.id(target))
            return true;
    }
    return false;
}

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

void minimizeGraph(ListDigraph& g, WeightMap& wMap, ArcIntMap& arcIdMap, ListDigraph::Node& source, ListDigraph::Node& target){
    RemoveIsolatedNodes(g, source, target);
    CollapseELementaryPaths(g, wMap, arcIdMap, source, target);
    RemoveSelfCycles(g);
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





