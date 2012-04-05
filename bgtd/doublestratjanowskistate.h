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

#ifndef bgtd_doublestratjanowskistate_h
#define bgtd_doublestratjanowskistate_h

#include "doublestrat.h"
#include "strategyprob.h"

class doublestratjanowskistate : public doublestrat
{
    // money game doubling strategy that uses the Janowski calculations, as per
    // http://www.bkgm.com/articles/Janowski/cubeformulae.pdf
    // Everything is based on cubeful equity numbers in the different cube states,
    // interpolating linearly between the live and dead cube limits. Different
    // cube life index depending on game state.
    
public:
    doublestratjanowskistate( double cubeLifeIndexContact, double cubeLifeIndexRace ) 
        : cubeLifeIndexContact(cubeLifeIndexContact), cubeLifeIndexRace(cubeLifeIndexRace) {};
    virtual ~doublestratjanowskistate() {};
    
    double getCubeLifeIndexContact() const { return cubeLifeIndexContact; };
    double getCubeLifeIndexRace() const { return cubeLifeIndexRace; };
    void setCubeLifeIndexContact( double newIndex ) { if( newIndex < 0 or newIndex > 1 ) throw string( "cube life index must be in [0,1]" ); cubeLifeIndexContact=newIndex; };
    void setCubeLifeIndexRace( double newIndex ) { if( newIndex < 0 or newIndex > 1 ) throw string( "cube life index must be in [0,1]" ); cubeLifeIndexRace=newIndex; };
    
    virtual double equity( strategyprob& strat, const board& b, int cube, bool ownsCube, bool holdsDice );
    
private:
    double cubeLifeIndexContact, cubeLifeIndexRace;
};


#endif
