//
//  runners.h
//  bgtd
/*****************
 Copyright 2011, 2012 Mark Higgins
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 *****************/


#ifndef bgtd_runners_h
#define bgtd_runners_h

#include "strategytdbase.h"

struct runStats
{
    double ppg, avgSteps, fracWin, absPpg, fracSingle, fracGammon, fracBg;
};

void writeWeightsToFiles( const vector<double>& outputWeights, const vector< vector<double> >& middleWeights, const vector<double>& outputTraces, const vector< vector<double> >& middleTraces, const string& fileSuffix );
void readWeightsFromFile( vector<double>& outputWeights, vector< vector<double> >& middleWeights, vector<double>& outputTraces, vector< vector<double> >& middleTraces, const string& fileSuffix );
void writeExpWeightsToFiles( const vector<double>& outputProbWeights, const vector<double>& outputGammonWeights, 
                             const vector< vector<double> >& middleWeights, 
                             const vector<double>& outputProbTraces, const vector<double>& outputGammonTraces, 
                             const vector< vector<double> >& middleProbTraces, const vector< vector<double> >& middleGammonTraces, 
                             const string& fileSuffix );
void writeExp2WeightsToFiles( const vector<double>& outputProbWeights, const vector<double>& outputGammonWinWeights, const vector<double>& outputGammonLossWeights, 
                              const vector< vector<double> >& middleWeights, const string& fileSuffix );
void writeExp3WeightsToFiles( const vector<double>& outputProbWeights, const vector<double>& outputGammonWinWeights, const vector<double>& outputBgWinWeights,
                              const vector< vector<double> >& middleWeights, const string& fileSuffix );
void writeExp4WeightsToFiles( const vector<double>& outputProbWeights, const vector<double>& outputGammonWinWeights, const vector<double>& outputGammonLossWeights,
                              const vector< vector<double> >& middleWeights, const string& fileSuffix );
void readExpWeightsFromFile( vector<double>& outputProbWeights, vector<double>& outputGammonWeights, 
                             vector< vector<double> >& middleWeights, 
                             vector<double>& outputProbTraces, vector<double>& outputGammonTraces, 
                             vector< vector<double> >& middleProbTraces, vector< vector<double> >& middleGammonTraces, 
                             const string& fileSuffix );
double playParallel( strategytdbase& s1, strategy& s2, long n, long initSeed, long displayIndex, const string& fileSuffix );
runStats playParallelGen( strategy& s1, strategy& s2, long n, long initSeed );
double playSerial( strategytdbase& s1, strategy& s2, long n, long initSeed, long displayIndex, const string& fileSuffix, bool returnPpg=false );
double playSerialGen( strategy& s1, strategy& s2, long n, long initSeed );
void sim1( int nMiddle, double alpha0, double beta0, const string& fileSuffix );
void sim2( int nMiddle, double alpha0, double beta0, const string& fileSuffix );
void sim3( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix );
void sim4( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix );
void sim5( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix );
void sim6( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix );
void sim7( int nMiddle, double alpha0, double beta0 );
void sim8( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix );
void sim9( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix );
void test1();
void test2();
void test3();
void test4();
void testOrigGam();
void testHittingShots();

void printWeights3( const string& srcSuffix );

void playBearoff();
void playBearoffOneSided();
void compareBearoff();

void playEscapes();


#endif
