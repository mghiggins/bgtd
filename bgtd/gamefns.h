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

using namespace std;

// possibleMoves gives all the possible moves (as a set of boards)
// that one could make given the two dice. Only legal moves.

set<board> possibleMoves( const board& brd, int die1, int die2 );

// isRace returns true if the board is in a race condition (no
// contact) and false otherwise.

bool isRace( const board& board );

#endif