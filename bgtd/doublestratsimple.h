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


#ifndef bgtd_doublestratsimple_h
#define bgtd_doublestratsimple_h

#include "doublestrat.h"
#include "strategyprob.h"

class doublestratdeadcube : public doublestrat
{
    // simple doubling strategy that assumes a dead cube after doubling.
    // So doubles if the player's cubeless equity > 0 and < 1.
    
public:
    doublestratdeadcube() {};
    virtual ~doublestratdeadcube() {};
    
    virtual double equity( strategyprob& strat, const board& b, int cube, bool ownsCube, bool holdsDice );
};

class doublestratnodouble : public doublestrat
{
    // strategy that never doubles, always takes
    
public:
    doublestratnodouble() {};
    virtual ~doublestratnodouble() {};
    
    // equity returns cubeless equity
    
    virtual double equity( strategyprob& strat, const board& b, int cube, bool ownsCube, bool holdsDice );
    virtual bool offerDouble( strategyprob& strat, const board& b, int cube ) { return false; };
    virtual bool takeDouble( strategyprob& strat, const board& b, int cube ) { return true; };
};

#endif
