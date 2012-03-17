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

#ifndef bgtd_doublestratmatch_h
#define bgtd_doublestratmatch_h

#include "doublestrat.h"
#include "match.h"
#include "matchequitytable.h"
#include "strategyprob.h"
#include "doublefns.h"

class doublestratmatch : public doublestrat
{
    // doubling strategy for match play
    
public:
    // constructor: a reference to the match that we're playing; a strategy
    // used to estimate probabilities; and a file name pointing to the match equity table.
    
    doublestratmatch( strategyprob& strat, const string& METFileName );
    doublestratmatch( strategyprob& strat, const matchequitytable& MET );
    virtual ~doublestratmatch() {};
    
    void setMatch( const match * currMatch ) { this->currMatch = currMatch; };
    
    virtual bool offerDouble( const board& b, int cube );
    virtual bool takeDouble( const board& b, int cube );
    
    matchequitytable getMET() const { return MET; };
    
    // equityInterpFn returns data defining a linear interpolation of match equity vs
    // probability of win, which we use for both take and offer calcs
    
    interpMEdata equityInterpFn( const gameProbabilities& probs, int perspective, int cube, int cubeOwner, int nOverride=0, int mOverride=0 );
    
private:
    const match * currMatch;
    strategyprob& strat;
    matchequitytable MET;
};

#endif
