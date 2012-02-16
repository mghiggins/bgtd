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
    cout << "Wrote file " << fileName << endl;
    
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

void generateBenchmarkPositions( strategyprob& strat, strategyprob& filterStrat, int nGames, const string& pathName, int nFileBenchmarks, int seed, int nThreads, int initIndex )
{
    using namespace boost;
    
    // run each set of games in its own thread, writing out files with the appropriate file suffix
    // so as not to step on each others' toes. Each thread writes out a set of benchmark files and
    // stores the list of file names in the parallelFileNames vector.
    
    parallelFileNames.resize( nThreads );
    if( nGames % nThreads != 0 ) throw string( "nGames must be a multiple of nThreads" );
    int nGamesPerThread = nGames / nThreads;
    
    thread_group ts;
    for( int i=0; i<nThreads; i++ ) ts.create_thread( workerGenBenchmarks( strat, filterStrat, nGamesPerThread, pathName, nFileBenchmarks, initIndex+i, seed )  );
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
            
            //cout << "     got 0-ply values\n";
            
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
            //b.print();
            
            nSteps++;
        }
    }
    
    // if there are any benchmarks we haven't written out, do so now
    
    if( benchmarks.size() )
        fileNames.push_back( writeBenchmarks( benchmarks, pathName, fileIndex, fileSuffix ) );
    
    return fileNames;
}

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

void rolloutBenchmarkPositions( strategyprob& strat, const string& pathName, int nFileBenchmarks, int nRuns, int seed, int nThreads, int initFileIndex )
{
    // open the benchinfo file to get the names of benchmark files
    
    string fileName = pathName + "/benchinfo.txt";
    ifstream fi( fileName.c_str(), ios::in );
    if( !fi ) throw string( "No benchinfo file found" );
    
    vector<boardAndRolloutProbs> rollouts;
    int fileIndex=initFileIndex;
    
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
    
    // write out an info file with the # of rollout files
    
    string infoFileName = pathName + "/rolloutinfo.txt";
    ofstream f( infoFileName.c_str() );
    f << fileIndex << endl;
    f.close();
}

void trainMult( strategytdmult& strat, const string& pathName, int seed )
{
    // get the number of rollout files from the rolloutinfo.txt file in the path
    
    string infoFileName = pathName + "/rolloutinfo.txt";
    ifstream f0( infoFileName.c_str(), ios::in );
    if( !f0 ) throw string( "Could not find rolloutinfo.txt file" );
    
    string line, bit;
    getline( f0, line );
    int nRolloutFiles = atoi( line.c_str() );
    
    double probWin, probGammonWin, probGammonLoss, probBgWin, probBgLoss;
    
    // load all the rollout info
    
    vector<boardAndRolloutProbs> states;
    for( int i=0; i<nRolloutFiles; i++ )
    {
        stringstream ss;
        ss << pathName << "/rollout" << i << ".txt";
        ifstream fr( ss.str().c_str(), ios::in );
        if( !fr )
        {
            ss << " not found";
            throw ss.str();
        }
        while( !fr.eof() )
        {
            getline( fr, line );
            if( line != "" )
            {
                // the line is comma-delimited: board,prob of win,prob of gammon win,prob of gammon loss,prob of bg win,prob of bg loss.
                // Parse all that out.
                
                stringstream sb(line);
                getline( sb, bit, ',' );
                board b(bit);
                getline( sb, bit, ',' );
                probWin = atof( bit.c_str() );
                getline( sb, bit, ',' );
                probGammonWin = atof( bit.c_str() );
                getline( sb, bit, ',' );
                probGammonLoss = atof( bit.c_str() );
                getline( sb, bit, ',' );
                probBgWin = atof( bit.c_str() );
                getline( sb, bit, ',' );
                probBgLoss = atof( bit.c_str() );
                
                // add the board & rolled-out probabilities to the list
                
                gameProbabilities probs( probWin, probGammonWin, probGammonLoss, probBgWin, probBgLoss );
                states.push_back( boardAndRolloutProbs( b, probs ) );
            }
        }
    }
    
    // now do the training on randomly-selected boards
    
    int index, count=0;
    CRandomMersenne rng(seed);
    
    while( states.size() > 0 )
    {
        // randomly select the board
        
        index = rng.IRandom( 0, (int) (states.size() - 1) );
        
        // run supervised learning for this step. The rollout probabilities represent probabilities
        // assuming the opponent has the dice, whereas the network probabilities assume that
        // the player has the dice. So we need to flip perspective around.
        
        board b( states.at(index).b );
        b.setPerspective( 1 - b.perspective() );
        
        strat.updateFromProbs( b, 1 - states.at(index).probs.probWin, 
                               states.at(index).probs.probGammonLoss, states.at(index).probs.probGammonWin,
                               states.at(index).probs.probBgLoss, states.at(index).probs.probBgWin );
        
        // remove the entry from the states list so we don't do it again
        
        states.erase( states.begin() + index );
        count++;
    }
}

