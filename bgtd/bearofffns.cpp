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
#include <fstream>
#include <string>
#include "bearofffns.h"
#include "gamefns.h"
#include "randomc.h"

hash_map<string,double> * _boardPnts=0;
hash_map<string,stepsDistribution> * _stepsProbs=0;
hash_map<string,stepsDistribution> * _gamStepsProbs=0;
hash_map<int,int> * _escapeRolls=0;

string boardID( const board& b, int nPnts )
{
    string idStr("");
    for( int i=0; i<nPnts; i++ )
        idStr += static_cast<char>( b.checker(i)+65 );
    for( int i=23; i>23-nPnts; i-- )
        idStr += static_cast<char>( b.otherChecker(i)+65 );
    return idStr;
}

board boardFromID( const string& ID )
{
    int nPnts( (int) ID.size() / 2 );
    board b;
    int i;
    for( i=0; i<24; i++ )
    {
        b.setChecker( i, 0 );
        b.setOtherChecker( i, 0 );
    }
    int count1=0, count2=0;
    int n;
    for( i=0; i<nPnts; i++ )
    {
        n = static_cast<int>( ID.at(i) ) - 65;
        count1 += n;
        if( n>0 ) b.setChecker( i, n );
        n = static_cast<int>( ID.at(i+nPnts) ) - 65;
        count2 += n;
        if( n>0 ) b.setOtherChecker( 23-i, n );
    }
    
    b.setBornIn(15-count1);
    b.setOtherBornIn(15-count2);
    
    return b;
}

hash_map<string,double> * boardPnts()
{
    return _boardPnts;
}

double getBoardPnt( const board& b, int nPnts )
{
    // initialize the local list if it hasn't been already
    
    if( _boardPnts == 0 )
        _boardPnts = new hash_map<string,double>;
    
    // if we've already calculated this, return it
    
    string id = boardID( b, nPnts );
    hash_map<string,double>::iterator it = _boardPnts->find( id );
    if( it != _boardPnts->end() ) return (*it).second;
    
    // otherwise calculate it
    
    // run through all the possible rolls
    
    double expPnts=0, weight;
    int i, j;
    
    for( i=1; i<7; i++ )
        for( j=1; j<=i; j++ )
        {
            // get the appropriate probability weight for this roll - twice as big for unmatched dice
            
            if( i == j )
                weight = 1./36;
            else
                weight = 1./18;
            
            // get the possible moves
            
            set<board> moves = possibleMoves( b, i, j );
            
            // figure out the expected points after each move and choose the one with the most positive
            
            double maxPnt=-100, pnt;
            for( set<board>::iterator k=moves.begin(); k!=moves.end(); k++ )
            {
                // if all pieces are off, this player wins; record that as a win or a gammon as appropriate.
                // Otherwise go to the lookup table to get the value.
                
                if( (*k).bornIn() == 15 )
                {
                    if( (*k).otherBornIn() == 0 )
                        pnt = 2.;
                    else
                        pnt = 1.;
                }
                else
                {
                    board newBoard( (*k) );
                    newBoard.setPerspective( 1 - b.perspective() );
                    
                    pnt = -1 * getBoardPnt( newBoard, nPnts );
                }
                
                if( pnt > maxPnt )
                    maxPnt = pnt;
            }
            
            expPnts += weight * maxPnt;
        }
    
    (*_boardPnts)[ id ] = expPnts;
    return expPnts;
}

void constructBearoff( int nPnts, int nCheckers )
{
    // just call getBoardPnt for possible nCheckers board layouts
    
    int nChecker1, nChecker2, i, j;
    double ppg;
    vector<int> checkers1(nCheckers);
    vector<int> checkers2(nCheckers);
    
    for( nChecker1=1; nChecker1<=nCheckers; nChecker1++ )
        for( nChecker2=1; nChecker2<=nCheckers; nChecker2++ )
        {
            // run through possible checker layouts on both sides
            
            // cycle through positions of player 1 first
            
            for( i=0; i<nCheckers; i++ ) checkers1.at(i) = 0;
            
            while(true)
            {
                // then through positions of player 2
                
                for( i=0; i<nCheckers; i++ ) checkers2.at(i) = 0;
                
                while( true )
                {
                    // construct a board with the appropriate checker locations
                    
                    board b;
                    for( i=0; i<24; i++ )
                    {
                        b.setChecker( i, 0 );
                        b.setOtherChecker( i, 0 );
                    }
                    for( i=0; i<nChecker1; i++ ) b.setChecker( checkers1.at(i), b.checker( checkers1.at(i) ) + 1 );
                    for( i=0; i<nChecker2; i++ ) b.setOtherChecker( 23-checkers2.at(i), b.otherChecker( 23-checkers2.at(i) ) + 1 );
                    b.setBornIn(15-nChecker1);
                    b.setOtherBornIn(15-nChecker2);
                    
                    // get the expected # of points - do nothing with it - just caches the value in the hash map
                    
                    ppg = getBoardPnt( b, nPnts );
                    
                    // increment the checker positions
                    
                    i=nChecker2-1;
                    while( true )
                    {
                        checkers2.at(i) += 1;
                        if( checkers2.at(i) == nPnts )
                        {
                            if( i == 0 )
                            {
                                i = -1;
                                break;
                            }
                            else
                            {
                                if( checkers2.at(i-1) < nPnts-1 ) 
                                {
                                    for( j=i; j<nChecker2; j++ ) checkers2.at(j) = checkers2.at(i-1)+1;
                                }
                                else
                                {
                                    checkers2.at(i) = nPnts - 1;
                                }
                            }
                        }
                        else
                            break;
                        i -= 1;
                    }
                    
                    if( i == -1 ) break;
                }
                
                // increment the checker positions
                
                i=nChecker1-1;
                while( true )
                {
                    checkers1.at(i) += 1;
                    if( checkers1.at(i) == nPnts )
                    {
                        if( i == 0 )
                        {
                            i = -1;
                            break;
                        }
                        else
                        {
                            if( checkers1.at(i-1) < nPnts-1 ) 
                            {
                                for( j=i; j<nChecker1; j++ ) checkers1.at(j) = checkers1.at(i-1)+1;
                            }
                            else
                            {
                                checkers1.at(i) = nPnts - 1;
                            }
                        }
                    }
                    else
                        break;
                    i -= 1;
                }
                
                if( i == -1 ) break;
            }
            
        }
}

