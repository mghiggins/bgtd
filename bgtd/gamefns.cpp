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
#include "bearofffns.h"
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

gameProbabilities rolloutBoardProbabilitiesVarReduction( const board& brd, strategyprob& strat, int nRuns, int seed, int depth )
{
    CRandomMersenne rng(seed);
    
    return rolloutBoardProbabilitiesVarReduction( brd, strat, nRuns, &rng, depth );
}

gameProbabilities rolloutBoardProbabilitiesVarReduction( const board& brd, strategyprob& strat, int nRuns, CRandomMersenne * rng, int depth )
{
    gameProbabilities avgVal, rollProbs, stratProbs;
    board b, bluck, bnext;
    int die1, die2, die1luck, die2luck;
    double weight;
    
    for( int i=0; i<nRuns; i++ )
    {
        // start off with board = the board they passed in and on the opponent's roll
        
        int turn=1-brd.perspective();
        
        b = brd;
        while( true )
        {
            // set the board to the appropriate perspective
            
            b.setPerspective(turn);
            
            // roll the dice
            
            die1 = rng->IRandom(1,6);
            die2 = rng->IRandom(1,6);
            
            // calculate the optimal moves for every dice roll, and the average value for
            // each of the probabilities. Also remember the move for the dice roll they got.
            
            gameProbabilities avgProbs; // will hold game probabilities after the roll, averaged over all rolls
            
            for( die1luck=1; die1luck<=6; die1luck++ )
                for( die2luck=1; die2luck<=die1luck; die2luck++ )
                {
                    set<board> moves( possibleMoves(b, die1luck, die2luck) );
                    bluck = strat.preferredBoard(b, moves);
                    stratProbs = strat.boardProbabilities(bluck);
                    if( die1luck==die2luck )
                        weight = 1./36;
                    else
                        weight = 1./18;
                    avgProbs = avgProbs + stratProbs * weight;
                    if( ( die1luck==die1 and die2luck==die2 ) or ( die1luck==die2 and die2luck==die1 ) )
                    {
                        bnext = bluck;
                        rollProbs = stratProbs;
                    }
                }
            
            // luck (separately for each probability) is the accumulated luck of the rolls - include
            // that in the total
            
            if( turn == brd.perspective() )
                avgVal = avgVal - ( rollProbs - avgProbs );
            else
                avgVal = avgVal - ( rollProbs.flippedProbs() - avgProbs.flippedProbs() );
            
            // if the game's over, break
            
            b = bnext;
            if( b.bornIn() == 15 or b.otherBornIn() == 15 ) break;
            turn = 1 - turn;
        }
        
        // get the final probabilities
        
        b.setPerspective(brd.perspective());
        avgVal = avgVal + strat.boardProbabilities(b);
    }
    
    // rescale by the number of runs and return
    
    avgVal = avgVal / nRuns;
    return avgVal;
}

gameProbabilities rolloutBoardProbabilities( const board& brd, strategyprob& strat, int nRuns, int seed, int depth )
{
    CRandomMersenne rng(seed);
    return rolloutBoardProbabilities( brd, strat, nRuns, &rng, depth );
}

