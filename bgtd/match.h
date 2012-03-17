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

#ifndef bgtd_match_h
#define bgtd_match_h

#include "board.h"
#include "common.h"
#include "doublestrat.h"
#include "game.h"
#include "randomc.h"
#include "strategy.h"

class match
{
    // represents a set of games to a fixed target score
    
public:
    match( int target, strategy * strat0, strategy * strat1, int seed, doublestrat* ds0, doublestrat* ds1 );
    ~match();
    
    // step moves one roll forward in the current game
    
    void step();
    
    // stepToGameEnd steps to the end of the current game
    
    void stepToGameEnd();
    
    // stepToEnd steps to the end of the whole match
    
    void stepToEnd();
    
    // matchOver returns true if the match is over, false otherwise
    
    bool matchOver() const;
    
    // winner returns 0 or 1 - throws if the match is not over
    
    int winner() const;
    
    // getGame returns a reference to the underlying game. inGame returns
    // true if we're in a game or false if we're before the first game,
    // in between games, or after the last game. (step() to move into a 
    // game if you're not at the end).
    
    bool inGame() const { return currentGame != 0; };
    game& getGame();
    
    // playerScore and opponentScore return score0 and score1
    
    int getTarget() const { return target; };
    int playerScore() const { return score0; };
    int opponentScore() const { return score1; };
    
private:
    int target;
    int score0;
    int score1;
    
    strategy * strat0;
    strategy * strat1;
    doublestrat * ds0;
    doublestrat * ds1;
    
    CRandomMersenne * rng;
    game * currentGame;
};

#endif