void writeBearoffDb( const string& fileName )
{
    if( !_boardPnts ) return;
    ofstream f( fileName.c_str() );
    for( hash_map<string,double>::iterator it=_boardPnts->begin(); it!=_boardPnts->end(); it++ )
        f << (*it).first << "," << (*it).second << ",";
    
    f.close();
}

void loadBearoffDb( const string& fileName )
{
    if( !_boardPnts ) _boardPnts = new hash_map<string,double>;
    
    ifstream f( fileName.c_str() );
    string bit, id;
    double val;
    
    while( true )
    {
        getline( f, id, ',' );
        if( id == "" ) break;
        getline( f, bit, ',' );
        if( bit == "" ) break;
        val = atof( bit.c_str() );
        
        (*_boardPnts)[ id ] = val;
    }
}

stepsDistribution::stepsDistribution( const stepsDistribution& otherDist )
{
    stepProbs = otherDist.stepProbs;
}

vector<stepProbability> stepsDistribution::pairs() const
{
    vector<stepProbability> pairs;
    pairs.resize( stepProbs.size() );
    int i=0;
    for( hash_map<int,double>::const_iterator it=stepProbs.begin(); it!=stepProbs.end(); it++ )
    {
        stepProbability s( it->first, it->second );
        pairs.at(i) = s;
        i += 1;
    }
    
    sort( pairs.begin(), pairs.end(), stepProbabilityCompare );
    return pairs;
}

bool stepProbabilityCompare( const stepProbability& s1, const stepProbability& s2 ) { return s1.nStep < s2.nStep; }

vector<stepProbability> stepsDistribution::pairsProbOrdered() const
{
    vector<stepProbability> pairs;
    pairs.resize( stepProbs.size() );
    int i=0;
    for( hash_map<int,double>::const_iterator it=stepProbs.begin(); it!=stepProbs.end(); it++ )
    {
        stepProbability s( it->first, it->second );
        pairs.at(i) = s;
        i += 1;
    }
    
    sort( pairs.begin(), pairs.end(), stepProbabilityCompareProbOrdered );
    return pairs;
}

bool stepProbabilityCompareProbOrdered( const stepProbability& s1, const stepProbability& s2 ) { return s1.prob > s2.prob; }

string boardIDOneSided( const board& b, int nPnts )
{
    string idStr("");
    for( int i=0; i<nPnts; i++ )
        idStr += static_cast<char>( b.checker(i)+65 );
    return idStr;
}

hash_map<string,stepsDistribution> * stepsProbs()
{
    return _stepsProbs;
}

