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

#include <algorithm>
#include <boost/thread.hpp>
#include <map>
#include <iostream>
#include "gamefns.h"
#include "game.h"

set<board> subPossibleMoves( const board& brd, vector<int> rolls );

set<board> subPossibleMoves( const board& brd, vector<int> rolls )
{
    // local fn called recursively for each roll
    
    set<board> moves;
    
    int roll = rolls.at(0);
    
    // if there are any checkers hit, try to get them in. If they can't
    // get in, there's nothing to do.
    
    if( brd.hit() > 0 )
    {
        // see if there's a free spot in the enemy's home board
        
        if( brd.otherChecker( 24-roll ) > 1 )
            return moves; // nothing one can do
        
        // create a new board we'll use to track this move
        
        board newBrd( brd );
        
        // set the checker onto the spot, since it just moved in from the bar
        
        newBrd.setChecker( 24-roll, brd.checker(24-roll)+1 );
        newBrd.decrementHit();
        
        
        // if there was an enemy checker there, remove it to the bar
        
        if( brd.otherChecker( 24-roll ) == 1 )
        {
            newBrd.setOtherChecker( 24-roll, 0 );
            newBrd.incrementOtherHit();
        }
        
        // if there are no more rolls, return just this board
        
        if( rolls.size() == 1 )
        {
            moves.insert( newBrd );
            return moves;
        }
        
        // get the moves from here
        
        vector<int> subRolls;
        subRolls.insert( subRolls.end(), rolls.begin() + 1, rolls.end() );
        
        set<board> nextMoves = subPossibleMoves( newBrd, subRolls );
        if( nextMoves.size() == 0 )
            moves.insert( newBrd );
        else
            moves.insert( nextMoves.begin(), nextMoves.end() );
        
        return moves;
    }
    
    // deal with the first roll - try moving each available piece
    
    int i, j;
    bool skip;
    
    for( i=0; i<24; i++ )
    {
        // if there isn't a piece there, skip the spot
        
        if( brd.checker( i ) == 0 ) continue;
        
        board newBrd( brd );

        // break up into spots where you can move the piece inside the board,
        // and those where moving would mean bearing off
        
        if( i > roll-1 )
        {
            // deal with the subset of moves where you can move the piece without
            // leaving the board.
            
            // if there's a piece but there's nowhere to move it, skip the spot
            
            if( brd.otherChecker( i-roll ) > 1 ) continue;
            
            // okay to move there; set up a new board with the piece moved
            
            newBrd.setChecker( i-roll, brd.checker( i-roll ) + 1 );
            newBrd.setChecker( i, brd.checker(i) - 1 );
            
            // if there is a single enemy there, hit him
            
            if( brd.otherChecker( i-roll ) == 1 )
            {
                newBrd.setOtherChecker( i-roll, 0 );
                newBrd.incrementOtherHit();
            }
        }
        else
        {
            // then moves where you might be able to bear off
            
            // start with seeing whether there are any pieces outside the home box - if so, nothing to do
            
            if( brd.hit() ) continue; // any pieces hit means you can't bear in
            skip = false;
            for( j=6; j<24; j++ )
                if( brd.checker(j) != 0 )
                {
                    skip = true;
                    break;
                }
            if( skip ) continue;
            
            // so potential to bear off. If we've got exactly the right roll, done. Otherwise need to see
            // if there are any pieces in a higher spot.
            
            if( i < roll-1 )
            {
                skip = false;
                for( j=i+1; j<6; j++ )
                    if( brd.checker(j) != 0 )
                    {
                        skip = true;
                        break;
                    }
                if( skip ) continue;
            }
            
            newBrd.setChecker( i, brd.checker(i)-1 );
            newBrd.incrementBornIn();
        }
        
        // if we made it this far, there's a move to do.
        
        // if there are no more rolls, add the board to the list and continue
        
        if( rolls.size() == 1 )
        {
            moves.insert( newBrd );
            continue;
        }
        
        // otherwise start here and generate the rest of the boards
        
        vector<int> subRolls;
        subRolls.insert( subRolls.end(), rolls.begin() + 1, rolls.end() );
        
        set<board> nextMoves = subPossibleMoves( newBrd, subRolls );
        if( nextMoves.size() == 0 )
            moves.insert( newBrd );
        else
            moves.insert( nextMoves.begin(), nextMoves.end() );
    }
    
    return moves;
}

