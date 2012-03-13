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

bool doublestratjanowski::offerDouble( const board& b, bool centeredCube )
{
    // double if cubeful equity is in (0,1). Cubeful equity for cube in the middle is approximated with a piecewise 
    // linear fn: -L -> -1 for prob [0,TP], -1 -> +1 in [TP,CP], and +1 to +W in [CP,1].
    // Cubeful equity when opponent owns the cube is approximated as piecewise linear as well: -L -> -1 in [0,TP]
    // and -1 -> W in [TP,1]. Cubeful equity when player owns the cube is -L -> +1 in [0,CP] and +1 -> W in [CP,1].
    
    marketWindow window( getMarketWindow(b) );
    
    double equityNoDouble, equityDouble;
    
    // equity if we double is always the one where the opponent holds the cube
    
    if( window.probWin < window.takePoint )
        equityDouble = ( window.probWin * (-1) + ( window.takePoint - window.probWin ) * (-window.L) ) / window.takePoint;
    else
        equityDouble = ( ( 1 - window.probWin ) * (-1) + ( window.probWin - window.takePoint ) * window.W ) / ( 1 - window.takePoint );
    
    equityDouble *= 2; // what we calculated before was for cube=1, but if they take, it's worth twice as much
    
    // equity if we don't double depends on whether the cube is centered
    
    if( centeredCube )
    {
        if( window.probWin < window.takePoint )
            equityNoDouble = ( window.probWin * (-1) + ( window.takePoint - window.probWin ) * (-window.L) ) / window.takePoint;
        else if( window.probWin < window.cashPoint )
            equityNoDouble = ( ( window.probWin - window.takePoint ) * 1 + ( window.cashPoint - window.probWin ) * (-1) ) / ( window.cashPoint - window.takePoint );
        else
            equityNoDouble = ( ( 1 - window.probWin ) * 1 + ( window.probWin - window.cashPoint ) * window.W ) / ( 1 - window.cashPoint );
    }
    else
    {
        if( window.probWin < window.cashPoint )
            equityNoDouble = ( window.probWin * 1 + ( window.cashPoint - window.probWin ) * (-window.L) ) / window.cashPoint;
        else
            equityNoDouble = ( ( 1 - window.probWin ) * 1 + ( window.probWin - window.cashPoint ) * window.W ) / ( 1 - window.cashPoint );
    }
    
    return equityDouble > equityNoDouble;
}

bool doublestratjanowski::takeDouble( const board& b, bool centeredCube )
{
    marketWindow window( getMarketWindow(b) );
    
    // once the player takes the cube he owns it and can't get doubled again (unless he redoubles). We need to calculate the cubeful equity
    // post-take, and we take if that's > -1. We approximate the cubeful equity when we hold the cube as piecewise linear, -L -> +1 in [0,CP]
    // and +1 -> W in [CP,1].
    
    double equityDouble;
    if( window.probWin < window.cashPoint )
        equityDouble = ( window.probWin * 1 + ( window.cashPoint - window.probWin ) * (-window.L) ) / window.cashPoint;
    else
        equityDouble = ( ( 1 - window.probWin ) * 1 + ( window.probWin - window.cashPoint ) * window.W ) / ( 1 - window.cashPoint );
    
    equityDouble *= 2; // cube doubles the value of the game
    
    return equityDouble > -1;
}

marketWindow doublestratjanowski::getMarketWindow( const board& b )
{
    gameProbabilities probs( strat.boardProbabilities( b ) );
    
    double W = ( probs.probWin + probs.probGammonWin + probs.probBgWin ) / probs.probWin; // winning score conditional on a win (+ve)
    double L = ( 1 - probs.probWin + probs.probGammonLoss + probs.probBgLoss ) / ( 1 - probs.probWin ); // losing score conditional on a loss (+ve)
    
    double TP = ( L - 0.5 ) / ( W + L + 0.5 * cubeLifeIndex ); // take point for prob of win
    double CP = ( L + 0.5 + 0.5 * cubeLifeIndex ) / ( W + L + 0.5 * cubeLifeIndex );
    
    return marketWindow( TP, CP, probs.probWin, W, L );
}