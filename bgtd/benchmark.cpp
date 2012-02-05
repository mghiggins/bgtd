//
//  benchmark.cpp
//  bgtd
//
//  Created by Mark Higgins on 2/4/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#include <fstream>
#include <sstream>
#include "benchmark.h"
#include "randomc.h"
#include "gamefns.h"
#include "strategyply.h"

class boardAndVals
{
public:
    boardAndVals() {};
    boardAndVals( const board& b, double val0Ply ) : b(b), val0Ply(val0Ply) {};
    
    board b;
    double val0Ply;
    double val2Ply;
};

bool boardAndValComp( const boardAndVals& v1, const boardAndVals& v2 );
bool boardAndValComp( const boardAndVals& v1, const boardAndVals& v2 ) { return v1.val0Ply > v2.val0Ply; }

string writeBenchmarks( vector<boardAndVals>& benchmarks, const string& pathName, int& fileIndex, int fileSuffix );
string writeBenchmarks( vector<boardAndVals>& benchmarks, const string& pathName, int& fileIndex, int fileSuffix )
{
    // construct the full file name. Assumes the filesystem path already exists;
    // it'll just create the file
    
    stringstream ss;
    ss << pathName << "/benchmark" << fileIndex << "_" << fileSuffix << ".txt";
    string fileName = ss.str();
    
    // write the file out
    
    ofstream f( fileName.c_str() );
    for( vector<boardAndVals>::iterator it=benchmarks.begin(); it!=benchmarks.end(); it++ )
        f << it->b.repr() << "," << it->val0Ply << "," << it->val2Ply << endl;
    
    f.close();
    //cout << "Wrote file " << fileName << endl;
    
    // increment the file index and clear the benchmark list so we can start fresh on
    // the next set
    
    fileIndex++;
    benchmarks.clear();
    benchmarks.resize(0);
    
    return fileName;
}

vector< vector<string> > parallelFileNames;

class workerGenBenchmarks
{
public:
    workerGenBenchmarks( strategyprob& strat, strategyprob& filterStrat, int nGames, const string& pathName, int nFileBenchmarks, int n, int initSeed ) 
    : strat(strat), filterStrat(filterStrat), nGames(nGames), pathName(pathName), nFileBenchmarks(nFileBenchmarks), n(n), initSeed(initSeed) {};
    
    void operator()()
    {
        int seed = initSeed + n;
        vector<string> fileNames = generateBenchmarkPositionsSerial( strat, filterStrat, nGames, pathName, nFileBenchmarks, seed, n );
        parallelFileNames.at(n) = fileNames;
    }
    
private:
    strategyprob& strat;
    strategyprob& filterStrat;
    int nGames;
    string pathName;
    int nFileBenchmarks;
    int n;
    int initSeed;
};

void generateBenchmarkPositions( strategyprob& strat, strategyprob& filterStrat, int nGames, const string& pathName, int nFileBenchmarks, int seed, int nThreads )
{
    using namespace boost;
    
    // run each set of games in its own thread, writing out files with the appropriate file suffix
    // so as not to step on each others' toes. Each thread writes out a set of benchmark files and
    // stores the list of file names in the parallelFileNames vector.
    
    parallelFileNames.resize( nThreads );
    if( nGames % nThreads != 0 ) throw string( "nGames must be a multiple of nThreads" );
    int nGamesPerThread = nGames / nThreads;
    
    thread_group ts;
    for( int i=0; i<nThreads; i++ ) ts.create_thread( workerGenBenchmarks( strat, filterStrat, nGamesPerThread, pathName, nFileBenchmarks, i, seed )  );
    ts.join_all();
    
    // write out the benchinfo.txt file with the names of all created files
    
    string fileName = pathName + "/benchinfo.txt";
    ofstream f( fileName.c_str() );
    
    for( int i=0; i<nThreads; i++ )
    {
        for( vector<string>::iterator it=parallelFileNames.at(i).begin(); it!=parallelFileNames.at(i).end(); it++ )
            f << (*it) << endl;
    }
    
    f.close();
}

