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
    // nPlies is the number of plies to search; baseStrat is the underlying strategy we 
    // use to dig into the tree.
    
    strategyply( int nPlies, strategy& baseStrat );
    virtual ~strategyply() {};
    
    int nPlies;
    
    // boardValue will look up the look-ahead estimate of the board probability, but
    // will do it as a consequence of the real calculation that's in preferredBoard.
    // For games, boardValue is useful only for interesting diagnostic information;
    // it's not used directly to decide moves.
    
    virtual double boardValue( const board& brd ) const { return 0.; };
    virtual board preferredBoard( const board& oldBoard, const set<board>& possibleMoves ) const;
    
private:
    strategy& baseStrat;
};

#endif
