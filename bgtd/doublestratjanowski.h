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

class marketWindow
{
public:
    marketWindow( double takePoint, double cashPoint, double probWin, double W, double L ) : takePoint(takePoint), cashPoint(cashPoint), probWin(probWin), W(W), L(L) {};
    
    double takePoint; // below this prob of any win, you pass when offered the cube
    double cashPoint; // above this prob of any win, your opponent will pass when offered the cube
    double probWin;   // probability of any win
    double W;         // expected winning points conditional on a win (+ve)
    double L;         // expected losing points conditional on a loss (-ve)
};

class doublestratjanowski : public doublestrat
{
    // money game doubling strategy that uses the Janowski calculations, as per
    // http://www.bkgm.com/articles/Janowski/cubeformulae.pdf
    
public:
    doublestratjanowski( strategyprob& strat, double cubeLifeIndex ) : strat(strat), cubeLifeIndex(cubeLifeIndex) {};
    virtual ~doublestratjanowski() {};
    
    virtual bool offerDouble( const board& b, bool centeredCube );
    virtual bool takeDouble( const board& b, bool centeredCube );
    
    marketWindow getMarketWindow( const board& b );
    
    double getCubeLifeIndex() const { return cubeLifeIndex; };
    void setCubeLifeIndex( double newIndex ) { if( newIndex < 0 or newIndex > 1 ) throw string( "cube life index must be in [0,1]" ); cubeLifeIndex=newIndex; };
    
private:
    strategyprob& strat;
    double cubeLifeIndex;
};

#endif
