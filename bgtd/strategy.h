//
//  strategy.h
//  bgtd
//
//  Created by Mark Higgins on 7/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategy_h
#define bgtd_strategy_h

#include "board.h"

class strategy
{
    // Base class for strategies
    
public:
    strategy() {};
    virtual ~strategy() {};
    
    virtual double boardValue( const board& brd ) const = 0;
    
    // some strategies will incrementally update themselves each step of 
    // the game (eg the neural net strategy for learning). For those, in
    // derived classes, override needsUpdate to return true when appropriate,
    // and make update do the update given the state of the board before and
    // after the move. The game class, in its step method, checks the strategy
    // for needsUpdate and passes the boards to update.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
};

#endif