stepsDistribution getStepsDistribution( const board& b, int nPnts, int maxElem, hash_map<string,stepsDistribution> * cache )
{
    // initialize the local list if it hasn't been already
    
    if( _stepsProbs == 0 )
        _stepsProbs = new hash_map<string,stepsDistribution>;
    
    // if the cache passed in is a null pointer, use the local cache
    
    if( cache == 0 )
        cache = _stepsProbs;
    
    // if we've already calculated this, return it. Check the global cache and also
    // the local cache if that's different.
    
    string id = boardIDOneSided( b, nPnts );
    hash_map<string,stepsDistribution>::iterator it = _stepsProbs->find( id );
    if( it != _stepsProbs->end() ) return (*it).second;
    if( cache != _stepsProbs )
    {
        hash_map<string,stepsDistribution>::iterator it = cache->find( id );
        if( it != cache->end() ) return (*it).second;
    }
    
    // otherwise calculate it
    
    // run through all the possible rolls. The optimal move should really take both
    // sides of the board into account; as an approximation though we choose the move
    // that gives the lowest expected number of steps.
    
    double weight;
    stepsDistribution dist;
    int i, j;
    
    for( i=1; i<7; i++ )
        for( j=1; j<=i; j++ )
        {
            // get the appropriate probability weight for this roll - twice as big for unmatched dice
            
            if( i == j )
                weight = 1./36;
            else
                weight = 1./18;
            
            // get the possible moves
            
            set<board> moves = possibleMoves( b, i, j );
            
            // figure out the expected number of steps after each move and choose the minimal one
            
            double minSteps=1000, steps;
            board minBoard;
            stepsDistribution minDist, moveDist;
            for( set<board>::iterator k=moves.begin(); k!=moves.end(); k++ )
            {
                // if all pieces are off, this player wins; record that as number of steps == 0.
                // Otherwise go to the lookup table to get the value.
                
                if( (*k).bornIn() == 15 )
                {
                    steps = 1; // zero, plus one step to get there
                    moveDist.stepProbs.clear();
                    moveDist.stepProbs[0] = 1; // probability of zero steps to finish is 100%
                }
                else
                {
                    board newBoard( (*k) );
                    moveDist = getStepsDistribution( newBoard, nPnts, maxElem, cache );
                    steps = 0;
                    for( hash_map<int,double>::iterator it=moveDist.stepProbs.begin(); it!=moveDist.stepProbs.end(); it++ )
                        steps += it->first * it->second;
                    steps += 1; // for the extra step to get to this board
                }
                
                if( steps < minSteps )
                {
                    minSteps = steps;
                    minBoard = (*k);
                    minDist  = moveDist;
                }
            }
            
            // add the weighted steps distribution from the (approximate) optimal move to the current distribution
            
            int nSteps;
            double prob;
            
            for( hash_map<int,double>::iterator it=minDist.stepProbs.begin(); it!=minDist.stepProbs.end(); it++ )
            {
                nSteps = it->first+1; // need to add one step to get to the next position
                prob   = it->second;
                
                if( dist.stepProbs.find(nSteps) == dist.stepProbs.end() )
                    dist.stepProbs[nSteps] = prob * weight;
                else
                    dist.stepProbs[nSteps] += prob * weight;
            }
        }
    
    // if the # of elements > maxElem, trim out the lowest-probability elements and renormalize the rest
    
    if( maxElem > 0 and dist.stepProbs.size() > maxElem )
    {
        vector<stepProbability> pairs( dist.pairsProbOrdered() );
        hash_map<int,double> newDist;
        double totProb=0;
        for( int i=0; i<maxElem; i++ )
            totProb += pairs.at(i).prob;
        for( int i=0; i<maxElem; i++ )
            newDist[pairs.at(i).nStep] = pairs.at(i).prob / totProb; // renormalize so all remaining probs sum to 1
        dist.stepProbs = newDist;
    }
    
    (*cache)[ id ] = dist;
    return dist;
}

double getFinishProb( const vector<stepProbability>& pairs1, const vector<stepProbability>& pairs2, bool move1 )
{
    // returns the probability that 1 finishes before 2, assuming pairs1 is the distribution of number
    // of steps for 1 and pairs2 is the same for 2. move1 is true if player1 is on move, false if player 2 is
    // on move.
    
    double probFinish=0;
    int i, j, nStep2;
    double prob2;
    
    for( i=0; i<pairs2.size(); i++ )
    {
        nStep2 = pairs2.at(i).nStep;
        prob2  = pairs2.at(i).prob;
        for( j=0; j<pairs1.size(); j++ )
        {
            if( ( move1 and pairs1.at(j).nStep <= nStep2 ) or ( !move1 and pairs1.at(j).nStep < nStep2 ) )
                probFinish += prob2 * pairs1.at(j).prob;
            else
                break;
        }
    }
    
    return probFinish;
}

double getProbabilityWin( const board& b, int nPnts )
{
    vector<stepProbability> pairs1( getStepsDistribution( b, nPnts ).pairs() );
    board flippedBoard(b);
    flippedBoard.setPerspective( 1 - b.perspective() );
    vector<stepProbability> pairs2( getStepsDistribution( flippedBoard, nPnts ).pairs() );
    return getFinishProb( pairs1, pairs2, true );
}

double getProbabilityGammonWin( const board& b, int nPnts )
{
    board flippedBoard(b);
    flippedBoard.setPerspective( 1 - b.perspective() );
    vector<stepProbability> gamPairs2( getGammonStepsDistribution( flippedBoard, nPnts ).pairs() );
    if( gamPairs2.size() == 0 ) return 0;
    vector<stepProbability> pairs1( getStepsDistribution( b, nPnts ).pairs() );
    return getFinishProb( pairs1, gamPairs2, true );
}