void printErrorStatistics( strategytdmult& strat, const string& pathName )
{
    // get the number of rollout files from the rolloutinfo.txt file in the path
    
    string infoFileName = pathName + "/rolloutinfo.txt";
    ifstream f0( infoFileName.c_str(), ios::in );
    if( !f0 ) throw string( "Could not find rolloutinfo.txt file" );
    
    string line, bit;
    getline( f0, line );
    int nRolloutFiles = atoi( line.c_str() );
    
    double probWin, probGammonWin, probGammonLoss, probBgWin, probBgLoss;
    
    // run through the rolled-out benchmarks
    
    double avgDiff=0, avgDiffSq=0, avgProb=0, avgProbSq=0;
    int count=0;
    
    for( int i=0; i<nRolloutFiles; i++ )
    {
        stringstream ss;
        ss << pathName << "/rollout" << i << ".txt";
        ifstream fr( ss.str().c_str(), ios::in );
        if( !fr )
        {
            ss << " not found";
            throw ss.str();
        }
        while( !fr.eof() )
        {
            getline( fr, line );
            if( line != "" )
            {
                // the line is comma-delimited: board,prob of win,prob of gammon win,prob of gammon loss,prob of bg win,prob of bg loss.
                // Parse all that out.
                
                stringstream sb(line);
                getline( sb, bit, ',' );
                board b(bit);
                getline( sb, bit, ',' );
                probWin = atof( bit.c_str() );
                getline( sb, bit, ',' );
                probGammonWin = atof( bit.c_str() );
                getline( sb, bit, ',' );
                probGammonLoss = atof( bit.c_str() );
                getline( sb, bit, ',' );
                probBgWin = atof( bit.c_str() );
                getline( sb, bit, ',' );
                probBgLoss = atof( bit.c_str() );
                
                gameProbabilities probs( probWin, probGammonWin, probGammonLoss, probBgWin, probBgLoss );
                
                // calculate the equity from rollout and the equity from the strategy
                
                gameProbabilities calcProbs( strat.boardProbabilities(b) );
                
                double equityDiff = strat.boardValueFromProbs(calcProbs) - strat.boardValueFromProbs(probs);
                double probDiff   = calcProbs.probWin - probWin;
                
                avgDiff   += equityDiff;
                avgDiffSq += equityDiff * equityDiff;
                avgProb   += probDiff;
                avgProbSq += probDiff * probDiff;
                
                count++;
            }
        }
    }
    
    cout << count << " scenarios\n";
    avgDiff   /= count;
    avgDiffSq /= count;
    avgProb   /= count;
    avgProbSq /= count;
    
    cout << "Average diff = " << avgDiff << endl;
    cout << "Std dev      = " << sqrt( avgDiffSq - avgDiff * avgDiff ) << endl;
    cout << "Avg prob df  = " << avgProb << endl;
    cout << "Std dev      = " << sqrt( avgProbSq - avgProb * avgProb ) << endl;
}

vector<boardAndRolloutProbs> gnuBgBenchmarkStates( const string& fileName )
{
    // load in the info
    
    vector<boardAndRolloutProbs> states;
    ifstream f( fileName.c_str(), ios::in );
    if( !f ) throw string( "File does not exist" );
    
    string line, bit;
    double probWin, probGammonWin, probGammonLoss, probBgWin, probBgLoss;
    
    while( !f.eof() )
    {
        getline( f, line );
        if( line != "" and line[0] != '#' )
        {
            stringstream sb(line);
            getline( sb, bit, ' ' );
            board b;
            b.setFromJosephID( bit );
            getline( sb, bit, ' ' );
            probWin = atof( bit.c_str() );
            getline( sb, bit, ' ' );
            probGammonWin = atof( bit.c_str() );
            getline( sb, bit, ' ' );
            probBgWin = atof( bit.c_str() );
            getline( sb, bit, ' ' );
            probGammonLoss = atof( bit.c_str() );
            getline( sb, bit, ' ' );
            probBgLoss = atof( bit.c_str() );
            
            gameProbabilities probs( probWin, probGammonWin, probGammonLoss, probBgWin, probBgLoss );
            
            states.push_back( boardAndRolloutProbs( b, probs ) );
        }
    }
    
    cout << "Loaded " << states.size() << " training states from " << fileName << "\n";
    
    return states;
}

