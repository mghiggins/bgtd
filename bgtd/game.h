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

#ifndef bgtd_game_h
#define bgtd_game_h

#import <hash_map.h>
#include "board.h"
#include "common.h"
#include "strategy.h"
#include "doublestrat.h"
#include "randomc.h"

class game
{
    // A game represents a game of backgammon. It holds a board, sorts out whose turn
    // it is, rolls dice, and advances the game according to the strategy of each
    // side.
    
public:
    // constructors - takes the strategys for each side (0 and 1) and the RNG seed
    
    game( strategy * strat0, strategy * strat1, int seed );
    game( strategy * strat0, strategy * strat1, CRandomMersenne * rng );
    game( const game& srcGame );
    
    // others that add doubling strategies for each player
    
    game( strategy * strat0, strategy * strat1, int seed, doublestrat * doubler0, doublestrat * doubler1 );
    game( strategy * strat0, strategy * strat1, doublestrat * doubler0, doublestrat * doubler1, CRandomMersenne * rng );
    
    // destructor
    
    virtual ~game();
    
    // public interface
    
    board getBoard();   // the underlying backgammon board
    int turn();         // 0 or 1, depending on whose turn it is
    void setTurn( int turn ); 
    void step();        // steps the game ahead
    void stepWithDice( const int& d1, const int& d2 ); // steps the game ahead given the dice rolls
    void stepToEnd();   // steps to the end of the game
    
    bool gameOver();    // true when the game is done
    int  winner();      // if the game is over, returns 0 or 1; otherwise throws
    int  winnerScore(); // returns the number of points the winner won if the game is zero; if not it throws
    
    void setSeed( int newSeed ); // resets the RNG seed
    
    bool verbose; // if true it prints out stuff internally when it steps
    int nSteps;
    
    void setBoard( const board& newBrd ) { brd = newBrd; };   // sets the board for the game
    
    // the game can pass context down to the strategies when evaluating board values.
    // This can be anything that fits into a string->int hash. In practise it's used for
    // things like 
    
    void setContextValue( const string& elemName, int elemVal ) { gameContext[elemName] = elemVal; };
    int getContextValue( const string& elemName ) const;
    
    // doubling cube methods
    
    // getCube returns the current doubling cube setting (1 if it's still centered).
    // doubleCube doubles the value.
    
    int getCube() const { return cube; };
    void doubleCube() { cube *= 2; };
    
private:
    board      brd;
    int        trn;
    strategy * strat0;
    strategy * strat1;
    
    doublestrat * doubler0;
    doublestrat * doubler1;
    
    int cube;
    int cubeOwner; // 0 or 1 if player 0 or 1 owns; -1 if the cube is in the middle
    int cubedOutPlayer; // 0 or 1 if player 0 or 1 has doubled and the other passed, ending the game. -1 if that hasn't happened.
    
    CRandomMersenne * rng;
    bool myRng;
    
    hash_map<string,int> gameContext;
};

#endif
