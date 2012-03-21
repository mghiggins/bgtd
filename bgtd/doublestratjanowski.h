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


#ifndef bgtd_doublestratjanowski_h
#define bgtd_doublestratjanowski_h

#include "doublestrat.h"
#include "strategyprob.h"

class marketWindowJanowski
{
    // class used to calculate cube decision points and cubeful equities in Janowski's model
    
public:
    marketWindowJanowski( gameProbabilities probs, double cubeLifeIndex ) : probs(probs), cubeLifeIndex(cubeLifeIndex) {};
    
    gameProbabilities probs; // cubeless game probabilities when the market window was created
    double cubeLifeIndex;   // Janowski's cube life index
    
    double W() const;
    double L() const;
    double takePoint() const;
    double cashPoint() const;
    double initialDoublePoint() const;
    double redoublePoint() const;
    double tooGoodPoint() const;
    double equity( double probWin, int cube, bool ownsCube ) const;
};

class doublestratjanowski : public doublestrat
{
    // money game doubling strategy that uses the Janowski calculations, as per
    // http://www.bkgm.com/articles/Janowski/cubeformulae.pdf
    
public:
    doublestratjanowski( strategyprob& strat, double cubeLifeIndex ) : strat(strat), cubeLifeIndex(cubeLifeIndex) {};
    virtual ~doublestratjanowski() {};
    
    virtual bool offerDouble( const board& b, int cube );
    virtual bool takeDouble( const board& b, int cube );
    
    double getCubeLifeIndex() const { return cubeLifeIndex; };
    void setCubeLifeIndex( double newIndex ) { if( newIndex < 0 or newIndex > 1 ) throw string( "cube life index must be in [0,1]" ); cubeLifeIndex=newIndex; };
    
    gameProbabilities boardProbabilities( const board& b );
    
private:
    strategyprob& strat;
    double cubeLifeIndex;
};

#endif
