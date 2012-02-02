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
    maxCacheSize = 30000;
}

gameProbabilities getProbs( const board& brd, strategyprob& strat, const hash_map<string,int>* context, hash_map<string,gameProbabilities>& cache, long& calcCount, long& cacheCount );

gameProbabilities getProbs( const board& brd, strategyprob& strat, const hash_map<string,int>* context, hash_map<string,gameProbabilities>& cache, long& calcCount, long& cacheCount )
{
    calcCount++;
    
    // see if the probabilities have been cached already for this one
    
    string brdRepr( brd.repr() );
    hash_map<string,gameProbabilities>::iterator it=cache.find( brdRepr );
    if( it!=cache.end() ) 
    {
        cacheCount++;
        return it->second;
    }
    
    // calculate the probs, cache them, and return them
    
    gameProbabilities probs = strat.boardProbabilities( brd, context );
    cache[ brdRepr ] = probs;
    return probs;
}

void strategyply::addToCache( const string& key, const gameProbabilities& probs )
{
    // adds to the cache of strategyply-level board->prob map. Makes sure the cache
    // doesn't get too big.
    
    // get a unique lock on the prob cache so we're safe writing to it - drops the
    // lock when we leave this method and the object goes out of scope.
    
    boost::unique_lock<boost::shared_mutex> lock(probCacheMutex);
    
    // update the cache
    
    probCache[ key ] = probs;
    keys.push_back( key );
    
    // if the cache is too big, drop the oldest element
    
    if( probCache.size() > maxCacheSize )
    {
        probCache.erase( keys.front() );
        keys.pop_front();
    }
}

board strategyply::preferredBoard( const board& oldBoard, const set<board>& possibleMoves, const hash_map<string,int>* context )
{
    if( possibleMoves.size() == 0 ) return oldBoard;
    if( possibleMoves.size() == 1 ) return *(possibleMoves.begin()); // only one move - choose it - no calcs req'd
    
    // create a context that merges the context based on the old board and whatever is passed in. The context
    // pass in externally overrides any values with the same key from the board context.
    
    hash_map<string,int> mergedContext( boardContext( oldBoard ) );
    if( context != 0 )
        for( hash_map<string,int>::const_iterator it=context->begin(); it!=context->end(); it++ )
            mergedContext[ it->first ] = it->second;
    
    // set up caches for board key->game probabilities map that will be shared through the levels of the calculation.
    // Also initialize the counts (for debugging purposes).
    
    hash_map<string,gameProbabilities> filterMap;
    hash_map<string,gameProbabilities> baseMap;
    
    filterCalcCount  = 0;
    filterCacheCount = 0;
    baseCalcCount    = 0;
    baseCacheCount   = 0;
    
    // return the board with the highest board value
    
    board maxBoard;
    double maxVal=-1e99, val;
    
    for( set<board>::iterator i=possibleMoves.begin(); i!=possibleMoves.end(); i++ )
    {
        string brdRepr( i->repr() );
        gameProbabilities probs;
        bool calcProbs=false;
        {
            // get a shared lock on the prob cache so we're safe reading from it
            
            boost::shared_lock<boost::shared_mutex> lock(probCacheMutex);
            hash_map<string,gameProbabilities>::iterator it=probCache.find(brdRepr);
            if( it!=probCache.end() )
                probs = it->second;
            else
                calcProbs = true;
        }
        if( calcProbs )
        {
            probs = boardProbsRecurse( (*i), nPlies, &mergedContext, filterMap, baseMap );
            addToCache( brdRepr, probs );
        }
        val = boardValueFromProbs( probs );
        if( val > maxVal )
        {
            maxVal = val;
            maxBoard = (*i);
        }
    }
    
    // Debugging info: print out the number of calcs done and the number of times we
    // read from the cache - used to figure out whether caching is working correctly etc.
    //cout << "Filter: cache " << filterCacheCount << " of " << filterCalcCount << endl;
    //cout << "Base:   cache " << baseCacheCount   << " of " << baseCalcCount << endl;
    
    return maxBoard;
}

gameProbabilities strategyply::boardProbabilities( const board& brd, const hash_map<string,int>* context )
{
    string brdRepr( brd.repr() );
    {
        // get a shared lock on the prob cache so that we're safe reading from it
        
        boost::shared_lock<boost::shared_mutex> lock(probCacheMutex);
        hash_map<string,gameProbabilities>::iterator it=probCache.find(brdRepr);
        if( it!=probCache.end() )
            return it->second;
    }
    
    // set up caches for board key->game probabilities map that will be shared through the levels of the calculation
    
    hash_map<string,gameProbabilities> filterMap;
    hash_map<string,gameProbabilities> baseMap;
    
    filterCalcCount  = 0;
    filterCacheCount = 0;
    baseCalcCount    = 0;
    baseCacheCount   = 0;
    
    gameProbabilities probs = boardProbsRecurse( brd, nPlies, context, filterMap, baseMap );
    addToCache( brdRepr, probs );
    return probs;
}

gameProbabilities strategyply::boardProbsRecurse( const board& brd, int stepNPlies, const hash_map<string,int>* context, hash_map<string,gameProbabilities>& filterMap, hash_map<string,gameProbabilities>& baseMap )
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
        //return baseStrat.boardProbabilities( brd, context );
        return getProbs( brd, baseStrat, context, baseMap, baseCalcCount, baseCacheCount );
    
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
                //gameProbabilities probs( filterStrat.boardProbabilities( (*it), context ) );
                gameProbabilities probs( getProbs( (*it), filterStrat, context, filterMap, filterCalcCount, filterCacheCount ) );
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
                        if( equityDiff > equityCutoff ) break;
                    }
                    
                    moveProbs = boardProbsRecurse( moveVals.at(i).brd, stepNPlies-1, context, filterMap, baseMap );
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
                maxProbs = boardProbsRecurse( stepBoard, stepNPlies - 1, context, filterMap, baseMap );
            
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