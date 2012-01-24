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

strategyply::strategyply( int nPlies, int nMoveFilter, strategy& baseStrat, strategy& filterStrat ) : nPlies(nPlies), nMoveFilter(nMoveFilter), baseStrat( baseStrat ), filterStrat( filterStrat )
{
}

double strategyply::boardValue( const board& brd, const hash_map<string,int>* context ) const
{
    return boardValueRecurse( brd, nPlies, context );
}

double strategyply::boardValueRecurse( const board& brd, int stepNPlies, const hash_map<string,int>* context ) const
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
        return baseStrat.boardValue( brd, context );
    
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
            
            // first get a coarse evaluation of the value of possible moves using the
            // filterStrat strategy. Later on we'll calculate the value more accurately
            // for the top nMoveFilter elements.
            
            set<board> moves( possibleMoves( stepBoard, die1, die2 ) );
            vector<boardAndVal> moveVals;
            for( set<board>::iterator it=moves.begin(); it!=moves.end(); it++ )
            {
                val = filterStrat.boardValue( (*it), context );
                boardAndVal elem;
                elem.brd = (*it);
                elem.zeroPlyVal = val;
                moveVals.push_back( elem );
            }
            
            maxVal = -1000; // reset the max value
        
            // sort the elements by board value, biggest board value first. Remember,
            // these board values are a coarse approx; we then calculate a more accurate board
            // value on the top nMoveFilter elements to make our choice of move.
            
            sort( moveVals.begin(), moveVals.end(), boardAndValCompare );
            
            // re-run the calculations using the deeper search for the top nMoveFilter moves
            
            nElems=moveVals.size();
            if( nElems > nMoveFilter ) nElems = nMoveFilter;
            for( int i=0; i<nElems; i++ )
            {
                // if we're at 1-ply and the filter strategy is the same as the regular strategy, just use the precalculated values.
                // Otherwise calculate them using the base strategy.
                
                if( stepNPlies == 1 and &filterStrat == &baseStrat )
                    val = moveVals.at(i).zeroPlyVal;
                else
                    val = boardValueRecurse( moveVals.at(i).brd, stepNPlies-1, context );
                if( val > maxVal )
                    maxVal = val;
            }
            
            // if it didn't find anything it's because the roll resulted in
            // no possible moves - eg a 2-2 where there's only one piece left
            // and the opponent has a point 2 steps away. In this case we're
            // left with the original board.
            
            if( maxVal == -1000 )
                maxVal = boardValueRecurse( stepBoard, stepNPlies - 1, context );
            
            // add the optimal board value to the weighted average
            
            avgVal += maxVal * weight;
        }
    }
    
    avgVal *= -1; // to flip the perspective back
    
    return avgVal;
}