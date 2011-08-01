//
//  runners.cpp
//  bgtd
//
//  Created by Mark Higgins on 7/30/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//


#include <iostream>
#include <fstream>
#include <cmath>
#include "runners.h"
#include "strategytd.h"
#include "strategypubeval.h"
#include "game.h"

void writeWeightsToFiles( const vector<double>& outputWeights, const vector< vector<double> >& middleWeights, const vector<double>& outputTraces, const vector< vector<double> >& middleTraces, const string& fileSuffix )
{
    // get the dimensions from the vectors
    
    int nMiddle = (int) middleWeights.size();
    
    // write the weights and traces to files
    
    string path    = "/Users/mghiggins";
    string foName  = path + "/weightsOut_" + fileSuffix + ".txt";
    string fotName = path + "/tracesOut_" + fileSuffix + ".txt";
    string fmName  = path + "/weightsMiddle_" + fileSuffix + ".txt";
    string fmtName = path + "/tracesMiddle_" + fileSuffix + ".txt";
    
    ofstream fo( foName.c_str() );
    ofstream fot( fotName.c_str() );
    ofstream fm( fmName.c_str() );
    ofstream fmt( fmtName.c_str() );
    
    // one line at the top of the output weights file that specifies the number of middles.
    
    fo << nMiddle << endl;
    
    for( int j=0; j<nMiddle; j++ )
    {
        if( j < nMiddle - 1 )
        {
            fo  << outputWeights[j] << endl;
            fot << outputTraces[j]  << endl;
        }
        for( int k=0; k<98; k++ )
        {
            fm  << middleWeights[j][k] << endl;
            fmt << middleTraces[j][k]  << endl;
        }
    }
    fo.close();
    fot.close();
    fm.close();
    fmt.close();
}

void readWeightsFromFile( vector<double>& outputWeights, vector< vector<double> >& middleWeights, vector<double>& outputTraces, vector< vector<double> >& middleTraces, const string& fileSuffix )
{
    string path    = "/Users/mghiggins";
    string foName  = path + "/weightsOut_" + fileSuffix + ".txt";
    string fotName = path + "/tracesOut_" + fileSuffix + ".txt";
    string fmName  = path + "/weightsMiddle_" + fileSuffix + ".txt";
    string fmtName = path + "/tracesMiddle_" + fileSuffix + ".txt";
    
    ifstream fo( foName.c_str() );
    ifstream fot( fotName.c_str() );
    ifstream fm( fmName.c_str() );
    ifstream fmt( fmtName.c_str() );

    // start by loading the number of middle weights from the output weights file
    
    string line;
    getline( fo, line );
    int nMiddle = atoi( line.c_str() );
    
    // resize all the vectors appropriately
    
    outputWeights.resize( nMiddle - 1 );
    outputTraces.resize( nMiddle - 1 );
    middleWeights.resize( nMiddle );
    middleTraces.resize( nMiddle );
    
    int i, j;
    
    for( i=0; i<nMiddle; i++ )
    {
        middleWeights[i].resize(98);
        middleTraces[i].resize(98);
    }
    
    // load the output weights and traces
    
    for( i=0; i<nMiddle-1; i++ )
    {
        getline( fo, line );
        outputWeights[i] = atof( line.c_str() );
        getline( fot, line );
        outputTraces[i] = atof( line.c_str() );
    }
    
    // load the middle weights and traces
    
    for( i=0; i<nMiddle; i++ )
        for( j=0; j<98; j++ )
        {
            getline( fm, line );
            middleWeights[i][j] = atof( line.c_str() );
            getline( fmt, line );
            middleTraces[i][j] = atof( line.c_str() );
        }
}

double playSerial( strategytd& s1, long n, long initSeed, long displayIndex, const string& fileSuffix )
{
    s1.learning = false;
    
    float w0=0;
    float p=0, q=0;
    float avgSteps=0;
    float ns=0, ng=0, nb=0;
    int   s;
    strategyPubEval s2;
    
    for( long i=0; i<n; i++ )
    {
        game g( &s1, &s2, (int)(i+initSeed) );
        g.setTurn( ((int) i)%2 );
        g.stepToEnd();
        s = g.winnerScore();
        if( g.winner() == 0 ) 
        {
            w0 ++;
            p += s;
        }
        else
            p -= s;
        q += s;
        if( s == 1 )
            ns ++;
        else if( s == 2 )
            ng ++;
        else
            nb ++;
        
        avgSteps += g.nSteps;
    }
    
    w0/=n;
    avgSteps/=n;
    p/=n;
    q/=n;
    ns/=n;
    ng/=n;
    nb/=n;
    
    cout << "Prob of TD winning = " << w0 * 100 << endl;
    cout << "Average steps/game = " << avgSteps << endl;
    cout << "Average ppg        = " << p << endl;
    cout << "Average abs ppg    = " << q << endl;
    cout << "Frac single        = " << ns << endl;
    cout << "Frac gammon        = " << ng << endl;
    cout << "Frac backgammon    = " << nb << endl;
    cout << endl;
    
    // write out the results to a file
    
    if( fileSuffix != "nowrite" )
    {
        string path  = "/Users/mghiggins";
        string fName = path + "/td_comparisonresults_" + fileSuffix + ".csv";
        ofstream f;
        if( displayIndex == 0 )
            f.open( fName.c_str(), fstream::trunc ); // start a new file
        else
            f.open( fName.c_str(), fstream::app ); // append to an existing one
        
        // write the data
        
        f << displayIndex << "," << w0*100 << "," << avgSteps << "," << p << "," << q << "," << ns << "," << ng << "," << nb << endl;
        
        f.close();
    }
    
    return w0;
}

