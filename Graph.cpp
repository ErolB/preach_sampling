//
// Created by erol on 5/9/16.
//

#include "Graph.h"

using lemon::ListDigraph;

// Term class

string Term::toString(){
    ostringstream result;
    //result << coeff << "X" << x.to_string<char,std::string::traits_type,std::string::allocator_type>()
    //    << "Y" << y.to_string<char,std::string::traits_type,std::string::allocator_type>()
    //    << "Z" << z.to_string<char,std::string::traits_type,std::string::allocator_type>()
    //    << "W" << w.to_string<char,std::string::traits_type,std::string::allocator_type>();
    result << coeff << "X" << x.count() << "Y" << y.count() << "Z" << z.count() << "W" << w.count();
    return result.str();
}

void Term::multiply(int subscript, double p, bool inverse){
    if (inverse){
        y.set(subscript);
        coeff *= (1-p);
    }else{
        x.set(subscript);
        coeff *= p;
    }
}

double Term::getCoeff(){return coeff;}

void Term::addToCoeff(double increment){coeff += increment;}

bool Term::hasZ(){return z.any();}

bool Term::collapse(Edges_T& midEdges, map< int, vector<int> >& edgeTerminals, Nodes_T& endNodes, Nodes_T& newZ){
    int edges[MAX_EDGES][2];
    int count;
    vector<int> terminals;
    Nodes_T visited;
    Nodes_T copy;

    /////FIRST: traverse the x edges and see which end nodes are reachable
    count = 0;
    FOREACH_BS(edge, x){ // form the list of edges to traverse
        terminals = edgeTerminals[edge];
        edges[count][0] = terminals[0];
        edges[count][1] = terminals[1];
        count ++;
    }
    visited = z;
    while (true){ // traverse the edges, setting targets as true until nothing changes
        copy = visited;
        for (int i=0; i<count; i++){
            if (visited[edges[i][0]]){
                visited.set(edges[i][1]);
            }
        }
        if (copy == visited) break;
    }
    Nodes_T reachable = endNodes & visited; // This is now the set of sure reachable nodes

    /////SECOND: traverse all edges except y and see which end nodes are unreachable
    Edges_T yInverse = midEdges & ~y;
    count = 0;
    FOREACH_BS(edge, yInverse){
        terminals = edgeTerminals[edge];
        edges[count][0] = terminals[0];
        edges[count][1] = terminals[1];
        count ++;
    }
    visited = z;
    while (true){ // traverse the edges, setting targets as true until nothing changes
        copy = visited;
        for (int i=0; i<count; i++){
            if (visited[edges[i][0]]){
                visited.set(edges[i][1]);
            }
        }
        if (copy == visited) break;
    }
    Nodes_T unreachable = endNodes & ~visited; // This is now the set of sure unreachable nodes

    /////LAST: if all endNodes are either reachable or unreachable: collapsed
    /////else (at least one node is neither reachable nor unreachable): doesn't collapse
    if (endNodes == (reachable|unreachable)){
        newZ = reachable;
        return true;
    }else{
        return false;
    }
}

// Polynomial class

/*Adds and edge: mutiplies the whole polynomial by
    pX_subscript + (1-p)Y_subscript
    DOES NOT COLLAPSE AUTOMATICALLY*/
void Polynomial::addEdge(int subscript, double p){
    //a new vector to accumulate the new terms
    vector<Term> newTerms;
    if (p > 0.0000001){
        FOREACH_STL(term, terms){
                //Multiply by X term
                Term xTerm = term;
                xTerm.multiply(subscript, p, false);
                newTerms.push_back(xTerm);
            }END_FOREACH;
    }
    if (p < 0.999999){
        FOREACH_STL(term, terms){
                //Multiply by Y term
                Term yTerm = term;
                yTerm.multiply(subscript, p, true);
                newTerms.push_back(yTerm);
            }END_FOREACH;
    }
    //swap newTerms with terms
    newTerms.swap(terms);
}

double Polynomial::getResult(){
    //SANITY CHECKS
    if (terms.size() == 1){
        assert(terms.front().getCoeff() > 0.9999);
    } else {
        assert(terms.size() == 2);
        double totalCoeff = terms.front().getCoeff() + terms.back().getCoeff();
        assert(totalCoeff < 1.01 && totalCoeff > 0.99);
    }

    if (terms.front().hasZ()){
        assert(terms.size() == 1 || !terms.back().hasZ());
        return terms.front().getCoeff();
    }else{
        assert(terms.back().hasZ());
        return terms.back().getCoeff();
    }
}

/*Collapses the polynomial: iterates over each term
    to collapse it if possible.
    midEdges are all edges currently considered between
    the past cut and the next one.
    edgeTerminals is a hash from edge id to its source and target ids
    endNodes are the nodes in the next cut*/
void Polynomial::collapse(Edges_T& midEdges, map< int, vector<int> >& edgeTerminals, Nodes_T& endNodes){
    vector<Term> newTerms;
    FOREACH_STL(term, terms){
            // check collapsing of term
            Nodes_T z; // will hold the nodes to which the term collapses (Z)
            bool collapsed = term.collapse(midEdges, edgeTerminals, endNodes, z);
            if (collapsed){ // term DOES collapse to z
                // Now we find the corresponding endTerm, or create it
                Term endTerm;
                string endTermId = z.to_string< char,char_traits<char>,allocator<char> >();
                if (endTerms.find(endTermId) != endTerms.end()){ // endTerm found
                    endTerm = endTerms[endTermId];
                }else{ // endTerm not found, create it
                    Nodes_T w = endNodes & ~z;
                    endTerm = Term(z, w);
                }
                endTerm.addToCoeff(term.getCoeff());
                endTerms[endTermId] = endTerm;
            }else { // term DOES NOT collapse
                newTerms.push_back(term);
            }
        }END_FOREACH;
    newTerms.swap(terms); //replace terms with the new collapsed terms
}

// stand-alone functions

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

/*Gets all edges as a bitset*/
void EdgesAsBitset(ListDigraph& g, Edges_T& edges){
    for (ListDigraph::ArcIt arc(g); arc != INVALID; ++arc){
        edges.set(g.id(arc));
    }
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






