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

#ifndef bgtd_strategyprob_h
#define bgtd_strategyprob_h

#include "strategy.h"

class gameProbabilities
{
public:
    gameProbabilities() : probWin(0), probGammonWin(0), probGammonLoss(0), probBgWin(0), probBgLoss(0) {};
    gameProbabilities( double probWin, double probGammonWin, double probGammonLoss, double probBgWin, double probBgLoss ) 
        : probWin(probWin), probGammonWin(probGammonWin), probGammonLoss(probGammonLoss), probBgWin(probBgWin), probBgLoss( probBgLoss ) {};
    double probWin;        // probability of any win. Prob of loss = 1 - probWin.
    double probGammonWin;  // probability of any gammon win
    double probGammonLoss; // probability of any gammon loss
    double probBgWin;      // probability of a backgammon win
    double probBgLoss;     // probability of a backgammon loss
    
    gameProbabilities flippedProbs() const { return gameProbabilities( 1 - probWin, probGammonLoss, probGammonWin, probBgLoss, probBgWin ); };
    
    double equity() const { return 1 * ( probWin - probGammonWin ) + 2 * ( probGammonWin - probBgWin ) + 3 * probBgWin - 1 * ( 1 - probWin - probGammonLoss ) - 2 * ( probGammonLoss - probBgLoss ) - 3 * probBgLoss; };
};

gameProbabilities operator+( const gameProbabilities& probs, double dv );
gameProbabilities operator-( const gameProbabilities& probs, double dv );
gameProbabilities operator*( const gameProbabilities& probs, double f );
gameProbabilities operator/( const gameProbabilities& probs, double f );
//gameProbabilities operator/( const gameProbabilities& probs, int f );
gameProbabilities operator+( const gameProbabilities& probs1, const gameProbabilities& probs2 );
gameProbabilities operator-( const gameProbabilities& probs1, const gameProbabilities& probs2 );
gameProbabilities operator*( const gameProbabilities& probs1, const gameProbabilities& probs2 );

ostream& operator<<( ostream& output, const gameProbabilities& probs );

class strategyprob : public strategy
{
    // Abstract base class for any strategy that wants to define
    // probabilities of different types of win and loss. From this
    // point we also assume that boardValue represents an equity.
    
public:
    strategyprob() {};
    virtual ~strategyprob() {};
    
    // any derived class must implement boardProbabilities
    
    virtual gameProbabilities boardProbabilities( const board& brd, const hash_map<string,int>* context=0 ) = 0;
    
    // boardValueFromProbs calculates the equity using pre-calculated probabilities
    
    double boardValueFromProbs( const gameProbabilities& probs ) const;
    
    // overide boardValue so it returns the value from probs
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 );
};

#endif