gameProbabilities rolloutBoardProbabilities( const board& brd, strategyprob& strat, int nRuns, CRandomMersenne * rng, int depth )
{
    gameProbabilities avgVal;
    board startBoard; // the starting board
    
    for( int i=0; i<nRuns; i++ )
    {
        // start a new game and run it to completion
        
        game g( &strat, &strat, rng );
        g.setBoard( brd );
        g.nSteps = ( brd == startBoard ) ? 0 : 1; // nSteps=0 means the first roll can't be a double etc
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
    workerRollout( const board& brd, strategyprob& strat, int nRuns, int seed, int index, bool varReduc, int depth ) : brd(brd), strat(strat), nRuns(nRuns), seed(seed), index(index), varReduc(varReduc), depth(depth) {};
    
    void operator()()
    {
        CRandomMersenne rng(seed);
        gameProbabilities val;
        if( varReduc )
            val = rolloutBoardProbabilitiesVarReduction( brd, strat, nRuns, &rng, depth );
        else
            val = rolloutBoardProbabilities( brd, strat, nRuns, &rng, depth );
        valsRollout.at(index) = val;
    }
    
private:
    const board& brd;
    strategyprob& strat;
    int nRuns;
    int seed;
    int index;
    bool varReduc;
    int depth;
};

gameProbabilities rolloutBoardProbabilitiesParallel( const board& brd, strategyprob& strat, int nRuns, int seed, int nThreads, bool varReduc, int depth )
{
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
        ts.create_thread( workerRollout( brd, strat, runsPerThread, seed+i*runsPerThread, i, varReduc, depth ) );
    
    // the last bucket contains the rest
    
    int lastNRuns = nRuns - runsPerThread * ( nThreads - 1 );
    bool useLast=false;
    if( lastNRuns > 0 )
    {
        ts.create_thread( workerRollout( brd, strat, nRuns - runsPerThread * ( nThreads - 1 ), seed+(nThreads-1)*runsPerThread, nThreads-1, varReduc, depth ) );
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

double hittingProbBar( const board& brd, bool forOpponent )
{
    // if we haven't initialized the hitting rolls list yet, do so now
    
    if( hittingRolls == 0 ) setupHittingRolls();
    
    // find all the blots and note which opponent rolls would hit them
    
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
    
    int i, j;
    set<roll>::iterator it;

    for( i=0; i<6; i++ )
    {
        if( checkers[i] == 1 ) // a blot that could be hit
        {
            // add in any roll with a die roll for this slot for a direct ht
            
            for( j=1; j<=i+1; j++ ) shots[ (*shotIndexes)[j-1][i+1-j] ] = true;
            for( j=i+1; j<7; j++ ) shots[ (*shotIndexes)[i+1-1][j-i-1] ] = true;
            
            // then check indirect hits
            
            for( it=hittingRolls->at(i).begin(); it!=hittingRolls->at(i).end(); it++ )
            {
                // if either of the slots in the hitting roll is free, include it
                
                if( checkers[it->die1] < 2 or checkers[it->die2] < 2 )
                    shots[ (*shotIndexes)[it->die1-1][it->die2-it->die1] ] = true;
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

set<roll> doubleHittingShots( const board& brd, bool forOpponent, bool noBlot, int minIndex, int maxIndex )
{
    if( minIndex < 0 or minIndex > 23 ) throw string( "minIndex must be in [0,23]" );
    if( maxIndex < 0 or maxIndex > 23 ) throw string( "maxIndex must be in [0,23]" );
    
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
    
    set<roll> rolls;
    
    // start by finding where the opponent's 1st- and 2nd-most back checkers are - no player checker can be hit after the 2nd
    
    int back1=-2, back2, i;
    
    if( otherHit >= 2 )
        back1 = back2 = -1;
    else
    {
        if( otherHit > 0 ) back1 = -1;
        
        int count = otherHit;
        for( i=0; i<24; i++ )
            if( otherCheckers[i] > 0 )
            {
                count += otherCheckers[i];
                if( count >= 1 and back1 == -2 )
                    back1 = i;
                if( count >= 2 )
                    break;
            }
        if( i >= 23 ) return rolls; // no possibility of a double hit
        
        back2 = i;
    }
    
    int j, k, startPos;
    
    // run through each player position from back2+1 and find blots - for each, check which rolls could hit it
    
    int iMin=back2+1;
    int iMax=23;
    
    if( iMin < minIndex ) iMin = minIndex;
    if( iMax > maxIndex ) iMax = maxIndex;
    
    for( i=iMin; i<=iMax; i++ )
    {
        if( checkers[i] == 1 )
        {
            // look for an opponent checker before it
            
            // start with non-doubles
            
            startPos = i-6;
            if( startPos < back1 ) startPos = back1;
            
            for( j=startPos; j<i; j++ )
            {
                // if j (and back1) == -1 that means there's someone hit, so definitely a checker on that "point". Otherwise need to check.
                
                if( j==-1 or ( noBlot and ( otherCheckers[j] > 2 or otherCheckers[j] == 1 ) ) or ( !noBlot and otherCheckers[j] > 0 ) )
                {
                    // look at possible values for the other die
                    
                    for( k=1; k<i-j; k++ )
                        if( ( noBlot and ( otherCheckers[i-k] == 1 or otherCheckers[i-k] > 2 ) ) or ( !noBlot and otherCheckers[i-k] > 0 ) )
                            rolls.insert( roll(k,i-j) );
                }
            }
            
            // check all the doubles
            
            for( j=1; j<=6; j++ )
            {
                // direct
                
                if( i-j >= -1 )
                {
                    if( i-j == -1 )
                        k = otherHit;
                    else
                        k = otherCheckers[i-j];
                    if( ( noBlot and ( k > 3 or k == 2 ) ) or ( !noBlot and k>1 ) )
                        rolls.insert(roll(j,j));
                }
                
                // 2-step
                
                if( i-2*j >= -1 )
                {
                    if( checkers[i-j] > 1 ) continue;
                    
                    if( i-2*j == -1 )
                        k = otherHit;
                    else
                        k = otherCheckers[i-2*j];
                    
                    if( ( noBlot and ( k > 3 or k == 2 ) ) or ( !noBlot and k>1 ) )
                        rolls.insert(roll(j,j));
                }
                
                // 3 of one and 1 of the other
                
                if( i-3*j >= -1 )
                {
                    if( checkers[i-j] > 1 or checkers[i-2*j] > 1 ) continue;
                    
                    // make sure there's an appropriate number of checkers on i-j to be the other leg
                    
                    if( not ( ( noBlot and ( otherCheckers[i-j] == 1 or otherCheckers[i-j] > 2 ) ) or ( !noBlot and otherCheckers[i-j] > 0 ) ) ) continue;
                    
                    if( i-3*j == -1 )
                        k = otherHit;
                    else
                        k = otherCheckers[i-3*j];
                    
                    if( ( noBlot and ( k > 2 or k == 1 ) ) or ( !noBlot and k > 0 ) )
                        rolls.insert(roll(j,j));
                }
            }
        }
    }
    
    return rolls;
}

double doubleHittingProb( const board& brd, bool forOpponent, bool noBlot, int minIndex, int maxIndex )
{
    // if we haven't initialized the hitting rolls list yet, do so now
    
    if( hittingRolls == 0 ) setupHittingRolls();
    
    if( minIndex < 0 or minIndex > 23 ) throw string( "minIndex must be in [0,23]" );
    if( maxIndex < 0 or maxIndex > 23 ) throw string( "maxIndex must be in [0,23]" );
    
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
    
    vector<bool> rolls(21,false);
    
    // start by finding where the opponent's 1st- and 2nd-most back checkers are - no player checker can be hit after the 2nd
    
    int back1=-2, back2, i;
    
    if( otherHit >= 2 )
        back1 = back2 = -1;
    else
    {
        if( otherHit > 0 ) back1 = -1;
        
        int count = otherHit;
        for( i=0; i<24; i++ )
            if( otherCheckers[i] > 0 )
            {
                count += otherCheckers[i];
                if( count >= 1 and back1 == -2 )
                    back1 = i;
                if( count >= 2 )
                    break;
            }
        if( i >= 23 ) return 0;
        
        back2 = i;
    }
    
    int j, k, startPos;
    
    // run through each player position from back2+1 and find blots - for each, check which rolls could hit it
    
    int iMin=back2+1;
    int iMax=23;
    
    if( iMin < minIndex ) iMin = minIndex;
    if( iMax > maxIndex ) iMax = maxIndex;
    
    for( i=iMin; i<=iMax; i++ )
    {
        if( checkers[i] == 1 )
        {
            // look for an opponent checker before it
            
            // start with non-doubles
            
            startPos = i-6;
            if( startPos < back1 ) startPos = back1;
            
            for( j=startPos; j<i; j++ )
            {
                // if j (and back1) == -1 that means there's someone hit, so definitely a checker on that "point". Otherwise need to check.
                
                if( j==-1 or ( noBlot and ( otherCheckers[j] > 2 or otherCheckers[j] == 1 ) ) or ( !noBlot and otherCheckers[j] > 0 ) )
                {
                    // look at possible values for the other die
                    
                    for( k=1; k<i-j; k++ )
                        if( ( noBlot and ( otherCheckers[i-k] == 1 or otherCheckers[i-k] > 2 ) ) or ( !noBlot and otherCheckers[i-k] > 0 ) )
                            rolls[ (*shotIndexes)[k-1][i-j-k] ] = true;
                }
            }
            
            // check all the doubles
            
            for( j=1; j<=6; j++ )
            {
                // direct
                
                if( i-j >= -1 )
                {
                    if( i-j == -1 )
                        k = otherHit;
                    else
                        k = otherCheckers[i-j];
                    if( ( noBlot and ( k > 3 or k == 2 ) ) or ( !noBlot and k>1 ) )
                        rolls[ (*shotIndexes)[j-1][0] ] = true;
                }
                
                // 2-step
                
                if( i-2*j >= -1 )
                {
                    if( checkers[i-j] > 1 ) continue;
                    
                    if( i-2*j == -1 )
                        k = otherHit;
                    else
                        k = otherCheckers[i-2*j];
                    
                    if( ( noBlot and ( k > 3 or k == 2 ) ) or ( !noBlot and k>1 ) )
                        rolls[ (*shotIndexes)[j-1][0] ] = true;
                }
                
                // 3 of one and 1 of the other
                
                if( i-3*j >= -1 )
                {
                    if( checkers[i-j] > 1 or checkers[i-2*j] > 1 ) continue;
                    
                    // make sure there's an appropriate number of checkers on i-j to be the other leg
                    
                    if( not ( ( noBlot and ( otherCheckers[i-j] == 1 or otherCheckers[i-j] > 2 ) ) or ( !noBlot and otherCheckers[i-j] > 0 ) ) ) continue;
                    
                    if( i-3*j == -1 )
                        k = otherHit;
                    else
                        k = otherCheckers[i-3*j];
                    
                    if( ( noBlot and ( k > 2 or k == 1 ) ) or ( !noBlot and k > 0 ) )
                        rolls[ (*shotIndexes)[j-1][0] ] = true;
                }
            }
        }
    }
    
    double prob;
    for( i=0; i<21; i++ )
    {
        if( rolls[i] )
        {
            if( i == 0 or i == 6 or i == 11 or i == 15 or i == 18 or i == 20 )
                prob += 1; // double
            else
                prob += 2; // mixed roll
        }
    }
    
    prob /= 36.;
    
    return prob;
}

double barAverageEscape( const board& brd, bool forOpponent )
{
    board useBrd(brd);
    if( forOpponent ) useBrd.setPerspective( 1 - brd.perspective() );
    
    double avgEscape=0;
    for( int i=23; i>=18; i-- )
        if( useBrd.otherChecker(i) < 2 )
            avgEscape += 1./6 * getBlockadeEscapeCount( useBrd, i );
    
    return avgEscape;
}