double getProbabilityGammonLoss( const board& b, int nPnts )
{
    vector<stepProbability> gamPairs1( getGammonStepsDistribution( b, nPnts ).pairs() );
    if( gamPairs1.size() == 0 ) return 0;
    board flippedBoard(b);
    flippedBoard.setPerspective( 1 - b.perspective() );
    vector<stepProbability> pairs2( getStepsDistribution( flippedBoard, nPnts ).pairs() );
    return getFinishProb( pairs2, gamPairs1, false );
}

double getBoardPntOS( const board& b, int nPnts )
{
    double probWin     = getProbabilityWin( b, nPnts );
    double probGamWin  = getProbabilityGammonWin( b, nPnts );
    double probGamLoss = getProbabilityGammonLoss( b, nPnts );
    
    // calculate the equity in the usual way
    
    return 1 * ( probWin - probGamWin ) + 2 * probGamWin - 1 * ( 1 - probWin - probGamLoss ) - 2 * probGamLoss;
}

void constructBearoffOneSided( int nPnts, int nCheckers, int maxElem )
{
    // just call getStepsDistribution for possible nCheckers board layouts
    
    int nChecker, i, j;
    stepsDistribution dist;
    vector<int> checkers(nCheckers); // checkers[i] represents the point that the i'th checker is sitting on
    
    long count=0;
    long lastSize=0;
    
    //for( nChecker=1; nChecker<=nCheckers; nChecker++ )
    for( nChecker=nCheckers; nChecker>0; nChecker-- )
    {
        cout << "Working on " << nChecker << " checkers on board\n";
        
        // run through possible checker layouts on the one side
        
        for( i=0; i<nCheckers; i++ ) checkers.at(i) = 0;
        
        while( true )
        {
            // construct a board with the appropriate checker locations
            
            board b;
            for( i=0; i<24; i++ )
            {
                b.setChecker( i, 0 );
                b.setOtherChecker( i, 0 ); // doesn't get used but done to make a consistent board
            }
            for( i=0; i<nChecker; i++ ) b.setChecker( checkers.at(i), b.checker( checkers.at(i) ) + 1 );
            b.setBornIn(15-nChecker);
            b.setOtherBornIn(15); // doesn't get used but done to make a consistent board
            
            // get the distribution of steps - do nothing with it - just caches the value in the hash map
            
            dist = getStepsDistribution( b, nPnts, maxElem );
            count ++;
            if( count % 500 == 0 ) 
            {
                long hashSize = _stepsProbs->size();
                if( hashSize > lastSize + 100 )
                {
                    cout << "    stepsProbs new size is " << hashSize << endl;
                    lastSize = hashSize;
                }
            }
            
            // increment the checker positions
            
            i=nChecker-1;
            while( true )
            {
                checkers.at(i) += 1;
                if( checkers.at(i) == nPnts )
                {
                    if( i == 0 )
                    {
                        i = -1;
                        break;
                    }
                    else
                    {
                        if( checkers.at(i-1) < nPnts-1 ) 
                        {
                            for( j=i; j<nChecker; j++ ) checkers.at(j) = checkers.at(i-1)+1;
                        }
                        else
                        {
                            checkers.at(i) = nPnts - 1;
                        }
                    }
                }
                else
                    break;
                i -= 1;
            }
            
            if( i == -1 ) break;
        }
    }
}

struct calcElement
{
    int nChecker;
    vector<int> checkers;
};

void doParallelCalcs( const vector< vector<calcElement> >& calcs, int nPnts, int maxElem );

vector< hash_map<string,stepsDistribution> > caches;

class workerBearoff
{
public:
    workerBearoff( int index, const vector<calcElement>& calcs, int nPnts, int maxElem ) : index( index ), calcs( calcs ), nPnts( nPnts ), maxElem( maxElem ) {}
    
    void operator()()
    {
        // for each calc we're doing, construct a board and calculate the distribution, saving it to
        // the appropriate temporary cache. These are done in serial of course.
        
        stepsDistribution dist;
        hash_map<string,stepsDistribution> * cache = &caches.at(index);
        
        for( vector<calcElement>::const_iterator it=calcs.begin(); it!=calcs.end(); it++ )
        {
            int nChecker = it->nChecker;
            vector<int> checkers = it->checkers;
            
            board b;
            int i;
            for( i=0; i<24; i++ )
            {
                b.setChecker( i, 0 );
                b.setOtherChecker( i, 0 ); // doesn't get used but done to make a consistent board
            }
            for( i=0; i<nChecker; i++ ) b.setChecker( checkers.at(i), b.checker( checkers.at(i) ) + 1 );
            b.setBornIn(15-nChecker);
            b.setOtherBornIn(15); // doesn't get used but done to make a consistent board
            
            // get the distribution of steps - do nothing with it - just caches the value in the hash map
            
            dist = getStepsDistribution( b, nPnts, maxElem, cache );
        }
    }
    
private:
    int index;
    const vector<calcElement>& calcs;
    int nPnts;
    int maxElem;
};

