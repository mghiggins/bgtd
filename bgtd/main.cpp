//
//  main.cpp
//  bgtd
//
//  Created by Mark Higgins on 7/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <string>
//#include <pthread.h>
#include <stdlib.h>
#include <cmath>
#include "game.h"
#include "board.h"
#include "randomc.h"
#include "gamefns.h"
#include "strategytd.h"
#include "strategypubeval.h"
#include "tests.h"

using namespace std;

/*
int main( int argc, const char * argv [] )
{
    try 
    {
        if( !rollTest() )
        cout << "FAILED!\n";
        else
        cout << "Succeeded\n";
    }
    catch( const string& err )
    {
        cout << "ERROR: " << err << endl;
    }

    return 0;
}
*/

/*
int main (int argc, const char * argv[])
{
    board b;
    
    int i;
    for( i=0; i<24; i++ )
    {
        b.setChecker(i, 0);
        b.setOtherChecker(i, 0);
    }
    
    for( i=0; i<12; i++ )
    {
        b.setPerspective( 0 );
        b.incrementBornIn();
        b.setPerspective( 1 );
        b.incrementBornIn();
    }
    
    b.setPerspective( 0 );
    b.setChecker( 1, 3 );
    b.setOtherChecker( 22, 3 );
    
    
    b.print();
    
    int d1=2;
    int d2=1;
    
    set<board> moves = possibleMoves( b, d1, d2 );
    
    cout << "number of proposed moves = " << moves.size() << "\n";
    
    for( set<board>::iterator i=moves.begin(); i!=moves.end(); i++ )
    {
        (*i).print();
    }
    
    
    return 0;
}
*/

/*
int main( int argc, const char * argv[] )
{
    strategySimple s1( 100, 1, 500 );
    strategySimple s2( 100, 10, 500 );
    //strategySimple s2;
    
    float w0=0;
    float avgSteps=0;
    long n=200;
    int initSeed=4;
    
    for( long i=0; i<n; i++ )
    {
        if( i!=0 && i%20 == 0 ) cout << "Step " << i << ": " << w0/i << " / " << avgSteps/i << endl;
        
        game g( &s2, &s1, (int)(i+initSeed) );
        g.setTurn( ((int)i)%2 );
        g.stepToEnd();
        //cout << g.winner() << "/" << g.nSteps << endl;
        if( g.winner() == 0 ) w0 ++;
        avgSteps += g.nSteps;
    }
    
    avgSteps /= n;
    w0 /= n;
    cout << endl;
    cout << "Prob of white winning = " << w0 * 100 << endl;
    cout << "Average steps/game    = " << avgSteps << endl;
}
*/
/*
class gameInputs
{
public:
    int initSeed;
    long gameIndex;
    long nRuns;
    vector<int> winners;
    vector<int> steps;
    strategytd refStrat;
    pthread_t thread;
};

void * runGame( void * input );

void * runGame( void * input )
{
    gameInputs * params = reinterpret_cast<gameInputs*>( input );
    if( params )
    {
        strategySimple s1( 100, 10, 500 );
        for( long i=0; i<params->nRuns; i++ )
        {
            game g( &params->refStrat, &s1, (int)(i+params->gameIndex+params->initSeed) );
            g.setTurn( ((int)(i+params->gameIndex))%2 );
            g.stepToEnd();
            params->winners[ i ] = g.winner();
            params->steps[ i ]   = g.nSteps;
        }
    }
    return 0;
}
*/
/*
int main( int argc, const char * argv[] )
{
    // this one runs a set of games, all in their own separate threads, so the OS can schedule them in parallel
    
    float w0=0;
    float avgSteps=0;
    long n=600;
    long nRuns=50;
    int initSeed=4;
    long i;
    
    if( n % nRuns != 0 ) throw string( "Must have uniform buckets" );
    
    long nThreads = n/nRuns;
    
    vector<gameInputs> paramVec;
    paramVec.resize( nThreads );
    
    for( i=0; i<nThreads; i++ )
    {
        paramVec[i].initSeed  = initSeed;
        paramVec[i].gameIndex = i*nRuns;
        paramVec[i].nRuns     = nRuns;
        paramVec[i].winners.resize( nRuns );
        paramVec[i].steps.resize( nRuns );
        
        pthread_create( &(paramVec[i].thread), 0, runGame, &(paramVec[i]) );
    }
    
    void * ignore = 0;
    for ( i=0; i<paramVec.size(); i++ )
        pthread_join( paramVec[i].thread, &ignore );
    
    for( vector<gameInputs>::iterator i=paramVec.begin(); i!=paramVec.end(); i++ )
    {
        for( long j=0; j<nRuns; j++ )
            if( (*i).winners[j] == 0 )
                w0 += 1;
        
        for( long j=0; j<nRuns; j++ )
            avgSteps += (*i).steps[j];
    }
    
    w0 /= n;
    avgSteps /= n;
    
    cout << "Prob of white winning = " << w0 * 100 << endl;
    cout << "Average steps/game    = " << avgSteps << endl;
}
*/
/*
void playDistrib( strategytd& s1, long n, long initSeed );

void playDistrib( strategytd& s1, long n, long initSeed )
{
    // now play it against the simple strategy to see how well it does
    
    s1.learning = false;
    
    float w0=0;
    float avgSteps=0;
    long nRuns=n/4;
    long j;
    
    if( n % nRuns != 0 ) throw string( "Must have uniform buckets" );
    
    long nThreads = n/nRuns;
    
    vector<gameInputs> paramVec;
    paramVec.resize( nThreads );
    
    for( j=0; j<nThreads; j++ )
    {
        paramVec[j].initSeed  = (int) initSeed;
        paramVec[j].gameIndex = j*nRuns;
        paramVec[j].nRuns     = nRuns;
        paramVec[j].winners.resize( nRuns );
        paramVec[j].steps.resize( nRuns );
        paramVec[j].refStrat  = s1;
        
        pthread_create( &(paramVec[j].thread), 0, runGame, &(paramVec[j]) );
    }
    
    void * ignore = 0;
    for( j=0; j<paramVec.size(); j++ )
        pthread_join( paramVec[j].thread, &ignore );
    
    for( vector<gameInputs>::iterator k=paramVec.begin(); k!=paramVec.end(); k++ )
    {
        for( j=0; j<nRuns; j++ )
            if( (*k).winners[j] == 0 )
                w0 += 1;
        
        for( j=0; j<nRuns; j++ )
            avgSteps += (*k).steps[j];
    }
    
    w0 /= n;
    avgSteps /= n;
    
    cout << "Prob of TD winning = " << w0 * 100 << endl;
    cout << "Average steps/game = " << avgSteps << endl;
    cout << endl;
}
*/

