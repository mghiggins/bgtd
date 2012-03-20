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


#include "doublestratsimple.h"

bool doublestratdeadcube::offerDouble( const board& b, int cube )
{
    gameProbabilities probs( boardProbabilities( b ) );
    
    double equity = probs.equity();
    return equity > 0 and equity < 1;
}

bool doublestratdeadcube::takeDouble( const board& b, int cube )
{
    gameProbabilities probs( boardProbabilities( b ) );
    double equity = probs.equity();
    return 2 * equity > -1;
}

gameProbabilities doublestratdeadcube::boardProbabilities( const board& b )
{
    board fb(b);
    fb.setPerspective(1-b.perspective());
    gameProbabilities probs( strat.boardProbabilities(fb) );
    return probs.flippedProbs();
}