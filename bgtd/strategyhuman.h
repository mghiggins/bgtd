//
//  strategyhuman.h
//  bgtd
//
//  Created by Mark Higgins on 8/5/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategyhuman_h
#define bgtd_strategyhuman_h

#include "strategy.h"

class strategyhuman : public strategy
{
public:
    strategyhuman() {};
    virtual ~strategyhuman() {};
    
    // the boardValue always returns 0 in this strategy because it never
    // gets used. Instead, preferredBoard lets them enter the choice of
    // board and makes sure what they chose is in the list of allowed
    // moves.
    
    virtual double boardValue( const board& brd ) const { return 0.; };
    virtual board preferredBoard( const board& oldBoard, const set<board>& possibleMoves ) const;
};

#endif