void sim1( int nMiddle, double alpha0, double beta0, const string& fileSuffix )
{
    // try out the TD learning strategy
    
    strategytd s1( nMiddle );
    double winFrac, maxWinFrac=0;
    long iMax;
    vector<double> maxOutputWeights, maxOutputTraces;
    vector< vector<double> > maxMiddleWeights, maxMiddleTraces;
    bool trackWeights=true;
    bool writeWeights=true;
    double alphaMin=0.025;
    double betaMin=0.025;
    double backstepFrac=0.8;
    
    s1.alpha = alpha0;
    s1.beta  = beta0;
    
    playSerial( s1, 200, 1, 0, "std" + fileSuffix );
    
    for( long i=0; i<200000; i++ )
    {
        s1.learning = true;
        
        game g( &s1, &s1, (int) (i+1) );
        try 
        {
            g.stepToEnd();
        }
        catch( const string& errMsg )
        {
            cout << "ERROR :" << errMsg << endl;
            return;
        }
        catch( exception& e )
        {
            cout << "Exception: " << e.what() << endl;
            return;
        }
        catch( ... )
        {
            cout << "Some other kind of error...\n";
            return;
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
            
            if( writeWeights ) writeWeightsToFiles( ow, mw, ot, mt, "std" + fileSuffix );
        }
        
        if( (i+1)%1000 == 0 ) 
        {
            winFrac = playSerial( s1, 200, 1, i+1, "std" + fileSuffix );   
            if( trackWeights )
            {
                if( winFrac >= maxWinFrac - 1e-5 )
                {
                    maxWinFrac = winFrac;
                    iMax = i;
                    
                    // remember the weights we used at this point
                    
                    maxOutputWeights = s1.getOutputWeights();
                    maxMiddleWeights = s1.getMiddleWeights();
                    maxOutputTraces  = s1.getOutputTraces();
                    maxMiddleTraces  = s1.getMiddleTraces();
                    
                    cout << "*********\n";
                    cout << "Rolling best performance!\n";
                    cout << endl;
                    
                    if( writeWeights ) writeWeightsToFiles( maxOutputWeights, maxMiddleWeights, maxOutputTraces, maxMiddleTraces, "max" + fileSuffix );
                }
                else if( winFrac < maxWinFrac - 0.1 and s1.alpha > alphaMin )
                {
                    // we seem to have gone astray - back it up and reduce alpha/beta a bit
                    
                    double newAlpha = s1.alpha * backstepFrac;
                    double newBeta  = s1.beta  * backstepFrac;
                    if( newAlpha < alphaMin ) newAlpha = alphaMin;
                    if( newBeta  < betaMin ) newBeta  = betaMin;
                    
                    s1 = strategytd( maxOutputWeights, maxMiddleWeights, maxOutputTraces, maxMiddleTraces, newAlpha, newBeta, s1.lambda );
                    cout << "**********\n";
                    cout << "Reduced alpha/beta and backed up to world as it was at step " << iMax+1 << endl;
                    cout << "New alpha = " << newAlpha << endl;
                    cout << "New beta  = " << newBeta << endl;
                    cout << endl;
                }
            }
        }
    }
}

void test1()
{
    // load the weights and traces from the saved files
    
    vector<double> outputWeights, outputTraces;
    vector< vector<double> > middleWeights, middleTraces;
    
    readWeightsFromFile( outputWeights, middleWeights, outputTraces, middleTraces, "max" );
    
    // construct the TD strategy
    
    strategytd s1( outputWeights, middleWeights, outputTraces, middleTraces, 0.32, 0.32, 0 );
    s1.learning = false;
    
    // construct the pubeval opponent
    
    strategyPubEval s2;
    
    // run some games
    
    double w0;
    for( int i=0; i<10; i++ )
    {
        try
        {
            w0 = playSerial( s1, 200, i+1, i+1, "nowrite" );
        }
        catch( string& err )
        {
            cout << "ERROR: " << err << endl;
        }
    }
}
