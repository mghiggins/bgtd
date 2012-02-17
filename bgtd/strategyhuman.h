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

#ifndef bgtd_strategyhuman_h
#define bgtd_strategyhuman_h

#include "strategy.h"
#include "strategyprob.h"

class strategyhuman : public strategy
{
public:
    strategyhuman( strategyprob& guideStrat ) : guideStrat(guideStrat), equityError(0), nMoves(0) {};
    virtual ~strategyhuman() {};
    
    // the boardValue always returns 0 in this strategy because it never
    // gets used. Instead, preferredBoard lets them enter the choice of
    // board and makes sure what they chose is in the list of allowed
    // moves.
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 )  { return 0.; };
    virtual board preferredBoard( const board& oldBoard, const set<board>& possibleMoves, const hash_map<string,int>* context=0 );
    
    double SnowieER() const { return nMoves == 0 ? 0 : 1000 * equityError / 2. / nMoves; };
    
private:
    strategyprob& guideStrat;
    
    double equityError;
    int    nMoves;
};

#endif
