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

#ifndef bgtd_gamefns_h
#define bgtd_gamefns_h

#include <set>
#include "board.h"
#include "strategyprob.h"
#include "randomc.h"

using namespace std;

// possibleMoves gives all the possible moves (as a set of boards)
// that one could make given the two dice. Only legal moves.

set<board> possibleMoves( const board& brd, int die1, int die2 );

// isRace returns true if the board is in a race condition (no
// contact) and false otherwise.

bool isRace( const board& board );

// rolloutBoardProbabilities runs a Monte Carlo simulation to get a more accurate
// estimate of the board probabilities, using the supplied strategy to
// figure out the moves. Assumes we start on the opponent's roll to
// be consistent with strategy boardValue calculations. depth=0 means run to end;
// otherwise it stops after depth plies.

gameProbabilities rolloutBoardProbabilities( const board& brd, strategyprob& strat, int nRuns, int seed, int depth=0 );
gameProbabilities rolloutBoardProbabilities( const board& brd, strategyprob& strat, int nRuns, CRandomMersenne * rng, int depth=0 );

// rolloutBoardProbabilitiesVarReduction luck-adjusts the Monte Carlo simulation at each step

gameProbabilities rolloutBoardProbabilitiesVarReduction( const board& brd, strategyprob& strat, int nRuns, int seed, int depth=0 );
gameProbabilities rolloutBoardProbabilitiesVarReduction( const board& brd, strategyprob& strat, int nRuns, CRandomMersenne * rng, int depth=0 );

// rolloutBoardProbabilitiesParallel does the same thing but breaks the runs into nThreads
// threads and runs them in parallel. 

gameProbabilities rolloutBoardProbabilitiesParallel( const board& brd, strategyprob& strat, int nRuns, int seed, int nThreads, bool varReduc, int depth=0 );

// roll is a simple class that holds a two-dice roll

class roll
{
public:
    roll( int die1, int die2 ) : die1(die1), die2(die2) {};
    int die1, die2;
    
    bool operator==( const roll& otherRoll ) const { return die1==otherRoll.die1 and die2==otherRoll.die2; };
    bool operator<( const roll& otherRoll ) const { return die1<otherRoll.die1 or ( die1==otherRoll.die1 and die2<otherRoll.die2 ); };
};

// hittingShots calculates the rolls of the opponent that would hit
// any of a player's blots if forOpponent=true and the reverse if
// forOpponent=false.

set<roll> hittingShots( const board& brd, bool forOpponent );

// hittingProb returns the probability of getting a hitting roll; takes
// either a board or a pre-calculated set of shots.

void setupHittingRolls();

double hittingProb( const board& brd, bool forOpponent );
double hittingProb( const set<roll>& shots );
double hittingProb2( const board& brd, bool forOpponent );

// hittingProbBar returns the probability that the player would hit an opponent blot in the opponent
// home board if the player had an opponent on the bar (doesn't matter if he really does).

double hittingProbBar( const board& brd, bool forOpponent );

// doubleHittingShots calculates the rolls of the opponent that would hit
// any of a player's blots and also cover up. Same convention as hittingShots
// for forOpponent argument. If noBlot is true, it ignores rolls that would
// leave a new blot. minIndex and maxIndex set the range for the checkers
// we'll check for being hit (0-based, so runs from 0->23).

set<roll> doubleHittingShots( const board& brd, bool forOpponent, bool noBlot, int minIndex, int maxIndex );
double doubleHittingProb( const board& brd, bool forOpponent, bool noBlot, int minIndex, int maxIndex );

// barAverageEscape measures the expected escape weighted by the probability of landing in
// various opens if rolling in from the bar. A number from zero to 36.

double barAverageEscape( const board& brd, bool forOpponent );

// primesCount counts the max number of points in a prime. forPlayer==true
// means it does it for the player; false means for the opponent. Only
// counts on the appropriate side of the board (ie for the player, it
// ignores any primes on the opponent's half of the board).

int primesCount( const board& brd, bool forPlayer );

// sigmoid returns the sigmoid fn 1/(1+exp(-x)), using a fast approximation

double sigmoid( double x );

// moveDiff takes a start and end board and returns a string describing
// the move. eg "24/18 15/11" would mean taking a checker from (player) point
// 24 and moving it to point 18, then another checker from 15 to 11.

string moveDiff( const board& startBoard, const board& endBoard );

#endif
