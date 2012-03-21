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

double marketWindowJanowski::initialDoublePoint() const
{
    double l=L();
    double w=W();
    return (l+(3-cubeLifeIndex)/(2-cubeLifeIndex)*cubeLifeIndex/2.)/(w+l+cubeLifeIndex/2.);
}

double marketWindowJanowski::redoublePoint() const
{
    double l=L();
    double w=W();
    return (l+cubeLifeIndex)/(w+l+0.5*cubeLifeIndex);
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
    // double if cubeful equity is in (0,1).
    
    marketWindowJanowski window( boardProbabilities(b), cubeLifeIndex );
    
    // equity if we double is always the one where the opponent holds the cube
    
    double equityDouble = window.equity( window.probs.probWin, 2*cube, false );
    
    // ... but he might pass, so min that with 1*cube
    
    if( equityDouble > cube ) equityDouble = cube;
    
    // equity if we don't double is the one where we own the cube at its current value
    
    double equityNoDouble = window.equity( window.probs.probWin, cube, true );
    
    // we double if it's better to do so from an equity perspective. Add a wee threshold
    // so that we don't double when they're equal (since that probably means too good).
    
    return equityDouble > equityNoDouble + 1e-6;
}

bool doublestratjanowski::takeDouble( const board& b, int cube )
{
    marketWindowJanowski window( boardProbabilities(b), cubeLifeIndex );
    
    // once the player takes the cube he owns it and can't get doubled again (unless he redoubles). We need to calculate the cubeful equity
    // post-take, and we take if that's > -1.
    
    double equityDouble = window.equity( window.probs.probWin, 2, true );
    return equityDouble > -1;
}

gameProbabilities doublestratjanowski::boardProbabilities( const board& b )
{
    // boardProbabilities from the strategy corresponds to probabilities *after* the player
    // has passed the dice; we need it before.
    
    board fb(b);
    fb.setPerspective(1-b.perspective());
    gameProbabilities probs( strat.boardProbabilities(fb) );
    return probs.flippedProbs();
}