/*
 int main( int argc, char * argv [] )
 {
 ifstream fo( "/Users/mghiggins/weightsOut.txt" );
 ifstream fot( "/Users/mghiggins/tracesOut.txt" );
 ifstream fm( "/Users/mghiggins/weightsMiddle.txt" );
 ifstream fmt( "/Users/mghiggins/tracesMiddle.txt" ); 
 
 // load the weights
 
 int i, j;
 int nMiddle=40;
 string line;
 
 vector<double> outputWeights, outputTraces;
 outputWeights.resize(nMiddle-1);
 outputTraces.resize(nMiddle-1);
 for( int i=0; i<nMiddle-1; i++ )
 {
 getline( fo, line );
 outputWeights[i] = atof( line.c_str() );
 getline( fot, line );
 outputTraces[i] = atof( line.c_str() );
 }
 fo.close();
 fot.close();
 
 vector< vector<double> > middleWeights, middleTraces;
 middleWeights.resize(nMiddle);
 middleTraces.resize(nMiddle);
 for( i=0; i<nMiddle; i++ )
 {
 middleWeights[i].resize(98);
 middleTraces[i].resize(98);
 for( j=0; j<98; j++ )
 {
 getline( fm, line );
 middleWeights[i][j] = atof( line.c_str() );
 getline( fmt, line );
 middleTraces[i][j] = atof( line.c_str() );
 }
 }
 fm.close();
 fmt.close();
 
 strategytd s1( outputWeights, middleWeights, outputTraces, middleTraces, 0.5, 0.5, 0. );
 i = 4608;
 s1.learning = true;
 
 game g( &s1, &s1, i+1 );
 g.verbose = true;
 g.stepToEnd();
 
 return 0;
 }
 */


double playSerial( strategytd& s1, long n, long initSeed );

