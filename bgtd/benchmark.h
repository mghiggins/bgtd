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


#ifndef bgtd_benchmark_h
#define bgtd_benchmark_h

#include "strategyprob.h"
#include "strategytdmult.h"

class boardAndRolloutProbs
{
public:
    boardAndRolloutProbs() {};
    boardAndRolloutProbs( const board& b, const gameProbabilities& probs ) : b(b), probs(probs) {};
    
    board b;
    gameProbabilities probs;
};

class benchmarkData
{
public:
    benchmarkData( const board& startBoard, int die1, int die2, const board& bestBoard, double bestEquity, const vector<board>& otherBoards, const vector<double>& otherEquities )
    : startBoard(startBoard), die1(die1), die2(die2), bestBoard(bestBoard), bestEquity(bestEquity), otherBoards(otherBoards), otherEquities(otherEquities) {};
    
    board startBoard;
    int die1, die2;
    board bestBoard;
    double bestEquity;
    vector<board> otherBoards;
    vector<double> otherEquities;
};

// Benchmark idea: we run a bunch of games to generate a set of benchmarks to run supervised learning against.
// At each point in the game, for a given roll, we generate the list of possible moves and
// evaluate each at 0-ply and 2-ply. When 0-ply and 2-ply choose different moves and the 
// equity difference in 2-ply btw the moves is bigger than a threshold, we include the two
// best-move choices (0-ply and 2-ply) in the benchmark set. Then for all entries in the set
// we roll out the board probabilities and store them. We then run supervised learning to
// train the network against those rollout values to try to improve the network directly instead
// of going through the TD steps.

// generateBenchmarkPositions saves out the board (serialized as a string) as well as the
// 0-ply and 2-ply board probabilities calculated for each. We write out benchmarks in files
// in a given directory, nFileBenchmarks benchmarks per file. Segregated like this to make it easier to
// randomly select benchmarks in supervised learning. It also writes an info file to note all the 
// names of of benchmark files. "strat" below is meant to be the 0-ply strategy; the 2-ply version is
// created internally along with the filter strategy filterStrat.

void generateBenchmarkPositions( strategyprob& strat, strategyprob& filterStrat, int nGames, const string& pathName, int nFileBenchmarks, int seed, int nThreads, int initIndex );
vector<string> generateBenchmarkPositionsSerial( strategyprob& strat, strategyprob& filterStrat, int nGames, const string& pathName, int nFileBenchmarks, int seed, int fileSuffix );

// rolloutBenchmarkPositions rolls out each of the benchmark positions already saved in the path
// pathName.

void rolloutBenchmarkPositions( strategyprob& strat, const string& pathName, int nFileBenchmarks, int nRuns, int seed, int nThreads, int initFileIndex );

// trainMult trains a strategytdmult set of networks on the rolled-out benchmark positions.
// Does one step through the available rolled-out benchmarks, using learning rates alpha and
// beta attached to the strategy.

void trainMult( strategytdmult& strat, const string& pathName, int seed );

// printErrorStatistics runs through the rollout database and calculates statistics on
// the diffs between the equity calculated by the strategy and the rolled-out equities.

void printErrorStatistics( strategytdmult& strat, const string& pathName );

// gnuBgBenchmarkStates loads the benchmark database from a file

vector<boardAndRolloutProbs> gnuBgBenchmarkStates( const string& fileName );

// trainMultGnuBG trains using a gnubg training file. Randomly orders the states and incrementally
// updates the weights after each element.

void trainMultGnuBg( strategytdmult& strat, const vector<boardAndRolloutProbs>& states, int seed );

// trainMultGnuBgParallel trains using a gnubg training file. Breaks the training set up into
// nBuckets buckets and calculates net weight update across each bucket assuming
// partial derivatives don't change.

void trainMultGnuBgParallel( strategytdmult& strat, const vector<boardAndRolloutProbs>& states, int nBuckets );

void printErrorStatisticsGnuBg( strategytdmult& strat, const vector<boardAndRolloutProbs>& states );

vector< vector<benchmarkData> > loadBenchmarkData( const string& fileName, int nBuckets );
double gnuBgBenchmarkStatisticsSerial( strategy& strat, const vector<benchmarkData>& benchmarks, hash_map<string,int> * ctx=0 );
double gnuBgBenchmarkER( strategy& strat, const vector< vector<benchmarkData> >& dataSet, hash_map<string,int> * ctx=0 );

#endif
