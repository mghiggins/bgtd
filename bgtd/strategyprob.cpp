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
    output << "Probabilities:\n";
    output << "Any win         = " << probs.probWin << endl;
    output << "Any gammon win  = " << probs.probGammonWin << endl;
    output << "Backgammon win  = " << probs.probBgWin << endl;
    output << "Any loss        = " << 1 - probs.probWin << endl;
    output << "Any gammon loss = " << probs.probGammonLoss << endl;
    output << "Backgammon loss = " << probs.probBgLoss << endl;
    return output;
}

double strategyprob::boardValueFromProbs( const gameProbabilities& probs ) const 
{ 
    return ( probs.probWin - probs.probGammonWin ) + 2 * ( probs.probGammonWin - probs.probBgWin ) + 3 * probs.probBgWin
         - ( 1 - probs.probWin - probs.probGammonLoss ) - 2 * ( probs.probGammonLoss - probs.probBgLoss ) - 3 * probs.probBgLoss;
};

double strategyprob::boardValue( const board& brd, const hash_map<string,int>* context ) const 
{ 
    return boardValueFromProbs( boardProbabilities( brd, context ) ); 
}
