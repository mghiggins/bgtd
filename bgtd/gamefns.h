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
// be consistent with strategy boardValue calculations.

gameProbabilities rolloutBoardProbabilities( const board& brd, strategyprob& strat, int nRuns, int seed );
gameProbabilities rolloutBoardProbabilities( const board& brd, strategyprob& strat, int nRuns, CRandomMersenne * rng );

// rolloutBoardProbabilitiesVarReduction explicitly sets the first set of rolls to the 21
// possible dice rolls, then does the usual MC simulation under that, to reduce
// variance. nRuns must be a multiple of 21.

gameProbabilities rolloutBoardProbabilitiesVarReduction( const board& brd, strategyprob& strat, int nRuns, int seed );
gameProbabilities rolloutBoardProbabilitiesVarReduction( const board& brd, strategyprob& strat, int nRuns, CRandomMersenne * rng );

// rolloutBoardProbabilitiesParallel does the same thing but breaks the runs into nThreads
// threads and runs them in parallel. if varReduc is true it will use the variance-reduced
// version, and nRuns must be a multiple of 21*nThreads.

gameProbabilities rolloutBoardProbabilitiesParallel( const board& brd, strategyprob& strat, int nRuns, int seed, int nThreads, bool varReduc );

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

// primesCount counts the max number of points in a prime. forPlayer==true
// means it does it for the player; false means for the opponent. Only
// counts on the appropriate side of the board (ie for the player, it
// ignores any primes on the opponent's half of the board).

int primesCount( const board& brd, bool forPlayer );

// sigmoid returns the sigmoid fn 1/(1+exp(-x)), using a fast approximation

double sigmoid( double x );

#endif