void doParallelCalcs( const vector< vector<calcElement> >& calcs, int nPnts, int maxElem )
{
    using namespace boost;
    
    // reset the caches for the elements we'll calculate
    
    caches.resize( calcs.size() );
    int i;
    for( i=0; i<calcs.size(); i++ ) caches.at(i).clear();
    
    thread_group ts;
    for( i=0; i<calcs.size(); i++ ) ts.create_thread( workerBearoff( i, calcs.at(i), nPnts, maxElem ) );
    ts.join_all();
    
    // copy all the calculated boards to the local cache, so that the next set of threads can
    // access the cached results from this set.
    
    for( i=0; i<calcs.size(); i++ )
    {
        cout << "   Bucket " << i << ": " << caches.at(i).size() << endl;
        for( hash_map<string,stepsDistribution>::iterator it=caches.at(i).begin(); it!=caches.at(i).end(); it++ )
            (*_stepsProbs)[ it->first ] = it->second;
        cout << "      stepsProbs new size is " << _stepsProbs->size() << endl;
    }
}

void constructBearoffOneSidedParallel( int nPnts, int nCheckers, int maxElem, int nThreads, int calcsPerThread )
{
    // initialize the local list if it hasn't been already - that way we never write
    // to it in the thread calcs, just read.
    
    if( _stepsProbs == 0 )
        _stepsProbs = new hash_map<string,stepsDistribution>;
    
    // just call getStepsDistribution for possible nCheckers board layouts
    
    int nChecker, i, index;
    stepsDistribution dist;
    vector<int> checkers(nCheckers); // checkers[i] represents the point that the i'th checker is sitting on
    
    vector< vector<calcElement> > calcs;
    calcs.resize( nThreads );
    
    // start with a random selection of boards - gives a broad spread through parameter space so parallel tasks
    // tend not to depend on the same lower-level boards.
    
    CRandomMersenne rng(1); // random # generator
    
    long nRuns=nElementsOS( nPnts, nCheckers );
    long run;
    int threadIndex=0;
    bool doneCalcs;
    
    for( run=0; run<nRuns; run++ )
    {
        nChecker = rng.IRandom( 1, nCheckers );
        for( i=0; i<nChecker; i++ )
        {
            index = rng.IRandom( 0, nPnts );
            checkers.at(i) = index;
        }
        calcElement elem;
        elem.nChecker = nChecker;
        elem.checkers = checkers;
        calcs.at(threadIndex).push_back(elem);
        doneCalcs = false;
        if( calcs.at(threadIndex).size() == calcsPerThread )
        {
            threadIndex++;
            if( threadIndex == nThreads )
            {
                doParallelCalcs( calcs, nPnts, maxElem );
                for( i=0; i<nThreads; i++ ) calcs.at(i).resize(0);
                threadIndex = 0;
                doneCalcs = true;
            }
        }
    }
    
    // if there are any more calcs to do, do them now
    
    if( !doneCalcs )
        doParallelCalcs( calcs, nPnts, maxElem );
    
    // do the serial calc to fill in any holes that we missed
    
    constructBearoffOneSided( nPnts, nCheckers, maxElem );
}

void writeBearoffDbOneSided( const string& fileName )
{
    if( !_stepsProbs ) return;
    ofstream f( fileName.c_str() );
    for( hash_map<string,stepsDistribution>::iterator it=_stepsProbs->begin(); it!=_stepsProbs->end(); it++ )
    {
        f << it->first << ",";
        for( hash_map<int,double>::iterator it2=it->second.stepProbs.begin(); it2!=it->second.stepProbs.end(); it2++ )
            f << it2->first << "," << it2->second << ",";
        f << "!,";
    }
    
    f.close();
}

void loadBearoffDbOneSided( const string& fileName )
{
    if( !_stepsProbs ) 
        _stepsProbs = new hash_map<string,stepsDistribution>;
    else
        _stepsProbs->clear();
    
    ifstream f( fileName.c_str() );
    string bit, id;
    double nStep, prob;
    
    while( true )
    {
        getline( f, id, ',' );
        if( id == "" ) break;
        stepsDistribution dist;
        while( true )
        {
            getline( f, bit, ',' );
            if( bit == "!" ) break;
            nStep = atoi( bit.c_str() );
            getline( f, bit, ',' );
            prob = atof( bit.c_str() );
            dist.stepProbs[nStep] = prob;
        }
        
        (*_stepsProbs)[ id ] = dist;
    }
}

hash_map<string,stepsDistribution> * gamStepsProbs()
{
    return _gamStepsProbs;
}

