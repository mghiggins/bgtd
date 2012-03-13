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

#ifndef bgtd_doublefns_h
#define bgtd_doublefns_h

#include <string>
#include "strategyprob.h"

// writeCrawfordFirstDoubleStateProbDb writes out a database that is used in post-Crawford
// match equity calculations. Each state is either after one roll of the
// dice or two rolls of the dice from the starting board, and together
// represent every state reachable in one or two rolls. For each state
// we save the probability of reaching that state at the first point
// the opponent can offer a double (1/30 for the first
// step, and either 1/30/36 or 1/30/18 for the second step, depending on
// whether the second roll is a double or not). We also save the full
// set of cubeless game probabilities (prob of any win, prob of any
// gammon win, prob of any backgammon win, prob of any gammon loss, and
// prob of any backgammon loss). Those are used when backward-inducting
// to get the post-Crawford match equities for player 1-away and opponent
// n-away. The args are the file name where we'll write out the db and the strategy
// we'll use to calculate the probs and moves; also whether to optimize moves for
// an outright win or for full equity.

void writeCrawfordFirstDoubleStateProbDb( const string& dbFileName, strategyprob& strat, bool outrightWin );

// loadCrawfordFirstDoubleStateProbDb loads data from the database and returns it as a vector
// of stateData

class stateData
{
public:
    stateData( double stateProb, const gameProbabilities& stateGameProbs ) : stateProb(stateProb), stateGameProbs(stateGameProbs) {};
    
    double stateProb;
    gameProbabilities stateGameProbs;
};

vector<stateData> loadCrawfordFirstDoubleStateProbDb( const string& dbFileName );

// matchEquityPostCrawford returns the match equity (+1 for a match win, -1 for a match loss, 2*prob of winning match-1 in
// general) for a match where the Crawford game has passed, the player is 1-away, and the opponent is nGames away.
// gamProb is the probability that the player wins a gammon in the game, before the first dice are thrown (symmetric
// for player and opponent of course). The equity is for right before the game starts and even the first die throw hasn't happened.

double matchEquityPostCrawford( int nGames, double gamProb, const vector<stateData>& singleData, const vector<stateData>& data );

// matchEquityCrawford returns the match requity for a match where we're on the Crawford game; the player is 1-away and the
// opponent is nGames-away. The equity is for right before the game starts and even the first die throw hasn't happened.

double matchEquityCrawford( int nGames, double gamProb, const vector<stateData>& singleData, const vector<stateData>& data );

#endif
