//
//  gamefns.cpp
//  bgtd
//
//  Created by Mark Higgins on 7/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <algorithm>
#include <map>
#include <iostream>
#include "gamefns.h"

set<board> subPossibleMoves( const board& brd, vector<int> rolls );

set<board> subPossibleMoves( const board& brd, vector<int> rolls )
{
    // local fn called recursively for each roll
    
    set<board> moves;
    
    int roll = rolls.at(0);
    
    // if there are any checkers hit, try to get them in. If they can't
    // get in, there's nothing to do.
    
    if( brd.hit() > 0 )
    {
        // see if there's a free spot in the enemy's home board
        
        if( brd.otherChecker( 24-roll ) > 1 )
            return moves; // nothing one can do
        
        // create a new board we'll use to track this move
        
        board newBrd( brd );
        
        // set the checker onto the spot, since it just moved in from the bar
        
        newBrd.setChecker( 24-roll, brd.checker(24-roll)+1 );
        newBrd.decrementHit();
        
        
        // if there was an enemy checker there, remove it to the bar
        
        if( brd.otherChecker( 24-roll ) == 1 )
        {
            newBrd.setOtherChecker( 24-roll, 0 );
            newBrd.incrementOtherHit();
        }
        
        // if there are no more rolls, return just this board
        
        if( rolls.size() == 1 )
        {
            moves.insert( newBrd );
            return moves;
        }
        
        // get the moves from here
        
        vector<int> subRolls;
        subRolls.insert( subRolls.end(), rolls.begin() + 1, rolls.end() );
        
        set<board> nextMoves = subPossibleMoves( newBrd, subRolls );
        if( nextMoves.size() == 0 )
            moves.insert( newBrd );
        else
            moves.insert( nextMoves.begin(), nextMoves.end() );
        
        return moves;
    }
    
    // deal with the first roll - try moving each available piece
    
    int i, j;
    bool skip;
    
    for( i=0; i<24; i++ )
    {
        // if there isn't a piece there, skip the spot
        
        if( brd.checker( i ) == 0 ) continue;
        
        board newBrd( brd );

        // break up into spots where you can move the piece inside the board,
        // and those where moving would mean bearing off
        
        if( i > roll-1 )
        {
            // deal with the subset of moves where you can move the piece without
            // leaving the board.
            
            // if there's a piece but there's nowhere to move it, skip the spot
            
            if( brd.otherChecker( i-roll ) > 1 ) continue;
            
            // okay to move there; set up a new board with the piece moved
            
            newBrd.setChecker( i-roll, brd.checker( i-roll ) + 1 );
            newBrd.setChecker( i, brd.checker(i) - 1 );
            
            // if there is a single enemy there, hit him
            
            if( brd.otherChecker( i-roll ) == 1 )
            {
                newBrd.setOtherChecker( i-roll, 0 );
                newBrd.incrementOtherHit();
            }
        }
        else
        {
            // then moves where you might be able to bear off
            
            // start with seeing whether there are any pieces outside the home box - if so, nothing to do
            
            if( brd.hit() ) continue; // any pieces hit means you can't bear in
            skip = false;
            for( j=6; j<24; j++ )
                if( brd.checker(j) != 0 )
                {
                    skip = true;
                    break;
                }
            if( skip ) continue;
            
            // so potential to bear off. If we've got exactly the right roll, done. Otherwise need to see
            // if there are any pieces in a higher spot.
            
            if( i < roll-1 )
            {
                skip = false;
                for( j=i+1; j<6; j++ )
                    if( brd.checker(j) != 0 )
                    {
                        skip = true;
                        break;
                    }
                if( skip ) continue;
            }
            
            newBrd.setChecker( i, brd.checker(i)-1 );
            newBrd.incrementBornIn();
        }
        
        // if we made it this far, there's a move to do.
        
        // if there are no more rolls, add the board to the list and continue
        
        if( rolls.size() == 1 )
        {
            moves.insert( newBrd );
            continue;
        }
        
        // otherwise start here and generate the rest of the boards
        
        vector<int> subRolls;
        subRolls.insert( subRolls.end(), rolls.begin() + 1, rolls.end() );
        
        set<board> nextMoves = subPossibleMoves( newBrd, subRolls );
        if( nextMoves.size() == 0 )
            moves.insert( newBrd );
        else
            moves.insert( nextMoves.begin(), nextMoves.end() );
    }
    
    return moves;
}

set<board> possibleMoves( const board& brd, int die1, int die2 )
{
    // set up the rolls we have - normally two, or four if it's a double
    
    vector<int> rolls;
    rolls.insert( rolls.end(), die1 );
    rolls.insert( rolls.end(), die2 );
    if( die1 == die2 )
    {
        rolls.insert( rolls.end(), die1 );
        rolls.insert( rolls.end(), die1 );
    }
    
    // generate the possible moves
    
    set<board> moves = subPossibleMoves( brd, rolls );
    
    // if the dice aren't all the same, try them in the reverse order
    
    if( die1 != die2 )
    {
        vector<int> revRolls = rolls;
        reverse( revRolls.begin(), revRolls.end() );
        
        set<board> newMoves = subPossibleMoves( brd, revRolls );
        moves.insert( newMoves.begin(), newMoves.end() );
    }
    
    // filter down the boards to ones that have the min pips - this automatically ensures
    // that all possible dice rolls that can be used are used.
    
    map<board,int> movePips;
    int minPips=30000, pips;
    for( set<board>::iterator i=moves.begin(); i!=moves.end(); i++ )
    {
        pips = (*i).pips();
        movePips[*i] = pips;
        if( pips < minPips )
            minPips = pips;
    }
    
    set<board> filtMoves;
    for( map<board,int>::iterator i=movePips.begin(); i!=movePips.end(); i++ )
    {
        if( i->second == minPips )
            filtMoves.insert( i->first );
    }
    
    return filtMoves;
}

bool isRace( const board& brd )
{
    return brd.isRace();
}
