//
//  gamefns.h
//  bgtd
//
//  Created by Mark Higgins on 7/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_gamefns_h
#define bgtd_gamefns_h

#include <set>
#include "board.h"
#include "strategy.h"

using namespace std;

// possibleMoves gives all the possible moves (as a set of boards)
// that one could make given the two dice. Only legal moves.

set<board> possibleMoves( const board& brd, int die1, int die2 );

// isRace returns true if the board is in a race condition (no
// contact) and false otherwise.

bool isRace( const board& board );

// rolloutBoardValue runs a Monte Carlo simulation to get a more accurate
// estimate of the board value, using the supplied strategy to
// figure out the moves. Assumes we start on the opponent's roll to
// be consistent with strategy boardValue calculations.

double rolloutBoardValue( const board& brd, strategy& strat, int nRuns, int seed );

// rolloutBoardValueParallel does the same thing but breaks the runs into nThreads
// threads and runs them in parallel.

double rolloutBoardValueParallel( const board& brd, strategy& strat, int nRuns, int seed, int nThreads );

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

double hittingProb( const board& brd, bool forOpponent );
double hittingProb( const set<roll>& shots );

// primesCount counts the max number of points in a prime. forPlayer==true
// means it does it for the player; false means for the opponent. Only
// counts on the appropriate side of the board (ie for the player, it
// ignores any primes on the opponent's half of the board).

int primesCount( const board& brd, bool forPlayer );

#endif
