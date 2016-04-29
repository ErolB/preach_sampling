//
// Contains functions for preprocessing a graph before calculating reachability
//

#include "graph_util.h"

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

