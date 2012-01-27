//
//  bearofffns.h
//  bgtd
//
//  Created by Mark Higgins on 10/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_bearofffns_h
#define bgtd_bearofffns_h

#import <hash_map.h>
#import "board.h"
#include "common.h"

// boardID returns a unique ID for the board (from the perspective of 
// the board, assuming the player with perspective has the dice), given
// that we're doing two-sided bearoff for nPnts points.

string boardID( const board& b, int nPnts );

// boardFromID takes the unique ID and returns the board

board boardFromID( const string& ID );

// boardPnts returns a map of board ID->expected number of points the
// player with the perspective will win (negative if he expects to lose).
// This map is filled in either by loading from a file or dynamically.

hash_map<string,double> * boardPnts();

// getBoardPnt calculates the expected number of points the player with
// perpective will win by doing a full rollout. Uses existing elements
// in the boardPnts hash if they're there, otherwise it recursively fills
// them in. Always fills in the point in the hash for this board.

double getBoardPnt( const board& b, int nPnts );

// constructBearoff constructs the bearoff database out to nPnts points
// and nCheckers max checkers. Results are stored in the local board ID
// hash.

void constructBearoff( int nPnts, int nCheckers );

// writeBearoffDb writes the data in the internal hash map to a file

void writeBearoffDb( const string& fileName );

// loadBearoffDb reads the bearoff data from a file into the internal hash map

void loadBearoffDb( const string& fileName );

// define a class to hold a (steps,prob) pair

class stepProbability
{
public:
    stepProbability() : nStep(0), prob(0) {};
    stepProbability( int nStep, double prob ) : nStep(nStep), prob(prob) {};
    int nStep;
    double prob;
};

// stepProbabilityCompare returns true if nStep for the first element is less than the second

bool stepProbabilityCompare( const stepProbability& s1, const stepProbability& s2 );

// stepProbabilityCompareProbOrdered returns true if prob for the first element is bigger than prob for the second

bool stepProbabilityCompareProbOrdered( const stepProbability& s1, const stepProbability& s2 );

// define a class to hold the probability of steps distribution

class stepsDistribution
{
public:
    stepsDistribution() {};
    stepsDistribution( const stepsDistribution& otherDist );
    
    // pairs is ordered by the # of steps - so returns pairs of (# of steps, prob of finishing in that #)
    
    vector<stepProbability> pairs() const; 
    
    // pairsProb is oredered by the probability - same pairs as "pairs" but in descending order of prob
    
    vector<stepProbability> pairsProbOrdered() const;
    
    hash_map<int,double> stepProbs; // hash of number of steps->probability of finishing in that number of steps
};

// boardIDOneSided returns a unique ID for the board (from the perspective of
// the player with the dice), assuming we're doing a one sided bearoff for
// nPnts points.

string boardIDOneSided( const board& b, int nPnts );

// stepsProbs returns a pointer to the underlying hash that holds the
// board ID->probability distribution map.

hash_map<string,stepsDistribution> * stepsProbs();

// getStepsDistribution returns the distribution of number of steps until the player's
// last piece is taken off, using the one sided bearoff db with nPnts max
// points. maxElem refers to the max # of probability elements we track - if the
// calculated number is more than maxElem, the smallest-prob elements are removed
// and the remaining probabilities renormalized.

stepsDistribution getStepsDistribution( const board& b, int nPnts, int maxElem=0, hash_map<string,stepsDistribution> * cache=0 );

// getFinishProb is the prob of 1 finishing before 2. move1 = true if 1 is one move, false if 2 is on move.
// pairs1 and pairs2 represent the distribution of # of moves of each player to finish (whatever we choose
// "finish" to mean). Assumes the distributions are independent.

double getFinishProb( const vector<stepProbability>& pairs1, const vector<stepProbability>& pairs2, bool move1 );

// getProbabilityWin returns the probability of any kind of win (normal or gammon), using
// the one-sided bearoff database

double getProbabilityWin( const board& b, int nPnts );

// getProbabilityGammonWin returns the probability of a gammon win, using the two one-sided
// bearoff databases

double getProbabilityGammonWin( const board& b, int nPnts );

// getProbabilityGammonLoss returns the probability of a gammon loss, using the two one-sided dbs

double getProbabilityGammonLoss( const board& b, int nPnts );

// getBoardPntOS gets the expected number of points that a player will win using
// the one-sided bearoff dbs (points to take off final checker and points to take
// off first checker). Includes the possibility of gammons.

double getBoardPntOS( const board& b, int nPnts );

// constructBearoffOneSided constructs the bearoff database out to nPnts points
// and nCheckers max checkers. maxElem defines the max # of probability entries
// we'll track for each position. Results are stored in the local hash.

void constructBearoffOneSided( int nPnts, int nCheckers, int maxElem );

// constructBearoffOneSidedParllel does the same as constructBearoffOneSided,
// but parallelizes the calculations. It generates nThreads threads; each thread
// do calcsPerThread calcs, then aggregates its calculated info into the shared
// hash.

void constructBearoffOneSidedParallel( int nPnts, int nCheckers, int maxElem, int nThreads, int calcsPerThread );

// writeBearoffDbOneSided writes the one sided bearoff data into a file from
// the internal hash map.

void writeBearoffDbOneSided( const string& fileName );

// loadBearoffOneSided reads the one sided bearoff data from a file into the
// internal hash map.

void loadBearoffDbOneSided( const string& fileName );

// gamStepsProbs returns a pointer to the underlying hash that holds the
// board ID->distribution of number of steps until the first checker is
// taken in

hash_map<string,stepsDistribution> * gamStepsProbs();

// getGammonStepsDistribution returns the distribution of the number of steps
// until the player's *first* checker is taken off; otherwise like getStepsDistribution.

stepsDistribution getGammonStepsDistribution( const board& b, int nPnts, int maxElem=0 );

// constructGammonBearoffOneSided constructs the one-sided "gammon" bearoff db,
// which tracks the distribution of steps until the first checker is taken off. Otherwise
// like constructBearoffOneSided. No need to pass nCheckers because this has to be
// 15; otherwise we've already taken off a piece.

void constructGammonBearoffOneSided( int nPnts, int maxElem );

// writeGammonBearoffDbOneSided writes the "gammon" version of the one-sided bearoff db

void writeGammonBearoffDbOneSided( const string& fileName );

// loadGammonBearoffDbOneSided loads the "gammon" version of the one-sided bearoff db

void loadGammonBearoffDbOneSided( const string& fileName );

// usual factorial

long factorial(int n);

// number of elements in the two-sided bearoff database

long nElementsTS( int nPnts, int nCheckers );

// number of elements in the one-sided bearoff database

long nElementsOS( int nPnts, int nCheckers );

// Next database: this one is used for counting escaping rolls against a blockade.
// The blockade is distributed across twelve points in front of the checker, with
// one to seven points of the twelve covered.

// blockadeID returns an integer that defines the blockage - between 1 and 
// 2^12=4096. Returns from the player's perspective, starting at point i (0-based).

int blockadeID( const board& b, int startPoint );

// blockadeEscapeRolls() returns a pointer to the database

hash_map<int,int> * blockadeEscapeRolls();

// getBlockadeEscapeCount returns an integer representing the number of
// escaping rolls for the player starting at point startPoint against an opponent
// blockade.

int getBlockadeEscapeCount( const board& b, int startPoint );

// constructBlockadeEscapeDb constructs the blockade db and stores it in memory.
// This is very fast to run, so you should do it on session startup.

void constructBlockadeEscapeDb();

#endif