void trainMultGnuBg( strategytdmult& strat, const vector<boardAndRolloutProbs>& states, int seed )
{
    // do the training on randomly-ordered boards
    
    int index, count=0;
    CRandomMersenne rng(seed);
    
    vector<int> indexes(states.size());
    for( int i=0; i<states.size(); i++ ) indexes[i] = i;
    
    while( indexes.size() > 0 )
    {
        if( count % 100000 == 0 ) cout << "Training step " << count << endl;
        
        // randomly select the board
        
        index = rng.IRandom( 0, (int) (indexes.size() - 1) );
        
        // run supervised learning for this step. The rollout probabilities represent probabilities
        // assuming the player has the dice.
        
        const board& b( states.at(indexes[index]).b );
        const gameProbabilities& probs( states.at(indexes[index]).probs );
        
        strat.updateFromProbs( b, probs.probWin, 
                              probs.probGammonWin, probs.probGammonLoss,
                              probs.probBgWin, probs.probBgLoss );
        
        // remove the entry from the states list so we don't do it again
        
        indexes.erase( indexes.begin() + index );
        count++;
    }
}

void printErrorStatisticsGnuBg( strategytdmult& strat, const vector<boardAndRolloutProbs>& states )
{
    // run through the rolled-out benchmarks
    
    double avgDiff=0, avgDiffSq=0, avgProb=0, avgProbSq=0;
    int count=0;
    
    for( vector<boardAndRolloutProbs>::const_iterator it=states.begin(); it!=states.end(); it++ )
    {
        // these probs are assuming the player holds the dice; want to compare them to
        // boardProbabilities which assumes that the opponent has the dice. So need to
        // flip things around a bit.
        
        gameProbabilities probs( it->probs.flippedProbs() );
        
        // calculate the equity from rollout and the equity from the strategy
        
        board flippedBoard(it->b);
        flippedBoard.setPerspective(1);
        
        gameProbabilities calcProbs( strat.boardProbabilities(flippedBoard) );
        
        double equityDiff = strat.boardValueFromProbs(calcProbs) - strat.boardValueFromProbs(probs);
        double probDiff   = calcProbs.probWin - probs.probWin;
        
        avgDiff   += equityDiff;
        avgDiffSq += equityDiff * equityDiff;
        avgProb   += probDiff;
        avgProbSq += probDiff * probDiff;
        
        count++;
    }
    
    cout << count << " scenarios\n";
    avgDiff   /= count;
    avgDiffSq /= count;
    avgProb   /= count;
    avgProbSq /= count;
    
    cout << "Average diff = " << avgDiff << endl;
    cout << "Std dev      = " << sqrt( avgDiffSq - avgDiff * avgDiff ) << endl;
    cout << "Avg prob df  = " << avgProb << endl;
    cout << "Std dev      = " << sqrt( avgProbSq - avgProb * avgProb ) << endl;
}

vector< vector<benchmarkData> > loadBenchmarkData( const string& fileName, int nBuckets )
{
    vector< vector<benchmarkData> > dataSets(nBuckets);
    int index=0;
    
    ifstream f( fileName.c_str(), ios::in );
    if( !f ) throw string( "File does not exist" );
    
    string line, bit;
    int die1, die2;
    double bestEquity, otherEquity;
    
    while( !f.eof() )
    {
        getline( f, line );
        if( line != "" and line[0] == 'm' )
        {
            stringstream sb(line);
            getline( sb, bit, ' ' ); // first one is the 'm'; skip it
            getline( sb, bit, ' ' ); // next is the board name
            board startBoard;
            startBoard.setFromJosephID( bit );
            
            // next are the two dice
            
            getline( sb, bit, ' ' );
            die1 = atoi( bit.c_str() );
            getline( sb, bit, ' ' );
            die2 = atoi( bit.c_str() );
            
            // next are pairs of (board name,equity). The first pair is the best move with its equity; after that are
            // other moves with equity diff vs the best move.
            
            getline( sb, bit, ' ' );
            board bestBoard;
            bestBoard.setFromJosephID( bit );
            getline( sb, bit, ' ' );
            bestEquity = atof( bit.c_str() );
            
            vector<board> otherBoards;
            vector<double> otherEquities;
            
            while( !sb.eof() )
            {
                getline( sb, bit, ' ' );
                if( bit == "" ) break;
                board otherB;
                otherB.setFromJosephID( bit );
                getline( sb, bit, ' ' );
                otherEquity = bestEquity - atof( bit.c_str() );
                
                otherBoards.push_back( otherB );
                otherEquities.push_back( otherEquity );
            }
            
            // add the element to the right bucket
            
            dataSets.at(index).push_back( benchmarkData( startBoard, die1, die2, bestBoard, bestEquity, otherBoards, otherEquities ) );
            index ++;
            if( index == nBuckets ) index = 0;
        }
    }
    
    int nTot=0;
    for( int i=0; i<dataSets.size(); i++ ) nTot += dataSets.at(i).size();
    
    cout << "Loaded " << nTot << " benchmark moves from " << fileName << endl;
    
    return dataSets;
}

