//
// Created by erol on 5/22/16.
//

#include "util.h"

//std::random_device theRandomDevice;
std::mt19937 theRandomMT;
std::uniform_real_distribution<double> theRandomGenerator;

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

void initRand(){
    //theRandomMT = std::mt19937(theRandomDevice);
    timeval time;
    gettimeofday(&time,NULL);
    theRandomMT.seed((time.tv_sec * 1000) + (time.tv_usec / 1000));
    theRandomGenerator = std::uniform_real_distribution<double>(0.0, 1.0);
}
double nextRand(){return theRandomGenerator(theRandomMT);}

//get cpu time in milliseconds
double getCPUTime(){
    return (double)clock() / (CLOCKS_PER_SEC/1000);
}