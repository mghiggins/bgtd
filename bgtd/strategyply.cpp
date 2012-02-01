/*****************
 Copyright 2011, 2012 Mark Higgins
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 *****************/

#include <algorithm>
#include <iostream>
#include "gamefns.h"
#include "strategyply.h"

// define some helper classes & fns for sorting potential moves by board value

struct boardAndVal
{
    board brd;
    double zeroPlyVal;
    gameProbabilities zeroPlyProbs;
};

bool boardAndValCompare( const boardAndVal& v1, const boardAndVal& v2 );
bool boardAndValCompare( const boardAndVal& v1, const boardAndVal& v2 ) { return v1.zeroPlyVal > v2.zeroPlyVal; }

strategyply::strategyply( int nPlies, int nMoveFilter, double equityCutoff, strategyprob& baseStrat, strategyprob& filterStrat )
 : nPlies(nPlies), nMoveFilter(nMoveFilter), equityCutoff( equityCutoff), baseStrat( baseStrat ), filterStrat( filterStrat )
{
}

gameProbabilities strategyply::boardProbabilities( const board& brd, const hash_map<string,int>* context ) const
{
    return boardProbsRecurse( brd, nPlies, context );
}

gameProbabilities strategyply::boardProbsRecurse( const board& brd, int stepNPlies, const hash_map<string,int>* context ) const
{
    // if the game is over, return the appropriate points
    
    if( brd.bornIn() == 15 )
    {
        gameProbabilities probs( 1, 0, 0, 0, 0 );
        if( brd.otherBornIn() == 0 )
        {
            probs.probGammonWin = 1;
            if( !brd.otherNoBackgammon() )
                probs.probBgWin = 1;
        }
        return probs;
    }
    if( brd.otherBornIn() == 15 )
    {
        gameProbabilities probs( 0, 0, 0, 0, 0 );
        if( brd.bornIn() == 0 )
        {
            probs.probGammonLoss = 1;
            if( !brd.noBackgammon() )
                probs.probBgLoss = 1;
        }
        return probs;
    }
    
    // if there are zero plies, call the underlying strategy
    
    if( stepNPlies == 0 )
        return baseStrat.boardProbabilities( brd, context );
    
    // otherwise recurse down through the next level of moves; flip to
    // the opponent's perspective and run through their moves.
    
    board stepBoard( brd );
    stepBoard.setPerspective( 1 - brd.perspective() );
    
    double weight, maxVal, val;
    gameProbabilities maxProbs(0,0,0,0,0);
    int die1, die2;
    long nElems;
    gameProbabilities avgVal(0,0,0,0,0);
    
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
                gameProbabilities probs( filterStrat.boardProbabilities( (*it), context ) );
                val = filterStrat.boardValueFromProbs( probs );
                boardAndVal elem;
                elem.brd = (*it);
                elem.zeroPlyVal = val;
                elem.zeroPlyProbs = probs;
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
            double equityDiff;
            gameProbabilities moveProbs;
            for( int i=0; i<nElems; i++ )
            {
                // if we're at 1-ply and the filter strategy is the same as the regular strategy, just use the precalculated values.
                // Otherwise calculate them using the base strategy.
                
                if( stepNPlies == 1 and &filterStrat == &baseStrat )
                {
                    val = moveVals.at(i).zeroPlyVal;
                    moveProbs = moveVals.at(i).zeroPlyProbs;
                }
                else
                {
                    // we further filter by ignoring moves whose (filter strategy) equity is more than
                    // equityCutoff different from the best one. If equityCutoff is zero we assume
                    // that means there is no cutoff.
                    
                    if( equityCutoff > 0 and i > 0 )
                    {
                        equityDiff = moveVals.at(0).zeroPlyVal - moveVals.at(i).zeroPlyVal;
                        if( equityDiff > equityCutoff ) continue;
                    }
                    
                    moveProbs = boardProbsRecurse( moveVals.at(i).brd, stepNPlies-1, context );
                    val = boardValueFromProbs( moveProbs );
                }
                if( val > maxVal )
                {
                    maxVal = val;
                    maxProbs = moveProbs;
                }
            }
            
            // if it didn't find anything it's because the roll resulted in
            // no possible moves - eg a 2-2 where there's only one piece left
            // and the opponent has a point 2 steps away. In this case we're
            // left with the original board.
            
            if( maxVal == -1000 )
                maxProbs = boardProbsRecurse( stepBoard, stepNPlies - 1, context );
            
            // add the optimal board value to the weighted average - flip everything back to
            // the correct perspective while we're at it.
            
            avgVal.probWin        += ( 1 - maxProbs.probWin ) * weight;
            avgVal.probGammonWin  += maxProbs.probGammonLoss * weight;
            avgVal.probGammonLoss += maxProbs.probGammonWin * weight;
            avgVal.probBgWin      += maxProbs.probBgLoss * weight;
            avgVal.probBgLoss     += maxProbs.probBgWin * weight;
        }
    }
    
    return avgVal;
}