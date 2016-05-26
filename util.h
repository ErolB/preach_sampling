//
// Created by erol on 5/22/16.
//

#ifndef PREACH_SAMPLING_UTIL_H
#define PREACH_SAMPLING_UTIL_H

#include <random>
#include <sys/time.h>

void initRand();

double nextRand();

//get cpu time in milliseconds
double getCPUTime();


#endif //PREACH_SAMPLING_UTIL_H
