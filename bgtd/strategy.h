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

#ifndef bgtd_strategy_h
#define bgtd_strategy_h

#import <hash_map.h>
#include <set>
#include "board.h"
#include "common.h"

class strategy
{
    // Base class for strategies
    
public:
    strategy() {};
    virtual ~strategy() {};
    
    // some strategies will incrementally update themselves each step of 
    // the game (eg the neural net strategy for learning). For those, in
    // derived classes, override needsUpdate to return true when appropriate,
    // and make update do the update given the state of the board before and
    // after the move. The game class, in its step method, checks the strategy
    // for needsUpdate and passes the boards to update.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    
    // boardValue is the key method on the strategy; it takes a board and returns
    // a value, using whatever method is appropriate. The game proceeds by
    // choosing the board with the highest value from the possible moves. The
    // game's step method ends up calling preferredBoard, which by default
    // returns the board with the highest value. However, you can also have an
    // interactive strategy where a user enters their choice for the move directly.
    // NOTE: boardValue is meant to return the value of the board *after* the player
    // with perspective has moved, so when their opponent has the dice. Also takes
    // an optional pointer to a hash of string->int that holds context for the 
    // board value. For example, pubeval uses this to tell the evaluator whether to
    // use the race or contact regressions; and the game can tell the board to
    // evaluate based on just prob of win (eg in a match one game from completion).
    // boardValueContext returns a map specific to that strategy (by default it returns
    // an empty hash).
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 ) = 0;
    virtual board preferredBoard( const board& oldBoard, const set<board>& possibleMoves, const hash_map<string,int>* context=0 );
    virtual hash_map<string,int> boardContext( const board& brd ) const { hash_map<string,int> empty; return empty; };
    
    // offerDouble and takeDouble use the underlying doubling strategy to determine cube actions.
    // Default is to never double and always take. Refined in derived classes.
    
    virtual bool offerDouble( const board& brd, int cube ) { return false; };
    virtual bool takeDouble( const board& brd, int cube ) { return true; };
};

#endif
