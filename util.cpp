//
// Created by erol on 5/22/16.
//

#include "util.h"

//std::random_device theRandomDevice;
std::mt19937 theRandomMT;
std::uniform_real_distribution<double> theRandomGenerator;

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