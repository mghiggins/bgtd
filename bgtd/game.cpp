//
//  game.cpp
//  bgtd
//
//  Created by Mark Higgins on 7/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <string>
#include "game.h"
#include "board.h"
#include "strategy.h"
#include "randomc.h"
#include "gamefns.h"

game::game( strategy * strat0, strategy * strat1, int seed )
{
    this->strat0 = strat0;
    this->strat1 = strat1;
    rng = new CRandomMersenne(seed);
    verbose = false;
    nSteps = 0;
    trn = 0;
}

game::game( const game& srcGame )
{
    brd     = srcGame.brd;
    strat0  = srcGame.strat0;
    strat1  = srcGame.strat1;
    verbose = srcGame.verbose;
    nSteps  = srcGame.nSteps;
    trn     = srcGame.trn;
}

game::~game()
{
    delete rng;
}

board game::getBoard()
{
    return brd;
}

int game::turn()
{
    return trn;
}

void game::setTurn( int turn )
{
    if( turn != 0 && turn != 1 ) throw string( "turn must be 0 or 1" );
    trn = turn;
}

void game::step()
{
    // roll the dice
    
    int d1 = rng->IRandom(1,6);
    int d2 = rng->IRandom(1,6);
    
    if( verbose )
    {
        if( trn == 0 )
            cout << "White turn: ";
        else
            cout << "Black turn: ";
        cout << "( " << d1 << ", " << d2 << " )\n";
    }
    
    // set the perspective of the board to be of the player that's making this move
    
    brd.setPerspective( trn );
    
    // keep track of the original state of the board (used for strategies that update)
    
    board oldBoard(brd);
    
    // figure out the possible moves
    
    set<board> moves = possibleMoves( brd, d1, d2 );
    
    // use the strategy to value the moves
    
    strategy * strat;
    if( trn == 0 )
        strat = strat0;
    else
        strat = strat1;
    
    brd = strat->preferredBoard( oldBoard, moves );
    
    // if the strategy wants to update itself, do it
    
    if( strat->needsUpdate() )
        strat->update( oldBoard, brd );
    
    trn = ( trn + 1 ) % 2;
    brd.setPerspective( 0 ); // always leave the board in perspective 0 so that it prints out consistently
    nSteps ++;
    
    if( verbose )
    {
        brd.print();
        cout << endl;
        brd.validate();
    }
}

void game::stepToEnd()
{
    if( verbose ) brd.print(); // show the starting config
    while( !this->gameOver() )
        this->step();
}

bool game::gameOver()
{
    if( brd.bornIn() == 15 || brd.otherBornIn() == 15 )
        return true;
    else
        return false;
}

int game::winner()
{
    if( gameOver() )
    {
        int winner;
        if( brd.bornIn() == 15 )
            winner = 0;
        else
            winner = 1;
        if( brd.perspective() == 1 )
            winner = ( winner + 1 ) % 2;
        return winner;
    }
    else
        throw string("Game is not over");
}

int game::winnerScore()
{
    if( gameOver() )
    {
        if( brd.bornIn() == 15 )
        {
            // check if it was a regular win
            if( brd.otherBornIn() != 0 ) return 1;
            
            // check for a backgammon
            if( brd.otherHit() != 0 ) return 3;
            for( int i=0; i<6; i++ )
                if( brd.otherChecker( i ) != 0 ) return 3;
            
            // otherwise a regular gammon
            return 2;
        }
        else
        {
            // check if it was a regular win
            if( brd.bornIn() != 0 ) return 1;
            
            // check for a backgammon
            if( brd.hit() != 0 ) return 3;
            for( int i=18; i<24; i++ )
                if( brd.checker( i ) != 0 ) return 3;
            
            // otherwise a regular gammon
            return 2;
        }
    }
    else
        throw string("Game is not over");
}
