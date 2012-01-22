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

board strategy::preferredBoard( const board& oldBoard, const set<board>& possibleMoves, const hash_map<string,int>* context ) const
{
    if( possibleMoves.size() == 0 ) return oldBoard;
    if( possibleMoves.size() == 1 ) return *(possibleMoves.begin()); // only one move - choose it - no calcs req'd
    
    // create a context that merges the context based on the old board and whatever is passed in. The context
    // pass in externally overrides any values with the same key from the board context.
    
    hash_map<string,int> mergedContext( boardContext( oldBoard ) );
    if( context != 0 )
        for( hash_map<string,int>::const_iterator it=context->begin(); it!=context->end(); it++ )
            mergedContext[ it->first ] = it->second;
    
    // by default return the board with the highest board value
    
    board maxBoard;
    double maxVal=-1e99, val;
    
    for( set<board>::iterator i=possibleMoves.begin(); i!=possibleMoves.end(); i++ )
    {
        val = boardValue( (*i), &mergedContext );
        if( val > maxVal )
        {
            maxVal = val;
            maxBoard = (*i);
        }
    }
    
    return maxBoard;
}