double gnuBgBenchmarkStatisticsSerial( strategy& strat, const vector<benchmarkData>& benchmarks )
{
    int i;
    bool foundBoard;
    double equityErr=0, dEquity;
    
    for( vector<benchmarkData>::const_iterator it=benchmarks.begin(); it!=benchmarks.end(); it++ )
    {
        // get the preferred move from the strategy
        
        set<board> moves( possibleMoves( it->startBoard, it->die1, it->die2 ) );
        board stratBoard( strat.preferredBoard( it->startBoard, moves ) );
        
        // check if the strategy chose the best move
        
        foundBoard = false;
        
        if( stratBoard.equalsFlipped( it->bestBoard ) )
        {
            dEquity = 0;
            foundBoard = true;
        }
        
        int nOthers = (int) it->otherBoards.size();
        if( !foundBoard )
        {
            for( i=0; i<nOthers; i++ )
            {
                if( stratBoard.equalsFlipped( it->otherBoards.at(i) ) )
                {
                    dEquity = it->bestEquity - it->otherEquities.at(i);
                    foundBoard = true;
                    break;
                }
            }
        }
        
        // if it didn't find the strategy's choice in the rollout list, it's worse than even the
        // last tracked move there. In this case use the worst equity from the list. That's a bit
        // aggressive since it'll make the strategy look better than it really is, but not sure what
        // would be better. Should never be the case that the list of other boards is empty in this
        // case though (since that'd mean that there's only one possible move, but the strategy didn't
        // choose it).
        
        if( !foundBoard and nOthers == 0 ) throw string( "Don't know how this happened!" );
        
        // generate the statistic for Snowie error rating. This is 1000*(sum of equity errors)/(# of player moves + # of opponent moves)
        // Since we're not playing a real game here, I'll approximate the denominator with 2*number of moves we check, which is just
        // 2*the number of elements we're analyzing (so no need to track that here - just the summed equity error).
        
        equityErr += dEquity;
    }
    
    return equityErr;
}

vector<double> parallelEquityErrors;

class workerGnuBgBenchmarks
{
public:
    workerGnuBgBenchmarks( strategy& strat, const vector< vector<benchmarkData> >& dataSet, int index )
      : strat(strat), dataSet(dataSet), index(index) {};
    
    void operator()()
    {
        double equityErr = gnuBgBenchmarkStatisticsSerial( strat, dataSet.at(index) );
        parallelEquityErrors.at(index) = equityErr;
    }
    
private:
    strategy& strat;
    const vector< vector<benchmarkData> >& dataSet;
    int index;
};

double gnuBgBenchmarkER( strategy& strat, const vector< vector<benchmarkData> >& dataSet )
{
    using namespace boost;
    int nThreads = (int) dataSet.size();
    
    // generate the threads for each piece
    
    parallelEquityErrors.resize( nThreads );
    
    thread_group ts;
    for( int i=0; i<nThreads; i++ ) ts.create_thread( workerGnuBgBenchmarks( strat, dataSet, i ) );
    ts.join_all();
    
    // aggregate the data
    
    double equityErr=0;
    double numMoves=0;
    for( int i=0; i<nThreads; i++ )
    {
        equityErr += parallelEquityErrors.at(i);
        numMoves  += dataSet.at(i).size();
    }
    
    double ER = 1000*equityErr/numMoves;
    
    cout << "ER = " << ER << endl;
    cout << "Number of analyzed moves = " << numMoves << endl;
    return ER;
}

