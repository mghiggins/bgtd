//
//  strategy.cpp
//  bgtd
//
//  Created by Mark Higgins on 7/24/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <string>
#include "strategy.h"

bool strategy::needsUpdate() const
{
    return false;
}

void strategy::update( const board& oldBoard, const board& newBoard )
{
    // doesn't do anything by default - override in derived classes if you 
    // want this to update something.
}

board strategy::preferredBoard( const board& oldBoard, const set<board>& possibleMoves ) const
{
    if( possibleMoves.size() == 0 ) return oldBoard;
    if( possibleMoves.size() == 1 ) return *(possibleMoves.begin()); // only one move - choose it - no calcs req'd
    
    // by default return the board with the highest board value
    
    board maxBoard;
    double maxVal=-1e99, val;
    
    for( set<board>::iterator i=possibleMoves.begin(); i!=possibleMoves.end(); i++ )
    {
        val = boardValue( (*i) );
        if( val > maxVal )
        {
            maxVal = val;
            maxBoard = (*i);
        }
    }
    
    return maxBoard;
}