stepsDistribution getGammonStepsDistribution( const board& b, int nPnts, int maxElem )
{
    // initialize the local list if it hasn't been already
    
    if( _gamStepsProbs == 0 )
        _gamStepsProbs = new hash_map<string,stepsDistribution>;
    
    // if the player has already taken in a piece, return a trivial distribution
    
    if( b.bornIn() > 0 ) return stepsDistribution();
    
    // if we've already calculated this, return it
    
    string id = boardIDOneSided( b, nPnts );
    hash_map<string,stepsDistribution>::iterator it = _gamStepsProbs->find( id );
    if( it != _gamStepsProbs->end() ) return (*it).second;
    
    //cout << "Calculating gammon dist for board\n";
    //b.print();
    //cout << endl;
    
    // otherwise calculate it
    
    // run through all the possible rolls. The optimal move should really take both
    // sides of the board into account; as an approximation though we choose the move
    // that gives the lowest expected number of steps to take off the first piece.
    // That's even more of an approximation that for the regular bearoff db; but in
    // the case where the prob of gammon is nontrivial, the best strategy probably is
    // to take off the first piece as quickly as possible.
    
    double weight;
    stepsDistribution dist;
    int i, j, l;
    bool foundDiff;
    
    // check if there are any pieces outside the home board - if so, later we'll
    // trim down possible moves to the ones that move those pieces in
    
    bool hasPieceOutside=false;
    for( i=6; i<nPnts; i++ )
        if( b.checker(i) > 0 )
        {
            hasPieceOutside = true;
            break;
        }
    
    for( i=1; i<7; i++ )
        for( j=1; j<=i; j++ )
        {
            // get the appropriate probability weight for this roll - twice as big for unmatched dice
            
            if( i == j )
                weight = 1./36;
            else
                weight = 1./18;
            
            // get the possible moves
            
            set<board> moves = possibleMoves( b, i, j );
            
            // figure out the expected number of steps to first checker off after each move and choose the minimal one
            
            double minSteps=1000, steps;
            board minBoard;
            stepsDistribution minDist, moveDist;
            for( set<board>::iterator k=moves.begin(); k!=moves.end(); k++ )
            {
                board newBoard((*k));
                
                if( hasPieceOutside )
                {
                    // if we start with a piece outside the home board, we only care about moves where we move at 
                    // least one of the pieces outside the home board; so if this move doesn't make that happen, skip it.
                    // This trims down tremendously the number of moves we need to test.
                    
                    foundDiff = false;
                    
                    for( l=6; l<nPnts; l++ )
                    {
                        if( k->checker(l) != b.checker(l) )
                        {
                            foundDiff = true;
                            break;
                        }
                    }
                    
                    if( !foundDiff ) 
                        continue; // move to the next board
                }
                
                // if a single piece is off, the player can't be gammoned; record that as number of steps == 0.
                // Otherwise go to the lookup table to get the value.
                
                if( newBoard.bornIn() > 0 )
                {
                    steps = 1; // zero, plus one step to get there
                    moveDist.stepProbs.clear();
                    moveDist.stepProbs[0] = 1; // probability of zero steps to take off the first checker is 100%
                }
                else
                {
                    //board newBoard( (*k) );
                    moveDist = getGammonStepsDistribution( newBoard, nPnts, maxElem );
                    steps = 0;
                    for( hash_map<int,double>::iterator it=moveDist.stepProbs.begin(); it!=moveDist.stepProbs.end(); it++ )
                        steps += it->first * it->second;
                    steps += 1; // for the extra step to get to this board
                }
                
                if( steps < minSteps )
                {
                    minSteps = steps;
                    minBoard = (*k);
                    minDist  = moveDist;
                }
            }
            
            // add the weighted steps distribution from the (approximate) optimal move to the current distribution
            
            int nSteps;
            double prob;
            
            for( hash_map<int,double>::iterator it=minDist.stepProbs.begin(); it!=minDist.stepProbs.end(); it++ )
            {
                nSteps = it->first+1; // need to add one step to get to the next position
                prob   = it->second;
                
                if( dist.stepProbs.find(nSteps) == dist.stepProbs.end() )
                    dist.stepProbs[nSteps] = prob * weight;
                else
                    dist.stepProbs[nSteps] += prob * weight;
            }
        }
    
    // if the # of elements > maxElem, trim out the lowest-probability elements and renormalize the rest
    
    if( maxElem > 0 and dist.stepProbs.size() > maxElem )
    {
        vector<stepProbability> pairs( dist.pairsProbOrdered() );
        hash_map<int,double> newDist;
        double totProb=0;
        for( int i=0; i<maxElem; i++ )
            totProb += pairs.at(i).prob;
        for( int i=0; i<maxElem; i++ )
            newDist[pairs.at(i).nStep] = pairs.at(i).prob / totProb; // renormalize so all remaining probs sum to 1
        dist.stepProbs = newDist;
    }
    
    (*_gamStepsProbs)[ id ] = dist;
    return dist;
}