set<board> possibleMoves( const board& brd, int die1, int die2 )
{
    // set up the rolls we have - normally two, or four if it's a double
    
    vector<int> rolls;
    rolls.insert( rolls.end(), die1 );
    rolls.insert( rolls.end(), die2 );
    if( die1 == die2 )
    {
        rolls.insert( rolls.end(), die1 );
        rolls.insert( rolls.end(), die1 );
    }
    
    // generate the possible moves
    
    set<board> moves = subPossibleMoves( brd, rolls );
    
    // if the dice aren't all the same, try them in the reverse order
    
    if( die1 != die2 )
    {
        vector<int> revRolls = rolls;
        reverse( revRolls.begin(), revRolls.end() );
        
        set<board> newMoves = subPossibleMoves( brd, revRolls );
        moves.insert( newMoves.begin(), newMoves.end() );
    }
    
    // filter down the boards to ones that have the min pips - this automatically ensures
    // that all possible dice rolls that can be used are used.
    
    map<board,int> movePips;
    int minPips=30000, pips;
    for( set<board>::iterator i=moves.begin(); i!=moves.end(); i++ )
    {
        pips = (*i).pips();
        movePips[*i] = pips;
        if( pips < minPips )
            minPips = pips;
    }
    
    set<board> filtMoves;
    for( map<board,int>::iterator i=movePips.begin(); i!=movePips.end(); i++ )
    {
        if( i->second == minPips )
            filtMoves.insert( i->first );
    }
    
    return filtMoves;
}

bool isRace( const board& brd )
{
    return brd.isRace();
}

gameProbabilities rolloutBoardProbabilitiesVarReduction( const board& brd, strategyprob& strat, int nRuns, int seed )
{
    CRandomMersenne rng(seed);
    
    return rolloutBoardProbabilitiesVarReduction( brd, strat, nRuns, &rng );
}

gameProbabilities rolloutBoardProbabilitiesVarReduction( const board& brd, strategyprob& strat, int nRuns, CRandomMersenne * rng )
{
    // nRuns must be a multiple of 21
    
    if( nRuns % 21 != 0 ) throw "nRuns must be a multiple of 21";
    
    int d1, d2, count=0;
    double weight;
    gameProbabilities val;
    
    for( d1=1; d1<=6; d1++ )
        for( d2=1; d2<=d1; d2++ )
        {
            if( d1 == d2 )
                weight = 1/36.;
            else
                weight = 1/18.;
            
            game g(&strat,&strat,1); // seed doesn't matter here
            g.setBoard(brd);
            g.setTurn(1-brd.perspective()); // the oppponent's turn - we calculate board value after the player's move
            g.stepWithDice( d1, d2 );
            
            board b( g.getBoard() );
            b.setPerspective(1-brd.perspective());
            
            // rollout from this point, starting with the opponent's perspective
            
            gameProbabilities bv = rolloutBoardProbabilities( b, strat, nRuns/21, rng );
            //cout << d1 << "; " << d2 << ": " << bv << endl;
            
            val = val - bv * weight;
            count++;
        }
    
    return val;
}

gameProbabilities rolloutBoardProbabilities( const board& brd, strategyprob& strat, int nRuns, int seed )
{
    CRandomMersenne rng(seed);
    return rolloutBoardProbabilities( brd, strat, nRuns, &rng );
}

gameProbabilities rolloutBoardProbabilities( const board& brd, strategyprob& strat, int nRuns, CRandomMersenne * rng )
{
    gameProbabilities avgVal;
    
    for( int i=0; i<nRuns; i++ )
    {
        // start a new game and run it to completion
        
        game g( &strat, &strat, rng );
        g.setBoard( brd );
        g.setTurn( 1 - brd.perspective() ); // start with opponent on roll
        
        g.stepToEnd();
        
        // get the value from the perspective of the board
        
        if( g.winner() == brd.perspective() )
        {
            avgVal.probWin += 1;
            if( g.winnerScore() > 1 ) avgVal.probGammonWin += 1;
            if( g.winnerScore() > 2 ) avgVal.probBgWin += 1;
        }
        else
        {
            if( g.winnerScore() > 1 ) avgVal.probGammonLoss += 1;
            if( g.winnerScore() > 2 ) avgVal.probBgLoss += 1;
        }
    }
    
    avgVal = avgVal / nRuns;
    return avgVal;
}

vector<gameProbabilities> valsRollout;

class workerRollout
{
public:
    workerRollout( const board& brd, strategyprob& strat, int nRuns, int seed, int index, bool varReduc ) : brd(brd), strat(strat), nRuns(nRuns), seed(seed), index(index), varReduc(varReduc) {};
    
