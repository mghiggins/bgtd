//
//  runners.h
//  bgtd
//
//  Created by Mark Higgins on 7/30/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_runners_h
#define bgtd_runners_h

#include "strategytd.h"

void writeWeightsToFiles( const vector<double>& outputWeights, const vector< vector<double> >& middleWeights, const vector<double>& outputTraces, const vector< vector<double> >& middleTraces, const string& fileSuffix );
void readWeightsFromFile( vector<double>& outputWeights, vector< vector<double> >& middleWeights, vector<double>& outputTraces, vector< vector<double> >& middleTraces, const string& fileSuffix );
double playSerial( strategytd& s1, long n, long initSeed, long displayIndex, const string& fileSuffix );
void sim1( int nMiddle, double alpha0, double beta0, const string& fileSuffix );
void test1();

#endif