void constructGammonBearoffOneSided( int nPnts, int maxElem )
{
    // if we're including less than 7 points, nothing to do
    
    if( nPnts < 7 ) return;
    
    // just call getGammonStepsDistribution for possible board layouts
    
    int nChecker=15, i, j;
    bool calcBoard;
    stepsDistribution dist;
    vector<int> checkers(nChecker); // checkers[i] represents the point that the i'th checker is sitting on
    
    cout << "Working on " << nChecker << " checkers on board\n";
    
    // run through possible checker layouts on the one side. We only include boards where at least one
    // checker is outside the player's home board. That's not really comprehensive, since there are board
    // layouts where all checkers are in the home board and there's still the possibility that it'll take
    // more than one step to take off the first checker. But they'll mostly be included as sub-calcs under
    // the ones we include here, and in any case other cases can be calculated cheaply on the fly if they're
    // missed out here.
    
    for( i=0; i<nChecker; i++ ) checkers.at(i) = 0;
    
    long count=0;
    
    while( true )
    {
        // check if this layout contains a checker outside the home board. If so, do the calc; if not, skip it.
        
        calcBoard = false;
        for( i=0; i<nChecker; i++ )
            if( checkers.at(i) > 5 )
            {
                calcBoard = true;
                break;
            }
        
        if( calcBoard )
        {
            // construct a board with the appropriate checker locations
            
            board b;
            for( i=0; i<24; i++ )
            {
                b.setChecker( i, 0 );
                b.setOtherChecker( i, 0 ); // doesn't get used but done to make a consistent board
            }
            for( i=0; i<nChecker; i++ ) b.setChecker( checkers.at(i), b.checker( checkers.at(i) ) + 1 );
            b.setBornIn(0); // by construction, we haven't borne in any checkers yet
            b.setOtherBornIn(15); // doesn't get used but done to make a consistent board
            
            // get the distribution of steps - do nothing with it - just caches the value in the hash map
            
            dist = getGammonStepsDistribution( b, nPnts, maxElem );
            count++;
            if( count % 1000 == 0 )
            {
                b.print();
                int num=0;
                for( i=0; i<nChecker; i++ )
                    if( checkers.at(i) > 5 ) num++;
                double expSteps=0;
                for( hash_map<int,double>::iterator it=dist.stepProbs.begin(); it!=dist.stepProbs.end(); it++ )
                    expSteps += it->first * it->second;
                cout << "  cache size = " << _gamStepsProbs->size() << "; num = " << num << "; exp steps = " << expSteps << "; size of dist = " << dist.stepProbs.size() << endl;
                cout << endl;
            }
        }
        
        // increment the checker positions
        
        i=nChecker-1;
        while( true )
        {
            checkers.at(i) += 1;
            if( checkers.at(i) == nPnts )
            {
                if( i == 0 )
                {
                    i = -1;
                    break;
                }
                else
                {
                    if( checkers.at(i-1) < nPnts-1 ) 
                    {
                        for( j=i; j<nChecker; j++ ) checkers.at(j) = checkers.at(i-1)+1;
                    }
                    else
                    {
                        checkers.at(i) = nPnts - 1;
                    }
                }
            }
            else
                break;
            i -= 1;
        }
        
        if( i == -1 ) break;
    }
}

void writeGammonBearoffDbOneSided( const string& fileName )
{
    if( !_gamStepsProbs ) return;
    ofstream f( fileName.c_str() );
    for( hash_map<string,stepsDistribution>::iterator it=_gamStepsProbs->begin(); it!=_gamStepsProbs->end(); it++ )
    {
        f << it->first << ",";
        for( hash_map<int,double>::iterator it2=it->second.stepProbs.begin(); it2!=it->second.stepProbs.end(); it2++ )
            f << it2->first << "," << it2->second << ",";
        f << "!,";
    }
    
    f.close();
}

void loadGammonBearoffDbOneSided( const string& fileName )
{
    if( !_gamStepsProbs ) 
        _gamStepsProbs = new hash_map<string,stepsDistribution>;
    else
        _gamStepsProbs->clear();
    
    ifstream f( fileName.c_str() );
    string bit, id;
    double nStep, prob;
    
    while( true )
    {
        getline( f, id, ',' );
        if( id == "" ) break;
        stepsDistribution dist;
        while( true )
        {
            getline( f, bit, ',' );
            if( bit == "!" ) break;
            nStep = atoi( bit.c_str() );
            getline( f, bit, ',' );
            prob = atof( bit.c_str() );
            dist.stepProbs[nStep] = prob;
        }
        
        (*_gamStepsProbs)[ id ] = dist;
    }
}

long factorial(int n)
{
    double prod=1;
    for( int i=1; i<=n; i++ ) prod *= i;
    return prod;
}

long nElementsTS( int nPnts, int nCheckers )
{
    long e = nElementsOS( nPnts, nCheckers );
    return e*e;
}

long nElementsOS( int nPnts, int nCheckers )
{
    int bigger, smaller;
    if( nPnts > nCheckers )
    {
        bigger = nPnts;
        smaller = nCheckers;
    }
    else
    {
        bigger = nCheckers;
        smaller = nPnts;
    }
    long e = 1;
    for( int i=bigger+1; i<=nPnts+nCheckers; i++ )
        e *= i;
    e /= factorial( smaller );
    e -= 1;
    return e;
}

int intPow(int a,int b);
int intPow(int a,int b)
{
    int res=1;
    for( int i=0; i<b; i++ ) res *= a;
    return res;
}