    void operator()()
    {
        CRandomMersenne rng(seed);
        gameProbabilities val;
        if( varReduc )
            val = rolloutBoardProbabilitiesVarReduction( brd, strat, nRuns, &rng );
        else
            val = rolloutBoardProbabilities( brd, strat, nRuns, &rng );
        valsRollout.at(index) = val;
    }
    
private:
    const board& brd;
    strategyprob& strat;
    int nRuns;
    int seed;
    int index;
    bool varReduc;
};

gameProbabilities rolloutBoardProbabilitiesParallel( const board& brd, strategyprob& strat, int nRuns, int seed, int nThreads, bool varReduc )
{
    if( varReduc and nRuns % ( 21 * nThreads ) != 0 ) throw "nRuns must be a multiple of 21*nThreads";
    
    using namespace boost;
    
    // initialize the vector that'll hold the results
    
    valsRollout.resize( nThreads );
    
    // break the runs into nThreads pieces. For each one we'll use a seed incremented by the bucket
    // size. This isn't properly parallelization but it should be okay. Really we should use a PRNG
    // like SPRNG.
    
    thread_group ts;
    int runsPerThread = nRuns / nThreads;
    
    // the buckets from 0->nThreads-2 contain runsPerThread each
    
    for( int i=0; i<nThreads-1; i++ )
        ts.create_thread( workerRollout( brd, strat, runsPerThread, seed+i*runsPerThread, i, varReduc ) );
    
    // the last bucket contains the rest
    
    int lastNRuns = nRuns - runsPerThread * ( nThreads - 1 );
    bool useLast=false;
    if( lastNRuns > 0 )
    {
        ts.create_thread( workerRollout( brd, strat, nRuns - runsPerThread * ( nThreads - 1 ), seed+(nThreads-1)*runsPerThread, nThreads-1, varReduc ) );
        useLast = true;
    }
    
    // wait for the calcs
    
    ts.join_all();
    
    // calculate the average of the averages
    
    int count=nThreads-1;
    gameProbabilities val;
    for( int i=0; i<nThreads-1; i++ )
        val = val + valsRollout.at(i);
    if( useLast )
    {
        count ++;
        val = val + valsRollout.at(nThreads-1);
    }
    
    val = val / count;
    return val;
}

vector< set<roll> > * hittingRolls=0;
vector< vector<int> > * shotIndexes=0;

void setupHittingRolls()
{
    // sets up the list that records the indirect ways to hit a blot for
    // different numbers of steps away. "Indirect" means ways to hit with
    // two dice, excluding any direct hits with a single die.
    
    if( hittingRolls != 0 ) return; // nothing to do
    
    // define the vector of hitting rolls
    
    hittingRolls = new vector< set<roll> >;
    
    // make it 24 elements long - but several of those elements are empty because there's no
    // roll where an opponent can hit a blot that's 13, 14, 19, 21, 22, or 23 steps away
    
    hittingRolls->resize(24);
    hittingRolls->at(1).insert( roll(1,1) ); // 2: (1,1)
    hittingRolls->at(2).insert( roll(1,1) ); // 3: (1,1),(1,2)
    hittingRolls->at(2).insert( roll(1,2) );
    hittingRolls->at(3).insert( roll(1,1) ); // 4: (1,1),(2,2),(1,3)
    hittingRolls->at(3).insert( roll(2,2) );
    hittingRolls->at(3).insert( roll(1,3) );
    hittingRolls->at(4).insert( roll(1,4) ); // 5: (1,4),(2,3)
    hittingRolls->at(4).insert( roll(2,3) );
    hittingRolls->at(5).insert( roll(2,2) ); // 6: (2,2), (3,3), (1,5), (2,4)
    hittingRolls->at(5).insert( roll(3,3) );
    hittingRolls->at(5).insert( roll(1,5) );
    hittingRolls->at(5).insert( roll(2,4) );
    hittingRolls->at(6).insert( roll(1,6) ); // 7: (1,6), (2,5), (3,4)
    hittingRolls->at(6).insert( roll(2,5) );
    hittingRolls->at(6).insert( roll(3,4) );
    hittingRolls->at(7).insert( roll(2,2) ); // 8: (2,2), (4,4), (2,6), (3,5)
    hittingRolls->at(7).insert( roll(4,4) );
    hittingRolls->at(7).insert( roll(2,6) );
    hittingRolls->at(7).insert( roll(3,5) );
    hittingRolls->at(8).insert( roll(3,3) ); // 9: (3,3), (3,6), (4,5)
    hittingRolls->at(8).insert( roll(3,6) );
    hittingRolls->at(8).insert( roll(4,5) );
    hittingRolls->at(9).insert( roll(5,5) ); // 10: (5,5), (4,6)
    hittingRolls->at(9).insert( roll(4,6) );
    hittingRolls->at(10).insert( roll(5,6) ); // 11: (5,6)
    hittingRolls->at(11).insert( roll(3,3) ); // 12: (3,3), (4,4), (6,6)
    hittingRolls->at(11).insert( roll(4,4) );
    hittingRolls->at(11).insert( roll(6,6) );
    hittingRolls->at(14).insert( roll(5,5) ); // 15: (5,5)
    hittingRolls->at(15).insert( roll(4,4) ); // 16: (4,4)
    hittingRolls->at(17).insert( roll(6,6) ); // 18: (6,6)
    hittingRolls->at(19).insert( roll(5,5) ); // 20: (5,5)
    hittingRolls->at(23).insert( roll(6,6) ); // 24: (6,6)
    
    // set up the table for (i,j) roll (with i<=j) to index in a 21-element list.
    // First index is first die - 1 (0-based), and second index is second die - first die
    // (assumes first die <= second die).
    
    shotIndexes = new vector< vector<int> >;
    
    for( int i=1; i<=6; i++ )
    {
        int indexLow = (-i*i+15*i-14)/2;
        vector<int> subIndexes;
        for( int j=i; j<=6; j++ )
            subIndexes.push_back( indexLow + j-i );
        shotIndexes->push_back(subIndexes);
    }
}