vector<string> generateBenchmarkPositionsSerial( strategyprob& strat, strategyprob& filterStrat, int nGames, const string& pathName, int nFileBenchmarks, int seed, int fileSuffix )
{
    int fileIndex=0, die1, die2;
    vector<boardAndVals> benchmarks;
    strategyply strat2Ply( 2, 8, 0.2, strat, filterStrat );
    int moveFilter=3; // number of moves the 2-ply strategy will consider from the sorted 0-ply movesdo
    double equityThreshold=0.005; // threshold equity difference btw best 0- and 2-ply moves to count as a real diff
    int i, index2Ply;
    double maxVal2Ply, val2Ply;
    vector<string> fileNames;
    
    CRandomMersenne rng(seed);
    
    for( int gameIndex=0; gameIndex<nGames; gameIndex++ )
    {
        //cout << "Game " << gameIndex+1 << endl;
        
        // starting board
        
        board b;
        int nSteps=0;
        
        // step through the game. Don't use a game object for this as we want to do custom
        // stuff at each step.
        
        while( b.bornIn() != 15 and b.otherBornIn() != 15 )
        {
            //cout << "  step " << nSteps << "; nb = " << benchmarks.size() << endl;
            
            die1 = rng.IRandom( 1, 6 );
            die2 = rng.IRandom( 1, 6 );
            
            set<board> moves( possibleMoves( b, die1, die2 ) );
            //cout << "  rolls " << die1 << ", " << die2 << ": " << moves.size() << " moves to check" << endl;
            
            // evaluate the moves using the 0-ply strategy
            
            vector<boardAndVals> vals;
            for( set<board>::iterator it=moves.begin(); it!=moves.end(); it++ )
                vals.push_back( boardAndVals( (*it), strat.boardValue( (*it) ) ) );
            
            // sort the moves by 0-ply equity
            
            sort( vals.begin(), vals.end(), boardAndValComp );
            
            // for the top moveFilter 0-ply moves, calculate the 2-ply equity
            
            maxVal2Ply = -100;
            index2Ply  = -1;
            
            for( i=0; i<moveFilter; i++ )
            {
                if( i >= vals.size() ) break;
                val2Ply = strat2Ply.boardValue( vals.at(i).b );
                //cout << "     " << vals.at(i).val0Ply << "; " << val2Ply << endl;
                if( val2Ply > maxVal2Ply )
                {
                    maxVal2Ply = val2Ply;
                    index2Ply  = i;
                }
                vals.at(i).val2Ply = val2Ply;
            }
            
            // if the best move suggested by 0-ply isn't the best one suggested
            // by 2-ply, and if the 2-ply equity diff btw those two moves is
            // greater than the threshold, add the two boards (0- and 2-ply choices)
            // to the benchmarks
            
            if( index2Ply > 0 and vals.at(index2Ply).val2Ply - vals.at(0).val2Ply > equityThreshold )
            {
                benchmarks.push_back( vals.at(0) );
                benchmarks.push_back( vals.at(index2Ply) );
                
                if( benchmarks.size() >= nFileBenchmarks )
                    fileNames.push_back( writeBenchmarks( benchmarks, pathName, fileIndex, fileSuffix ) );
            }
            
            // make the best 2-ply move
            
            if( index2Ply != -1 ) b = vals.at(index2Ply).b;
            b.setPerspective( 1 - b.perspective() );
            b.print();
            
            nSteps++;
        }
    }
    
    // if there are any benchmarks we haven't written out, do so now
    
    if( benchmarks.size() )
        fileNames.push_back( writeBenchmarks( benchmarks, pathName, fileIndex, fileSuffix ) );
    
    return fileNames;
}

class boardAndRolloutProbs
{
public:
    boardAndRolloutProbs() {};
    boardAndRolloutProbs( const board& b, const gameProbabilities& probs ) : b(b), probs(probs) {};
    
    board b;
    gameProbabilities probs;
};

void writeRollouts( vector<boardAndRolloutProbs>& rollouts, const string& pathName, int& fileIndex );
void writeRollouts( vector<boardAndRolloutProbs>& rollouts, const string& pathName, int& fileIndex )
{
    // construct the full file name. Assumes the filesystem path already exists;
    // it'll just create the file
    
    stringstream ss;
    ss << pathName << "/rollout" << fileIndex << ".txt";
    string fileName = ss.str();
    
    // write the file out
    
    ofstream f( fileName.c_str() );
    for( vector<boardAndRolloutProbs>::iterator it=rollouts.begin(); it!=rollouts.end(); it++ )
        f << it->b.repr() << "," << it->probs.probWin << "," << it->probs.probGammonWin << "," << it->probs.probGammonLoss << "," << it->probs.probBgWin << "," << it->probs.probBgLoss << endl;
    
    f.close();
    cout << "Wrote file " << fileName << endl;
    
    // increment the file index and clear the rollout list so we can start fresh on
    // the next set
    
    fileIndex++;
    rollouts.clear();
    rollouts.resize(0);
}

void rolloutBenchmarkPositions( strategyprob& strat, const string& pathName, int nFileBenchmarks, int nRuns, int seed, int nThreads )
{
    // open the benchinfo file to get the names of benchmark files
    
    string fileName = pathName + "/benchinfo.txt";
    ifstream fi( fileName.c_str(), ios::in );
    if( !fi ) throw string( "No benchinfo file found" );
    
    vector<boardAndRolloutProbs> rollouts;
    int fileIndex=0;
    
    // run through the benchmark files and roll out each position
    
    while( !fi.eof() )
    {
        string benchFileName;
        getline( fi, benchFileName );
        ifstream fb( benchFileName.c_str(), ios::in );
        if( !fb )
        {
            stringstream ss;
            ss << "Could not find benchmark file name " << benchFileName;
            throw ss.str();
        }
        string line, brdRepr;
        while( !fb.eof() )
        {
            getline( fb, line );
            if( line != "" )
            {
                stringstream sb(line);
                getline( sb, brdRepr, ',' );
                
                // construct the board and roll it out
                
                board b( brdRepr );
                b.print();
                gameProbabilities probs = rolloutBoardProbabilitiesParallel( b, strat, nRuns, seed, nThreads, false );
                //gameProbabilities probs = rolloutBoardProbabilities( b, strat, nRuns, seed );
                cout << probs << endl;
                
                rollouts.push_back( boardAndRolloutProbs( b, probs ) );
                if( rollouts.size() >= nFileBenchmarks )
                    writeRollouts( rollouts, pathName, fileIndex );
            }
        }
    }
    
    // if there are any benchmarks we haven't written out, do so now
    
    if( rollouts.size() )
        writeRollouts( rollouts, pathName, fileIndex );
}