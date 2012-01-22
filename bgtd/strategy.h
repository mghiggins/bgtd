//
//  strategy.h
//  bgtd
//
//  Created by Mark Higgins on 7/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

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
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 ) const = 0;
    virtual board preferredBoard( const board& oldBoard, const set<board>& possibleMoves, const hash_map<string,int>* context=0 ) const;
    virtual hash_map<string,int> boardContext( const board& brd ) const { hash_map<string,int> empty; return empty; };
};

#endif