set<roll> hittingShots( const board& brd, bool forOpponent )
{
    // if we haven't initialized the hitting rolls list yet, do so now
    
    if( hittingRolls == 0 ) setupHittingRolls();
    
    // find all the player's blots and note which opponent rolls would hit them
    
    set<roll> shots;
    
    vector<int> checkers, otherCheckers;
    int otherHit;
    
    if( forOpponent )
    {
        checkers = brd.checkers();
        otherCheckers = brd.otherCheckers();
        otherHit = brd.otherHit();
    }
    else
    {
        checkers = brd.otherCheckers();
        otherCheckers = brd.checkers();
        reverse( checkers.begin(), checkers.end() );
        reverse( otherCheckers.begin(), otherCheckers.end() );
        
        otherHit = brd.hit();
    }
    int i, j, k, diff, nOtherChecker;
    bool checkIndirect;
    set<roll>::iterator it;
    
    for( i=0; i<24; i++ )
    {
        // if there's more than one checker on the bar, a checker outside the home board can't be
        // hit
        
        if( otherHit > 1 and i > 5 ) break;
        
        // otherwise check for a blot
           
        if( checkers[i] == 1 )
        {
            // found a blot - see whether anything can hit it. Start at
            // position "-1" which means a hit checker.
            
            for( j=-1; j<i; j++ )
            {
                diff = i-j;
                
                // some # of steps have no way to hit - skip them
                
                if( diff == 13 || diff == 14 || diff == 19 || diff == 21 || diff == 22 || diff == 23 ) continue;
                
                // if we're looking at a checker on the bar and there's two there, or if we're looking
                // at a checker away from the bar and there's a checker on it, we can only use direct hits
                // (doubles treated a bit separately).
                
                if( ( j == -1 and otherHit > 1 ) or ( j > -1 and otherHit > 0 ) )
                    checkIndirect = false;
                else
                    checkIndirect = true;
                
                if( j == -1 )
                    nOtherChecker = otherHit;
                else
                    nOtherChecker = otherCheckers.at(j);
                
                if( nOtherChecker > 0 )
                {
                    // if the diff is 1-6, we add all rolls that include that die. For comparison
                    // purposes we always put the smaller die first in the roll object.
                    
                    if( diff < 7 )
                    {
                        for( k=1; k<=diff; k++ ) shots.insert( roll(k,diff) );
                        for( k=diff+1; k<7; k++ ) shots.insert( roll(diff,k) );
                    }
                    
                    // for each possible roll that could hit it indirectly, check whether it's valid
                    
                    for( it=hittingRolls->at(diff-1).begin(); it!=hittingRolls->at(diff-1).end(); it++ )
                    {
                        if( not checkIndirect and it->die1 != it->die2 ) continue;
                        if( otherHit == 1 and it->die1 == it->die2 and diff == 4 * it->die1 ) continue;
                        
                        // if both intermediate slots are covered by the player, the opponent can't
                        // use the roll
                        
                        if( checkers[i-it->die1] > 1 and checkers[i-it->die2] > 1 ) continue;
                        
                        // otherwise it's a possibility
                        
                        shots.insert( (*it) );
                    }
                }
            }
        }
    }
    
    return shots;
}

