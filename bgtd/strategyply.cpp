//
//  strategyply.cpp
//  bgtd
//
//  Created by Mark Higgins on 12/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "strategyply.h"

strategyply::strategyply( int nPlies, strategy& baseStrat ) : baseStrat( baseStrat )
{
    this->nPlies = nPlies;
}

board strategyply::preferredBoard( const board& oldBoard, const set<board>& possibleMoves ) const
{
    
}