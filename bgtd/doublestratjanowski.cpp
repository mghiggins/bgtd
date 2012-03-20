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

#include "doublestratjanowski.h"

double marketWindowJanowski::W() const
{
    return probs.probWin == 0 ? 1 : ( probs.probWin + probs.probGammonWin + probs.probBgWin ) / probs.probWin;
}

double marketWindowJanowski::L() const
{
    return 1-probs.probWin == 0 ? 1 : ( 1-probs.probWin + probs.probGammonLoss + probs.probBgLoss ) / ( 1 - probs.probWin );
}

double marketWindowJanowski::takePoint() const
{
    double l=L();
    double w=W();
    return ( l - 0.5 ) / ( w + l + cubeLifeIndex/2. );
}

double marketWindowJanowski::cashPoint() const
{
    double l=L();
    double w=W();
    return ( l + 0.5 + 0.5*cubeLifeIndex ) / ( w + l + cubeLifeIndex/2. );
}

double marketWindowJanowski::equity( double probWin, int cube, bool ownsCube ) const
{
    if( probWin < takePoint() ) return -cube;
    if( probWin > cashPoint() ) return cube;
    double l=L();
    double w=W();
    
    if( cube == 1 ) // centered
        return 4*cube/(4-cubeLifeIndex) * ( probWin * ( w + l + 0.5*cubeLifeIndex ) - l - 0.25*cubeLifeIndex );
    if( ownsCube )
        return cube * ( probWin * ( w + l + 0.5*cubeLifeIndex ) - l );
    else
        return cube * ( probWin * ( w + l + 0.5*cubeLifeIndex ) - l - 0.5*cubeLifeIndex );
}

bool doublestratjanowski::offerDouble( const board& b, int cube )
{
    // double if cubeful equity is in (0,1). Cubeful equity for cube in the middle is approximated with a piecewise 
    // linear fn: -L -> -1 for prob [0,TP], -1 -> +1 in [TP,CP], and +1 to +W in [CP,1].
    // Cubeful equity when opponent owns the cube is approximated as piecewise linear as well: -L -> -1 in [0,TP]
    // and -1 -> W in [TP,1]. Cubeful equity when player owns the cube is -L -> +1 in [0,CP] and +1 -> W in [CP,1].
    
    marketWindowJanowski window( strat.boardProbabilities(b), cubeLifeIndex );
    
    // equity if we double is always the one where the opponent holds the cube
    
    double equityDouble = window.equity( window.probs.probWin, 2*cube, false );
    
    // equity if we don't double is the one where we own the cube at its current value
    
    double equityNoDouble = window.equity( window.probs.probWin, cube, true );
    
    // we double if it's better to do so from an equity perspective
    
    return equityDouble > equityNoDouble;
}

bool doublestratjanowski::takeDouble( const board& b, int cube )
{
    marketWindowJanowski window( strat.boardProbabilities(b), cubeLifeIndex );
    
    // once the player takes the cube he owns it and can't get doubled again (unless he redoubles). We need to calculate the cubeful equity
    // post-take, and we take if that's > -1.
    
    double equityDouble = window.equity( window.probs.probWin, 2, false );
    return equityDouble > -1;
}