double hittingProb( const board& brd, bool forOpponent )
{
    return hittingProb( hittingShots( brd, forOpponent ) );
}

double hittingProb( const set<roll>& shots )
{
    double count=0;
    for( set<roll>::const_iterator it=shots.begin(); it!=shots.end(); it++ )
    {
        if( it->die1 == it->die2 )
            count += 1;
        else
            count += 2;
    }
    
    return count/36.;
}

double hittingProb2( const board& brd, bool forOpponent )
{
    // if we haven't initialized the hitting rolls list yet, do so now
    
    if( hittingRolls == 0 ) setupHittingRolls();
    
    // find all the player's blots and note which opponent rolls would hit them
    
    vector<bool> shots(21,false);
    
    vector<int> checkers, otherCheckers;
    int otherHit;
    
    if( forOpponent )
    {
        checkers = brd.checkers();
        otherCheckers = brd.otherCheckers();
        otherHit = brd.otherHit();
    }
    else
    {
        checkers = brd.otherCheckers();
        otherCheckers = brd.checkers();
        reverse( checkers.begin(), checkers.end() );
        reverse( otherCheckers.begin(), otherCheckers.end() );
        
        otherHit = brd.hit();
    }
    int i, j, k, diff, nOtherChecker;
    bool checkIndirect;
    set<roll>::iterator it;
    
    for( i=0; i<24; i++ )
    {
        // if there's more than one checker on the bar, a checker outside the home board can't be
        // hit
        
        if( otherHit > 1 and i > 5 ) break;
        
        // otherwise check for a blot
        
        if( checkers[i] == 1 )
        {
            // found a blot - see whether anything can hit it. Start at
            // position "-1" which means a hit checker.
            
            for( j=-1; j<i; j++ )
            {
                diff = i-j;
                
                // some # of steps have no way to hit - skip them
                
                if( diff == 13 || diff == 14 || diff == 19 || diff == 21 || diff == 22 || diff == 23 ) continue;
                
                // if we're looking at a checker on the bar and there's two there, or if we're looking
                // at a checker away from the bar and there's a checker on it, we can only use direct hits
                // (doubles treated a bit separately).
                
                if( ( j == -1 and otherHit > 1 ) or ( j > -1 and otherHit > 0 ) )
                    checkIndirect = false;
                else
                    checkIndirect = true;
                
                if( j == -1 )
                    nOtherChecker = otherHit;
                else
                    nOtherChecker = otherCheckers.at(j);
                
                if( nOtherChecker > 0 )
                {
                    // if the diff is 1-6, we add all rolls that include that die. For comparison
                    // purposes we always put the smaller die first in the roll object.
                    
                    if( diff < 7 )
                    {
                        for( k=1; k<=diff; k++ ) shots[ (*shotIndexes)[k-1][diff-k] ] = true;
                        for( k=diff+1; k<7; k++ ) shots[ (*shotIndexes)[diff-1][k-diff] ] = true;
                    }
                    
                    // for each possible roll that could hit it indirectly, check whether it's valid
                    
                    for( it=hittingRolls->at(diff-1).begin(); it!=hittingRolls->at(diff-1).end(); it++ )
                    {
                        if( not checkIndirect and it->die1 != it->die2 ) continue;
                        if( otherHit == 1 and it->die1 == it->die2 and diff == 4 * it->die1 ) continue;
                        
                        // if both intermediate slots are covered by the player, the opponent can't
                        // use the roll
                        
                        if( checkers[i-it->die1] > 1 and checkers[i-it->die2] > 1 ) continue;
                        
                        // otherwise it's a possibility
                        
                        shots[ (*shotIndexes)[it->die1-1][it->die2-it->die1] ] = true;
                    }
                }
            }
        }
    }
    
    // sum up all the hitting shots and weight appropriately
    
    double prob=0;
    
    for( i=0; i<21; i++ )
    {
        if( shots[i] )
        {
            if( i == 0 or i == 6 or i == 11 or i == 15 or i == 18 or i == 20 )
                prob += 1; // double
            else
                prob += 2; // mixed roll
        }
    }
    
    return prob/36.;
}

