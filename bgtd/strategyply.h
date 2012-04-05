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

#ifndef bgtd_strategyply_h
#define bgtd_strategyply_h

#include <boost/thread.hpp>
#include <deque>
#include "strategyprob.h"

class strategyply : public strategyprob
{
    // strategy that holds another strategy internally, and uses it to look ahead
    // a certain number of "plies". One ply is one turn by one player.
    
public:
    // nPlies is the number of plies to search; nMoveFilter is the # of moves to include
    // when doing deeper searches; equityCutoff is an equity cutoff based on the filter
    // strategy equity that we also use to filter out unlikely moves; baseStrat is the underlying strategy we 
    // use to dig into the tree; filterStrat is the strategy we use to come up with 
    // nMoveFilter-element list that we calculate a more accurate board value for 
    // (filterStrat is usually a fairly coarse strategy that gets equity correct only
    // approximately).
    
    strategyply( int nPlies, int nMoveFilter, double equityCutoff, strategyprob& baseStrat, strategyprob& filterStrat, doublestrat * ds=0 );
    virtual ~strategyply() {};
    
    int nPlies, nMoveFilter;
    double equityCutoff;
    
    virtual board preferredBoard( const board& oldBoard, const set<board>& possibleMoves, const hash_map<string,int>* context=0 );
    virtual gameProbabilities boardProbabilities( const board& brd, const hash_map<string,int>* context=0 ); 
    gameProbabilities boardProbsRecurse( const board& brd, int stepNPlies, const hash_map<string,int>* context, hash_map<string,gameProbabilities>& filterMap, hash_map<string,gameProbabilities>& baseMap );
    
    unsigned long cacheSize() const { return probCache.size(); };
    
private:
    strategyprob& baseStrat;
    strategyprob& filterStrat;
    
    long filterCalcCount;
    long filterCacheCount;
    long baseCalcCount;
    long baseCacheCount;
    
    // prob cache stuff. We wrap reads and writes in thread-safe code so that we can run
    // the same strategy in different threads. Realistically though the odds of a collision
    // on the prob cache are tiny, since multiple-ply calcs are slow and don't all finish
    // at the same time (normally). We also track the string names in the cache in a FIFO
    // deque, so that we can avoid having the cache get too large.
    
    int maxCacheSize;
    hash_map<string,gameProbabilities> probCache;
    deque<string> keys;
    boost::shared_mutex probCacheMutex;
    
    void addToCache( const string& key, const gameProbabilities& probs );
};

#endif
