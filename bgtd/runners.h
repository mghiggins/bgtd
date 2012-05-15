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
#include "doublestrat.h"

struct runStats
{
    double ppg, avgSteps, fracWin, absPpg, fracSingle, fracGammon, fracBg, avgCube;
};

double playParallel( strategytdbase& s1, strategy& s2, long n, long initSeed, long displayIndex, const string& fileSuffix, int nBuckets=1 );
runStats playParallelGen( strategy& s1, strategy& s2, long n, long initSeed, int nBuckets=1, bool varReduc=true );
double playSerial( strategytdbase& s1, strategy& s2, long n, long initSeed, long displayIndex, const string& fileSuffix, bool returnPpg=false );
runStats playSerialGen( strategy& s1, strategy& s2, long n, long initSeed );
void sim6( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix );
void sim7( int nMiddle, double alpha0, double beta0 );
void sim8( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix );
void sim9( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix );
void test4();
void testOrigGam();
void testHittingShots();
void testDoubleHittingShots();

void playBearoff();
void playBearoffOneSided();
void compareBearoff();

void playEscapes();

void createBenchmarks();
void trainBenchmarks();
void testBenchmark();

void testMatchEquity();

runStats playParallelCubeful( strategy& s1, strategy& s2, long n, long initSeed, int nBuckets, bool varReduc, int bktPrintInterval );
void testCubefulMoney();
void estimateJumpVol();

void testMatch();

void testContactClustering();

void testRollout();

void trainPubEval();
void testAvgEscapeCount();

#endif
