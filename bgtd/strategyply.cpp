//
//  strategyply.cpp
//  bgtd
//
//  Created by Mark Higgins on 12/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <algorithm>
#include <iostream>
#include "gamefns.h"
#include "strategyply.h"

// define some helper classes & fns for sorting potential moves by board value

struct boardAndVal
{
    board brd;
    double zeroPlyVal;
};

bool boardAndValCompare( const boardAndVal& v1, const boardAndVal& v2 );
bool boardAndValCompare( const boardAndVal& v1, const boardAndVal& v2 ) { return v1.zeroPlyVal > v2.zeroPlyVal; }

strategyply::strategyply( int nPlies, int nMoveFilter, strategy& baseStrat ) : nPlies(nPlies), nMoveFilter(nMoveFilter), baseStrat( baseStrat )
{
}

double strategyply::boardValue( const board& brd ) const
{
    return boardValueRecurse( brd, nPlies );
}

double strategyply::boardValueRecurse( const board& brd, int stepNPlies ) const
{
    // if the game is over, return the appropriate points
    
    if( brd.bornIn() == 15 )
    {
        if( brd.otherBornIn() == 0 )
        {
            if( brd.otherNoBackgammon() )
                return 2;
            else
                return 3;
        }
        else
            return 1;
    }
    if( brd.otherBornIn() == 15 )
    {
        if( brd.bornIn() == 0 )
        {
            if( brd.noBackgammon() )
                return -2;
            else
                return -3;
        }
        else
            return -1;
    }
    
    // if there are zero plies, call the underlying strategy
    
    if( stepNPlies == 0 )
        return baseStrat.boardValue( brd );
    
    // otherwise recurse down through the next level of moves; flip to
    // the opponent's perspective and run through their moves.
    
    board stepBoard( brd );
    stepBoard.setPerspective( 1 - brd.perspective() );
    
    double weight, maxVal, val;
    int die1, die2;
    long nElems;
    double avgVal=0;
    
    for( die1=1; die1<7; die1++ )
    {
        for( die2=1; die2<=die1; die2++ )
        {
            if( die1 == die2 )
                weight = 1/36.;
            else
                weight = 1/18.;
            
            // first get a zero-ply evaluation of the possible moves; we'll use this
            // to trim down the moves we calculate the more accurate board value for.
            // Or, if we're doing 1-ply, this is the only calculation required.
            
            set<board> moves( possibleMoves( stepBoard, die1, die2 ) );
            maxVal=-1000;
            vector<boardAndVal> moveVals;
            for( set<board>::iterator it=moves.begin(); it!=moves.end(); it++ )
            {
                val = baseStrat.boardValue( (*it) );
                if( val > maxVal )
                    maxVal = val;
                if( stepNPlies > 1 )
                {
                    boardAndVal elem;
                    elem.brd = (*it);
                    elem.zeroPlyVal = val;
                    moveVals.push_back( elem );
                }
            }
            
            if( stepNPlies > 1 )
            {
                maxVal = -1000; // reset the max value
            
                // sort those elements by board value, biggest board value first. Remember,
                // these board values are zero ply; we then calculate a more accurate board
                // value on the top nMoveFilter elements to make our choice of move.
                
                sort( moveVals.begin(), moveVals.end(), boardAndValCompare );
                
                // re-run the calculations using the deeper search for the top nMoveFilter moves
                
                nElems=moveVals.size();
                if( nElems > nMoveFilter ) nElems = nMoveFilter;
                for( int i=0; i<nElems; i++ )
                {
                    val = boardValueRecurse( moveVals.at(i).brd, stepNPlies-1 );
                    if( val > maxVal )
                        maxVal = val;
                }
            }
            
            // if it didn't find anything it's because the roll resulted in
            // no possible moves - eg a 2-2 where there's only one piece left
            // and the opponent has a point 2 steps away. In this case we're
            // left with the original board.
            
            if( maxVal == -1000 )
                maxVal = boardValueRecurse( stepBoard, stepNPlies - 1 );
            
            // add the optimal board value to the weighted average
            
            avgVal += maxVal * weight;
        }
    }
    
    avgVal *= -1; // to flip the perspective back
    
    return avgVal;
}