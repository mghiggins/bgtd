//
//  strategy.cpp
//  bgtd
//
//  Created by Mark Higgins on 7/24/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

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