//
//  game.h
//  bgtd
//
//  Created by Mark Higgins on 7/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_game_h
#define bgtd_game_h

#include "board.h"
#include "strategy.h"
#include "randomc.h"

class game
{
    // A game represents a game of backgammon. It holds a board, sorts out whose turn
    // it is, rolls dice, and advances the game according to the strategy of each
    // side.
    
public:
    // constructors - takes the strategys for each side (0 and 1) and the RNG seed
    
    game( strategy * strat0, strategy * strat1, int seed );
    game( const game& srcGame );
    
    // destructor
    
    virtual ~game();
    
    // public interface
    
    board getBoard();   // the underlying backgammon board
    int turn();         // 0 or 1, depending on whose turn it is
    void setTurn( int turn ); 
    void step();        // steps the game ahead
    void stepToEnd();   // steps to the end of the game
    
    bool gameOver();    // true when the game is done
    int  winner();      // if the game is over, returns 0 or 1; otherwise throws
    int  winnerScore(); // returns the number of points the winner won if the game is zero; if not it throws
    
    void setSeed( int newSeed ); // resets the RNG seed
    
    bool verbose; // if true it prints out stuff internally when it steps
    int nSteps;
    
    void setBoard( const board& newBrd ) { brd = newBrd; };   // sets the board for the game
    
private:
    board      brd;
    int        trn;
    strategy * strat0;
    strategy * strat1;
    
    CRandomMersenne * rng;
};

#endif
