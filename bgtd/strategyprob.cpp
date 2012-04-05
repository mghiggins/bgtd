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


#include "strategyprob.h"

ostream& operator<<( ostream& output, const gameProbabilities& probs )
{
    double equity = 1 * ( probs.probWin - probs.probGammonWin ) + 2 * ( probs.probGammonWin - probs.probBgWin ) + 3 * probs.probBgWin
                  - 1 * ( 1 - probs.probWin - probs.probGammonLoss ) - 2 * ( probs.probGammonLoss - probs.probBgLoss ) - 3 * probs.probBgLoss;
    output << "Equity          = " << equity << endl;
    output << "Any win         = " << probs.probWin << endl;
    output << "Any gammon win  = " << probs.probGammonWin << endl;
    output << "Backgammon win  = " << probs.probBgWin << endl;
    output << "Any loss        = " << 1 - probs.probWin << endl;
    output << "Any gammon loss = " << probs.probGammonLoss << endl;
    output << "Backgammon loss = " << probs.probBgLoss << endl;
    return output;
}

gameProbabilities operator+( const gameProbabilities& probs, double dv )
{
    gameProbabilities probsNew( probs.probWin+dv, probs.probGammonWin+dv, probs.probGammonLoss+dv, probs.probBgWin+dv, probs.probBgLoss+dv );
    return probsNew;
}

gameProbabilities operator-( const gameProbabilities& probs, double dv )
{
    return operator+( probs, -dv );
}

gameProbabilities operator*( const gameProbabilities& probs, double f )
{
    gameProbabilities probsNew( probs.probWin*f, probs.probGammonWin*f, probs.probGammonLoss*f, probs.probBgWin*f, probs.probBgLoss*f );
    return probsNew;
}

gameProbabilities operator/( const gameProbabilities& probs, double f )
{
    if( f == 0 ) throw string( "Cannot divide by zero" );
    gameProbabilities probsNew( probs.probWin/f, probs.probGammonWin/f, probs.probGammonLoss/f, probs.probBgWin/f, probs.probBgLoss/f );
    return probsNew;
}

gameProbabilities operator+( const gameProbabilities& probs1, const gameProbabilities& probs2 )
{
    gameProbabilities probsNew( probs1.probWin+probs2.probWin, 
                                probs1.probGammonWin+probs2.probGammonWin, probs1.probGammonLoss+probs2.probGammonLoss, 
                                probs1.probBgWin+probs2.probBgWin, probs1.probBgLoss+probs2.probBgLoss );
    return probsNew;
}

gameProbabilities operator-( const gameProbabilities& probs1, const gameProbabilities& probs2 )
{
    gameProbabilities probsNew( probs1.probWin-probs2.probWin, 
                               probs1.probGammonWin-probs2.probGammonWin, probs1.probGammonLoss-probs2.probGammonLoss, 
                               probs1.probBgWin-probs2.probBgWin, probs1.probBgLoss-probs2.probBgLoss );
    return probsNew;
}

gameProbabilities operator*( const gameProbabilities& probs1, const gameProbabilities& probs2 )
{
    gameProbabilities probsNew( probs1.probWin*probs2.probWin, 
                               probs1.probGammonWin*probs2.probGammonWin, probs1.probGammonLoss*probs2.probGammonLoss, 
                               probs1.probBgWin*probs2.probBgWin, probs1.probBgLoss*probs2.probBgLoss );
    return probsNew;
}

double strategyprob::boardValueFromProbs( const gameProbabilities& probs ) const 
{ 
    return ( probs.probWin - probs.probGammonWin ) + 2 * ( probs.probGammonWin - probs.probBgWin ) + 3 * probs.probBgWin
         - ( 1 - probs.probWin - probs.probGammonLoss ) - 2 * ( probs.probGammonLoss - probs.probBgLoss ) - 3 * probs.probBgLoss;
};

double strategyprob::boardValue( const board& brd, const hash_map<string,int>* context ) 
{ 
    return boardValueFromProbs( boardProbabilities( brd, context ) ); 
}

double strategyprob::equityCubeless( const board& brd, bool holdsDice )
{
    gameProbabilities probs;
    if( holdsDice )
    {
        board fb(brd);
        fb.setPerspective(1-brd.perspective());
        probs = boardProbabilities(fb).flippedProbs();
    }
    else
        probs = boardProbabilities(brd);
    
    return boardValueFromProbs(probs);
}

double strategyprob::equityCubeful( const board& brd, int cube, bool ownsCube, bool holdsDice )
{
    if( ds )
        return ds->equity( *this, brd, cube, ownsCube, holdsDice );
    else
        return equityCubeless( brd, holdsDice );
}