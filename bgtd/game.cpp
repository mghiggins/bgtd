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
    myRng = true;
    verbose = false;
    nSteps = 0;
    trn = 0;
    
    doubler0 = doubler1 = 0;
    cube = 1;
    cubedOutPlayer = -1;
    cubeOwner = -1;
}

game::game( strategy * strat0, strategy * strat1, CRandomMersenne * rng )
{
    this->strat0 = strat0;
    this->strat1 = strat1;
    this->rng = rng;
    myRng = false;
    verbose = false;
    nSteps = 0;
    trn = 0;
    
    doubler0 = doubler1 = 0;
    cube = 1;
    cubedOutPlayer = -1;
    cubeOwner = -1;
}

game::game( const game& srcGame )
{
    brd      = srcGame.brd;
    strat0   = srcGame.strat0;
    strat1   = srcGame.strat1;
    verbose  = srcGame.verbose;
    nSteps   = srcGame.nSteps;
    trn      = srcGame.trn;
    doubler0 = srcGame.doubler0;
    doubler1 = srcGame.doubler1;
    cube     = srcGame.cube;
    cubedOutPlayer = srcGame.cubedOutPlayer;
    cubeOwner      = srcGame.cubeOwner;
}

game::game( strategy * strat0, strategy * strat1, int seed, doublestrat * doubler0, doublestrat * doubler1 )
{
    this->strat0 = strat0;
    this->strat1 = strat1;
    rng = new CRandomMersenne(seed);
    myRng = true;
    verbose = false;
    nSteps = 0;
    trn = 0;
    
    this->doubler0 = doubler0;
    this->doubler1 = doubler1;
    cube = 1;
    cubedOutPlayer = -1;
    cubeOwner = -1;
}

game::game( strategy * strat0, strategy * strat1, doublestrat * doubler0, doublestrat * doubler1, CRandomMersenne * rng )
{
    this->strat0 = strat0;
    this->strat1 = strat1;
    this->rng = rng;
    myRng   = false;
    verbose = false;
    nSteps  = 0;
    trn     = 0;
    
    this->doubler0 = doubler0;
    this->doubler1 = doubler1;
    cube = 1;
    cubedOutPlayer = -1;
    cubeOwner = -1;
}

game::~game()
{
    if( myRng )
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
    // check for a double
    
    doublestrat * doublerPlayer;
    doublestrat * doublerOpponent;
    if( trn == 0 )
    {
        doublerPlayer   = doubler0;
        doublerOpponent = doubler1;
    }
    else
    {
        doublerPlayer   = doubler1;
        doublerOpponent = doubler0;
    }
    
    // if they haven't specified a player doubler, it'll never offer a double. Otherwise, check.
    // Can only double if player has the cube or it's in the middle. Also can't double before the
    // first roll. Also can't double if the cube's already at 64.
    
    brd.setPerspective(trn); // make sure the board is from the appropriate perspective when checking double stuff
    
    if( nSteps > 0 and cube < 64 and doublerPlayer and ( cubeOwner == -1 or cubeOwner == trn ) and doublerPlayer->offerDouble(brd,cube) )
    {
        // if there's no opponent doubler specified, by arbitrary convention the opponent always takes.
        // Otherwise, check the doubling strategy.
        
        if( !doublerOpponent or doublerOpponent->takeDouble(brd,cube) )
        {
            doubleCube();
            cubeOwner = 1 - trn;
        }
        else
        {
            // the player automatically wins a single game
            
            cubedOutPlayer = trn;
        }
    }
    
    // if the game isn't over (ie neither player has been doubled and passed), roll the
    // dice and move.
    
    if( cubedOutPlayer == -1 )
    {
        // roll the dice
        
        int d1 = rng->IRandom(1,6);
        int d2 = rng->IRandom(1,6);
        
        // if it's the first roll, the dice can't be the same, and we'll choose the player based on which is bigger
        
        while( nSteps == 0 and d1 == d2 )
        {
            d1 = rng->IRandom(1, 6);
            d2 = rng->IRandom(1, 6);
        }
        
        if( nSteps == 0 )
        {
            if( d1 > d2 )
                trn = 0;
            else
                trn = 1;
        }
        
        // step with them
        
        stepWithDice( d1, d2 );
    }
}

void game::stepWithDice( const int& d1, const int& d2 )
{
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
    
    // let the strategy choose the preferred move based on its internal evaluation function
    
    brd = strat->preferredBoard( oldBoard, moves, &gameContext );
    
    // if the strategy wants to update itself, do it. This eg trains neural networks.
    
    if( strat->needsUpdate() )
        strat->update( oldBoard, brd );
    
    trn = 1 - trn;
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
    if( cubedOutPlayer != -1 ) return true;
    
    if( brd.bornIn() == 15 || brd.otherBornIn() == 15 )
        return true;
    else
        return false;
}

int game::winner()
{
    // if a player cubed out, it means they doubled they opponent and the opponent passed
    
    if( cubedOutPlayer != -1 )
        return cubedOutPlayer;
    
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
        if( cubedOutPlayer != -1 )
            return cube;
        
        if( brd.bornIn() == 15 )
        {
            // check if it was a regular win
            if( brd.otherBornIn() != 0 ) return cube;
            
            // check for a backgammon
            if( brd.otherHit() != 0 ) return 3*cube;
            for( int i=0; i<6; i++ )
                if( brd.otherChecker( i ) != 0 ) return 3*cube;
            
            // otherwise a regular gammon
            return 2*cube;
        }
        else
        {
            // check if it was a regular win
            if( brd.bornIn() != 0 ) return cube;
            
            // check for a backgammon
            if( brd.hit() != 0 ) return 3*cube;
            for( int i=18; i<24; i++ )
                if( brd.checker( i ) != 0 ) return 3*cube;
            
            // otherwise a regular gammon
            return 2*cube;
        }
    }
    else
        throw string("Game is not over");
}

int game::getContextValue( const string& elemName ) const
{
    hash_map<string,int>::const_iterator it = gameContext.find( elemName );
    if( it == gameContext.end() )
        throw "No such element in the game context";
    return it->second;
}
