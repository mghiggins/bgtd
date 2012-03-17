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

#include "match.h"


match::match( int target, strategy * strat0, strategy * strat1, int seed, doublestrat* ds0, doublestrat* ds1 )
: target(target), strat0(strat0), strat1(strat1), ds0(ds0), ds1(ds1)
{
    rng = new CRandomMersenne(seed);
    score0 = 0;
    score1 = 0;
    currentGame = 0; // flag that we're not currently in a game
}

match::~match()
{
    delete rng;
    delete currentGame;
}

void match::step()
{
    if( currentGame == 0 )
    {
        // if the current game pointer is 0, it means we need to set up a new game. We'll
        // wait for the next step to actually start playing it though.
        // Only exception: we're at the end of the match, in which case do nothing.
        
        if( score0 >= target or score1 >= target ) return;
        
        currentGame = new game( strat0, strat1, ds0, ds1, rng );
    }
    else if( currentGame->gameOver() )
    {
        // record the score
        
        if( currentGame->winner() == 0 )
            score0 += currentGame->winnerScore();
        else
            score1 += currentGame->winnerScore();
        
        // destroy the current game and set the pointer to 0 as a flag that we're in between games
        
        delete currentGame;
        currentGame = 0;
    }
    else
    {
        currentGame->step();
    }
}

void match::stepToGameEnd()
{
    if( currentGame == 0 )
        step();
    
    while( currentGame != 0 )
        step();
}

void match::stepToEnd()
{
    while( score0 < target and score1 < target )
        step();
}

bool match::matchOver() const
{
    return score0 >= target or score1 >= target;
}

int match::winner() const
{
    if( !matchOver() ) throw string( "The match is not over - cannot give a winner yet" );
    if( score0 >= target )
        return 0;
    else
        return 1;
}

game& match::getGame()
{
    if( currentGame == 0 ) throw string( "No current game" );
    return *currentGame;
}