int blockadeID( const board& b, int startPoint )
{
    int ID=0;
    for( int i=startPoint-1; i>=startPoint-12; i-- )
    {
        if( i == -1 ) break;
        if( b.otherChecker(i) > 1 ) ID += intPow(2,startPoint-i-1);
    }
    return ID;
}

hash_map<int,int> * blockadeEscapeRolls() { return _escapeRolls; }

bool canEscape( const board& b, int startPoint, int minPoint, const vector<int>& rolls );
bool canEscape( const board& b, int startPoint, int minPoint, const vector<int>& rolls )
{
    // true if a checker started at startPoint can get past minPoint by moving the rolls
    // in order.
    
    int pnt=startPoint;
    for( int i=0; i<rolls.size(); i++ )
    {
        pnt -= rolls.at(i);
        if( pnt < minPoint ) return true;
        if( b.otherChecker(pnt) > 1 ) return false;
    }
    return false;
}

int getBlockadeEscapeCount( const board& b, int startPoint )
{
    // if the db hasn't been initialized, throw
    
    if( _escapeRolls == 0 ) throw "Escape database has not been constructed - call constructBlockadeEscapeDb";
    
    // get the board ID
    
    int ID = blockadeID( b, startPoint );
    if( ID == 0 ) return 36; // no blockade
    
    // if there's already an entry for this one, use it
    
    hash_map<int,int>::iterator it=_escapeRolls->find( ID );
    if( it != _escapeRolls->end() ) return it->second;
    
    // get the index of the last covered point out to the 12 points further than startPoint
    
    int minPoint=startPoint;
    int i, j;
    for( i=startPoint-1; i>=startPoint-12; i-- )
    {
        if( i == -1 ) break;
        if( b.otherChecker(i) > 1 ) minPoint = i;
    }
    
    // if there are no opponent covered points ahead, then return 36 - all rolls "escape"
    
    if( minPoint == startPoint ) return 36;
    
    // otherwise calculate it - totally brute force!
    
    int nRolls=0;
    
    for( i=1; i<=6; i++ )
        for( j=1; j<=i; j++ )
        {
            vector<int> rolls;
            rolls.push_back(i);
            rolls.push_back(j);
            if( i == j )
            {
                rolls.push_back(i);
                rolls.push_back(j);
                if( canEscape( b, startPoint, minPoint, rolls ) )
                    nRolls += 1;
            }
            else
            {
                bool esp = canEscape( b, startPoint, minPoint, rolls );
                if( !esp )
                {
                    reverse( rolls.begin(), rolls.end() );
                    esp = canEscape( b, startPoint, minPoint, rolls );
                }
                if( esp ) nRolls += 2;
            }
        }
    
    // cache the value in the map for use next time
    
    (*_escapeRolls)[ ID ] = nRolls;
    
    // return the # of rolls
    
    return nRolls;
}

vector< vector<bool> > getLayouts( int nCovered, int nPnts );
vector< vector<bool> > getLayouts( int nCovered, int nPnts )
{
    if( nCovered > nPnts ) throw "nCovered cannot be larger than nPnts";
    
    vector< vector<bool> > layouts;
    if( nCovered == 1 )
    {
        for( int i=0; i<nPnts; i++ )
        {
            vector<bool> layout(nPnts,false);
            layout.at(i) = true;
            layouts.push_back(layout);
        }
    }
    else
    {
        for( int i=0; i<nPnts-nCovered+1; i++ )
        {
            vector< vector<bool> > subLayouts( getLayouts( nCovered-1, nPnts-i-1 ) );
            for( vector< vector<bool> >::iterator it=subLayouts.begin(); it!=subLayouts.end(); it++ )
            {
                vector<bool> layout(i+1,false);
                layout.at(i) = true;
                layout.insert( layout.end(), it->begin(), it->end() );
                layouts.push_back( layout );
            }
        }
    }
    return layouts;
}

void constructBlockadeEscapeDb()
{
    // initialize the local map if it hasn't been already
    
    if( _escapeRolls == 0 )
        _escapeRolls = new hash_map<int,int>;
    
    // run through all possible ways to arrange up to seven covered points on twelve points
    // and calculate for each
    
    int i, nRolls;
    
    for( int nCovered=1; nCovered<8; nCovered++ )
    {
        vector< vector<bool> > layouts( getLayouts( nCovered, 12 ) );
        for( vector< vector<bool> >::iterator it=layouts.begin(); it!=layouts.end(); it++ )
        {
            // set up a board with opponent checkers in the appropriate places
            
            board b;
            for( i=0; i<24; i++ )
            {
                b.setChecker( i, 0 );
                b.setOtherChecker( i, 0 );
            }
            for( i=0; i<12; i++ )
                if( it->at(i) == true )
                    b.setOtherChecker( 22-i, 2 );
            
            // calculate the number of escaping rolls. No need to do anything with this; the
            // result gets cached in the local hash automatically.
            
            nRolls = getBlockadeEscapeCount( b, 23 );
        }
    }
}
