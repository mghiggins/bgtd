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

#include "doublestratjanowskistate.h"

double doublestratjanowskistate::equity( strategyprob& strat, const board& b, int cube, bool ownsCube, bool holdsDice )
{
    gameProbabilities probs( boardProbabilities( strat, b, holdsDice ) );
    
    double W=probs.probWin==0 ? 1 : (probs.probWin + probs.probGammonWin + probs.probBgWin)/probs.probWin;
    double L=1-probs.probWin==0 ? 1 : (1-probs.probWin + probs.probGammonLoss + probs.probBgLoss)/(1-probs.probWin);
    
    // equity for the dead cube is always the same (normalized by the cube value)
    
    double P = probs.probWin;
    double equityDead = P*(W+L)-L;
    
    // equity for the live cube depends on whether the cube is centered, we own, or the opponent owns,
    // and whether equity is before the live cube take point or above its cash point. Piecewise linear
    // in all sections. Equity is -L at P=0, +W at P=1, -1 at the take point, and +1 at the cash point.
    
    double TPlive = ( L - 0.5 ) / ( W + L + 0.5 );
    double CPlive = ( L + 1   ) / ( W + L + 0.5 );
    
    double equityLive;
    if( cube == 1 )
    {
        if( P < TPlive )
            equityLive = ( P*(-1) + (TPlive-P)*(-L) ) / TPlive;
        else if( P > CPlive )
            equityLive = ( (P-CPlive)*W + (1-P)*1 ) / ( 1 - CPlive );
        else
            equityLive = ( (P-TPlive)*(1) + (CPlive-P)*(-1) ) / ( CPlive-TPlive );
    }
    else if( ownsCube )
    {
        if( P > CPlive )
            equityLive = ( (P-CPlive)*W + (1-P)*1 ) / ( 1 - CPlive );
        else
            equityLive = ( P*1 + (CPlive-P)*(-L) ) / CPlive;
    }
    else
    {
        if( P < TPlive )
            equityLive = ( P*(-1) + (TPlive-P)*(-L) ) / TPlive;
        else
            equityLive = ( (P-TPlive)*W + (1-P)*(-1) ) / ( 1 - TPlive );
    }
    
    // the model equity is a regular linear interpolation between the live and dead cube equities.
    // Use the appropriate cube life index for the game state.
    
    double cubeLifeIndex = b.isRace() ? cubeLifeIndexRace : cubeLifeIndexContact;
    return cube * ( cubeLifeIndex * equityLive + ( 1 - cubeLifeIndex ) * equityDead );
}