int primesCount( const board& brd, bool forPlayer )
{
    vector<int> checkers;
    if( forPlayer )
        checkers = brd.checkers();
    else
    {
        checkers = brd.otherCheckers();
        reverse( checkers.begin(), checkers.end() );
    }
    
    int maxPrimes=0;
    int primes=0;
    
    for( int i=0; i<12; i++ )
    {
        if( checkers.at(i) > 1 )
        {
            primes += 1;
            if( primes > maxPrimes ) maxPrimes = primes;
        }
        else
            primes = 0;
    }
    
    return maxPrimes;
}

double sigmoid( double x )
{
    //return 1./(1+exp(-x));
    
    if( x > 10 ) return 1;
    if( x < -10 ) return 0;
    
    double arg=1-x/64.;
    arg *= arg;
    arg *= arg;
    arg *= arg;
    arg *= arg;
    arg *= arg;
    arg *= arg;
    return 1./(1+arg);
}

string moveDiff( const board& startBoard, const board& endBoard )
{
    vector<int> fromPoints;
    vector<int> toPoints;
    
    int startNum, endNum, i, j, k, count;
    bool foundOne;
    
    for( i=24; i>=0; i-- )
    {
        if( i == 24 )
        {
            startNum = startBoard.hit();
            endNum   = endBoard.hit();
        }
        else
        {
            startNum = startBoard.checker(i);
            endNum   = endBoard.checker(i);
        }
        int nextInd=i-1;
        while( startNum > endNum )
        {
            // find the first place the checker count went up - make sure 
            // we don't re-use a slot
            
            foundOne = false;
            
            for( j=nextInd; j>=0; j-- )
                if( startBoard.checker(j) < endBoard.checker(j) )
                {
                    // count the number we've already added here - if it's >= diff then
                    // we can't use this slot
                    
                    count=0;
                    for( k=0; k<toPoints.size(); k++ )
                        if( toPoints.at(k) == j ) count++;
                    if( count >= endBoard.checker(j) - startBoard.checker(j) ) continue;
                    
                    foundOne = true;
                    fromPoints.push_back(i);
                    toPoints.push_back(j);
                    if( count + 1 == endBoard.checker(j) - startBoard.checker(j) ) nextInd = j-1;
                    break;
                }
            if( !foundOne )
            {
                if( startBoard.bornIn() < endBoard.bornIn() )
                {
                    foundOne = true;
                    fromPoints.push_back(i);
                    toPoints.push_back(-1);
                }
                else
                    throw string( "Shouldn't be possible" );
            }
            
            startNum -= 1;
        }
    }
    
    // now deal with hitting player blots
    
    int hitDiff = endBoard.otherHit() - startBoard.otherHit();
    int startIndex = 0;
    
    while( hitDiff > 0 )
    {
        // figure out where the blots came from and potentially adjust the moves to hit them
        
        for( i=startIndex; i<24; i++ )
            if( startBoard.otherChecker(i) > endBoard.otherChecker(i) )
            {
                // see if any checker move landed on this spot; if so, we're good
                
                foundOne = false;
                
                for( j=0; j<toPoints.size(); j++ )
                    if( toPoints.at(j) == i )
                    {
                        foundOne = true;
                        break;
                    }
                if( foundOne ) 
                {
                    hitDiff --;
                    startIndex = i+1;
                    break;
                }
                
                // otherwise we need to split a move to land on the opponent blot
                
                foundOne = false;
                
                for( j=0; j<toPoints.size(); j++ )
                {
                    if( fromPoints.at(j) > i and toPoints.at(j) < i )
                    {
                        fromPoints.push_back(fromPoints.at(j));
                        toPoints.push_back(i);
                        fromPoints.push_back(i);
                        toPoints.push_back(toPoints.at(j));
                        foundOne = true;
                        break;
                    }
                }
                
                if( !foundOne ) throw string( "Shouldn't be possible" );
                
                // delete the move we split
                
                fromPoints.erase( fromPoints.begin() + j );
                toPoints.erase( toPoints.begin() + j );
                hitDiff --;
                startIndex = i+1;
                break;
            }
    }
    
    // now construct the string
    
    stringstream ss;
    for( i=0; i<fromPoints.size(); i++ )
    {
        if( fromPoints.at(i) == 24 )
            ss << "b";
        else
            ss << fromPoints.at(i)+1;
        ss << "/";
        if( toPoints.at(i) == -1 )
            ss << "o";
        else
            ss << toPoints.at(i)+1;
        if( i < fromPoints.size() - 1 )
            ss << " ";
    }
    return ss.str();
}