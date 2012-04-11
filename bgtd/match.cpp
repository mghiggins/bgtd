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


match::match( int target, strategy * strat0, strategy * strat1, int seed )
: target(target), strat0(strat0), strat1(strat1)
{
    rng = new CRandomMersenne(seed);
    score0 = 0;
    score1 = 0;
    currentGame = 0; // flag that we're not currently in a game
    ownsGamePtr = true;
    nGames = 0;
    doneCrawford = false;
    verbose = false;
}

match::~match()
{
    delete rng;
    if( ownsGamePtr and currentGame ) delete currentGame;
}

void match::step()
{
    if( currentGame == 0 )
    {
        // if the current game pointer is 0, it means we need to set up a new game. We'll
        // wait for the next step to actually start playing it though.
        // Only exception: we're at the end of the match, in which case do nothing.
        
        if( score0 >= target or score1 >= target ) return;
        
        if( verbose )
        {
            cout << endl;
            cout << "***********\n";
            cout << "New game: current score " << score0 << " vs " << score1 << " to " << target << endl;
            cout << "***********\n";
            cout << endl;
        }
        
        currentGame = new game( strat0, strat1, rng );
        currentGame->verbose = verbose;
        ownsGamePtr = true;
        
        // if this is the Crawford game, tell the game that no doubling is allowed
        
        if( target-score0 == 1 or target-score1 == 1 )
        {
            if( verbose )
                cout << "Crawford game\n\n";
            currentGame->canDouble = false;
        }
    }
    else if( currentGame->gameOver() )
    {
        // increment the games count
        
        nGames ++;
        
        // if this was the Crawford game, note that it's done
        
        if( target-score0 == 1 or target-score1 == 1 )
            doneCrawford = true;
        
        // record the score
        
        if( currentGame->winner() == 0 )
            score0 += currentGame->winnerScore();
        else
            score1 += currentGame->winnerScore();
        
        if( verbose and ( score0 >= target or score1 >= target ) )
        {
            cout << endl;
            cout << "***********\n";
            cout << "Match over\n";
            cout << "Winner: player ";
            if( score0 >= target )
                cout << "0\n";
            else
                cout << "1\n";
            cout << "Final score:  " << score0 << " vs " << score1 << " to " << target << endl;
            cout << "Games played: " << nGames << endl;
        }
        
        // destroy the current game and set the pointer to 0 as a flag that we're in between games
        
        if( ownsGamePtr ) delete currentGame;
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

void match::setGame( game * newGame )
{
    currentGame = newGame;
    ownsGamePtr = false;
}

void match::setPlayerScore( int newScore )
{
    if( newScore < 0 or newScore >= target ) throw string( "Cannot set the score below zero or at or past the target" );
    score0 = newScore;
}

void match::setOpponentScore( int newScore )
{
    if( newScore < 0 or newScore >= target ) throw string( "Cannot set the score below zero or at or past the target" );
    score1 = newScore;
}