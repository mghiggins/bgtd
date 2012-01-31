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

#include "strategy.h"

class strategyply : public strategy
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
    
    strategyply( int nPlies, int nMoveFilter, double equityCutoff, strategy& baseStrat, strategy& filterStrat );
    virtual ~strategyply() {};
    
    int nPlies, nMoveFilter;
    double equityCutoff;
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 ) const;
    double boardValueRecurse( const board& brd, int stepNPlies, const hash_map<string,int>* context ) const;
    
private:
    strategy& baseStrat;
    strategy& filterStrat;
};

#endif
