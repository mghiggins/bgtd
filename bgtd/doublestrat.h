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


#ifndef bgtd_doublestrat_h
#define bgtd_doublestrat_h

#include "board.h"

class strategyprob; // to avoid include cycles we include the .h file in the .cpp
class gameProbabilities; // ditto

class doublestrat
{
    // base class for doubling strategies. These are attached to regular strategies,
    // which typically are the only objects calling their methods.
    
public:
    doublestrat() {};
    virtual ~doublestrat() {};
    
    // equity returns cubeful equity - abstract, so must be implemented in derived classes
    
    virtual double equity( strategyprob& strat, const board& b, int cube, bool ownsCube, bool holdsDice ) = 0;
    
    // offerDouble takes a board, assuming the player holds the dice and has the opportunity
    // to double. Returns true if the player should double and false if not. cube is the value
    // of the cube (1 if in the center). By default this checks equity at the doubled cube level.
    
    virtual bool offerDouble( strategyprob& strat, const board& b, int cube );
    
    // takeDouble takes a board, assuming the player is being offered the cube. If it returns
    // true the player takes; false, the player passes. By default this checks equity at the doubled
    // cube level.
    
    virtual bool takeDouble( strategyprob& strat, const board& b, int cube );
    
    // gameProbabilities returns the game probabilities from the right perspective
    
    gameProbabilities boardProbabilities( strategyprob& strat, const board& b, bool holdsDice );
};

#endif