double playSerial( strategytd& s1, long n, long initSeed )
{
    s1.learning = false;
    
    float w0=0;
    float avgSteps=0;
    strategyPubEval s2;
    //strategyRandom s2(1);
    
    for( long i=0; i<n; i++ )
    {
        game g( &s1, &s2, (int)(i+initSeed) );
        g.setTurn( ((int) i)%2 );
        g.stepToEnd();
        if( g.winner() == 0 ) w0 ++;
        avgSteps += g.nSteps;
    }
    
    w0/=n;
    avgSteps/=n;
    
    cout << "Prob of TD winning = " << w0 * 100 << endl;
    cout << "Average steps/game = " << avgSteps << endl;
    cout << endl;
    
    return w0;
}

int main( int argc, const char * argv [] )
{
    // try out the TD learning strategy
    
    strategytd s1;
    double winFrac, maxWinFrac=0;
    vector<double> maxOutputWeights, maxOutputTraces;
    vector< vector<double> > maxMiddleWeights, maxMiddleTraces;
    bool trackWeights=false;
    bool writeWeights=false;
    
    playSerial( s1, 200, 1 );
    
    for( int i=0; i<30000; i++ )
    {
        s1.learning = true;
        
        game g( &s1, &s1, i+1 );
        try 
        {
            g.stepToEnd();
        }
        catch( const string& errMsg )
        {
            cout << "ERROR :" << errMsg << endl;
            return 0;
        }
        catch( exception& e )
        {
            cout << "Exception: " << e.what() << endl;
        }
        catch( ... )
        {
            cout << "Some other kind of error...\n";
            return 0;
        }
        
        if( (i+1)%100 == 0 ) 
        {
            vector<double> ow = s1.getOutputWeights();
            vector< vector<double> > mw = s1.getMiddleWeights();
            vector<double> ot = s1.getOutputTraces();
            vector< vector<double> > mt = s1.getMiddleTraces();
            double ov=0, mv=0, otv=0, mtv=0;
            for( int j=0; j<s1.nMiddle; j++ ) 
            {
                if( j < s1.nMiddle-1 )
                {
                    ov += fabs(ow[j]);
                    otv += fabs(ot[j]);
                }
                for( int k=0; k<98; k++ )
                {
                    mv += fabs(mw[j][k]);
                    mtv += fabs(mt[j][k]);
                }
            }
            ov  /= s1.nMiddle-1;
            mv  /= s1.nMiddle * 98;
            otv /= s1.nMiddle-1;
            mtv /= s1.nMiddle * 98;
            cout << (i+1) << "   " << g.nSteps << " " << ov << "  " << mv << "  " << "   " << otv << "   " << mtv << endl;
            
            if( writeWeights )
            {
                // write the weights and traces to files
                
                ofstream fo( "/Users/mghiggins/weightsOut.txt" );
                ofstream fot( "/Users/mghiggins/tracesOut.txt" );
                ofstream fm( "/Users/mghiggins/weightsMiddle.txt" );
                ofstream fmt( "/Users/mghiggins/tracesMiddle.txt" );
                for( int j=0; j<s1.nMiddle; j++ )
                {
                    if( j < s1.nMiddle - 1 )
                    {
                        fo << ow[j] << endl;
                        fot << ot[j] << endl;
                    }
                    for( int k=0; k<98; k++ )
                    {
                        fm << mw[j][k] << endl;
                        fmt << mt[j][k] << endl;
                    }
                }
                fo.close();
                fot.close();
                fm.close();
                fmt.close();
            }
        }
        
        if( (i+1)%1000 == 0 ) 
        {
            winFrac = playSerial( s1, 200, 1 );   
            if( trackWeights )
            {
                if( winFrac > maxWinFrac )
                {
                    maxWinFrac = winFrac;
                    
                    // remember the weights we used at this point
                    
                    maxOutputWeights = s1.getOutputWeights();
                    maxMiddleWeights = s1.getMiddleWeights();
                    maxOutputTraces  = s1.getOutputTraces();
                    maxMiddleTraces  = s1.getMiddleTraces();
                }
                else if( winFrac < maxWinFrac - 0.1 )
                {
                    // we seem to have gone astray - back it up and reduce alpha/beta a bit
                    
                    s1 = strategytd( maxOutputWeights, maxMiddleWeights, maxOutputTraces, maxMiddleTraces, s1.alpha * 0.8, s1.beta * 0.8, s1.lambda );
                    cout << "Backed up and reduced alpha/beta\n";
                }
            }
        }
    }
    
    return 0;
}

