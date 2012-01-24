//
//  strategyply.h
//  bgtd
//
//  Created by Mark Higgins on 12/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategyply_h
#define bgtd_strategyply_h

#include "strategy.h"

class strategyply : public strategy
{
    // strategy that holds another strategy internally, and uses it to look ahead
    // a certain number of "plies". One ply is one turn by one player.
    
public:
    // nPlies is the number of plies to search; nMoveFilter is the # of moves to include
    // when doing deeper searches; baseStrat is the underlying strategy we 
    // use to dig into the tree; filterStrat is the strategy we use to come up with 
    // nMoveFilter-element list that we calculate a more accurate board value for 
    // (filterStrat is usually a fairly coarse strategy that gets equity correct only
    // approximately).
    
    strategyply( int nPlies, int nMoveFilter, strategy& baseStrat, strategy& filterStrat );
    virtual ~strategyply() {};
    
    int nPlies, nMoveFilter;
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 ) const;
    double boardValueRecurse( const board& brd, int stepNPlies, const hash_map<string,int>* context ) const;
    
private:
    strategy& baseStrat;
    strategy& filterStrat;
};

#endif
