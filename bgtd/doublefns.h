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

double matchEquityPostCrawford( int nGames, double gamProb, const vector<stateData>& singleData, const vector<stateData>& data, bool useCache );

// matchEquityCrawford returns the match requity for a match where we're on the Crawford game; the player is 1-away and the
// opponent is nGames-away. The equity is for right before the game starts and even the first die throw hasn't happened.

double matchEquityCrawford( int nGames, double gamProb, const vector<stateData>& singleData, const vector<stateData>& data, bool useCache );

// matchEquity returns the pre-Crawford match equity for player n-away and opponent m-away. Includes Crawford
// game match equities for n or m == 1. Represents match equity before the game starts, so cube centered at 1 and
// before the first dice throw (assumes equal players, so both have 50% prob of win).

double matchEquity( int n, int m, double gamProb, const vector<stateData>& singleData, const vector<stateData>& data, bool useCache );

class interpMEdata
{
public:
    interpMEdata( double takePoint, double cashPoint, double takeME, double cashME ) : takePoint(takePoint), cashPoint(cashPoint), takeME(takeME), cashME(cashME) {};
    
    // operator() interpolates a match equity given a probability of (any) win
    
    double operator()( double probWin )
    {
        if( probWin < takePoint ) return takeME;
        if( probWin > cashPoint ) return cashME;
        
        return ( ( probWin - takePoint ) * cashME + ( cashPoint - probWin ) * takeME ) / ( cashPoint - takePoint );
    }
    
    // solve finds the probability such that match equity equals what they passed in. Throws if the resulting
    // probability is outside [takePoint,matchPoint].
    
    double solve( double ME )
    {
        if( ME < takeME or ME > cashME ) throw string( "Cannot interpolate outside [ME(take point),ME(cash point)] range" );
        
        return ( ( ME - takeME ) * cashPoint + ( cashME - ME ) * takePoint ) / ( cashME - takeME );
    }
    
    double takePoint, cashPoint;
    double takeME, cashME;
};

// matchEquityInterpData returns the data that defines match equity for a given cube level as a function of 
// probability of win. It assumes that the fraction of wins that are gammons stays constant as prob of win
// changes. gamProb is the same as above: the fraction of games that the player wins a gammon (so half the
// probability that a player win is a gammon win).

interpMEdata matchEquityInterpData( int n, int m, int cube, bool playersCube, double gamProb, const vector<stateData>& singleData, const vector<stateData>& data, bool useCache );

// matchEquityBI runs a little PDE solver to value the match equity

double matchEquityBI( int n, int m, double gamProb, const vector<stateData>& singleData, const vector<stateData>& data, double sigma, int nP, int nT, double timeMultiple );

// writeMatchEquityTable calculates the match equities on a grid and writes them out to a file. Writes out
// pre-Crawford match equities and post-Crawford match equities. loadMatchEquityTable loads the match equities
// from a file.

void writeMatchEquityTable( double gamProb, const vector<stateData>& singleData, const vector<stateData>& data, const string& fileName );
void loadMatchEquityTable( const string& fileName );

struct METData
{
    int nGames;
    vector<double> equitiesPostCrawford;
    vector< vector<double> > equitiesPreCrawford;
};

METData loadMatchEquityTableData( const string& fileName );

// match equities from cache

double matchEquityPostCrawfordCached( int n );
double matchEquityCached( int n, int m );
double matchEquityTableSize();

#endif
