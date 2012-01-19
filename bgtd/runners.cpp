//
//  runners.cpp
//  bgtd
//
//  Created by Mark Higgins on 7/30/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//


#include <algorithm>
#include <iostream>
#include <fstream>
#include <cmath>
#include <boost/thread.hpp>
#include "bearofffns.h"
#include "runners.h"
#include "strategytd.h"
#include "strategytdexp.h"
#include "strategytdexp2.h"
#include "strategytdexp3.h"
#include "strategytdexp4.h"
#include "strategytdmult.h"
#include "strategytdorig.h"
#include "strategytdorigsym.h"
#include "strategytdoriggam.h"
#include "strategytdorigbg.h"
#include "strategypubeval.h"
#include "strategyply.h"
#include "strategyhuman.h"
#include "game.h"
#include "gamefns.h"

void writeWeightsToFiles( const vector<double>& outputWeights, const vector< vector<double> >& middleWeights, const vector<double>& outputTraces, const vector< vector<double> >& middleTraces, const string& fileSuffix )
{
    // get the dimensions from the vectors
    
    int nMiddle = (int) middleWeights.size();
    
    // write the weights and traces to files
    
    string path    = "/Users/mghiggins/bgtdtres";
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
    string path    = "/Users/mghiggins/bgtdres";
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

void writeExpWeightsToFiles( const vector<double>& outputProbWeights, const vector<double>& outputGammonWeights, 
                            const vector< vector<double> >& middleWeights, 
                            const vector<double>& outputProbTraces, const vector<double>& outputGammonTraces, 
                            const vector< vector<double> >& middleProbTraces, const vector< vector<double> >& middleGammonTraces, 
                            const string& fileSuffix )
{
    // get the dimensions from the vectors
    
    int nMiddle = (int) middleWeights.size();
    
    // write the weights and traces to files
    
    string path     = "/Users/mghiggins/bgtdres";
    string fopName  = path + "/weightsOutProb_" + fileSuffix + ".txt";
    string fogName  = path + "/weightsOutGammon_" + fileSuffix + ".txt";
    string foptName = path + "/tracesOutProb_" + fileSuffix + ".txt";
    string fogtName = path + "/tracesOutGammon_" + fileSuffix + ".txt";
    string fmName   = path + "/weightsMiddle_" + fileSuffix + ".txt";
    string fmptName = path + "/tracesMiddleProb_" + fileSuffix + ".txt";
    string fmgtName = path + "/tracesMiddleGammon_" + fileSuffix + ".txt";
    
    ofstream fop( fopName.c_str() );
    ofstream fog( fogName.c_str() );
    ofstream fopt( foptName.c_str() );
    ofstream fogt( fogtName.c_str() );
    ofstream fm( fmName.c_str() );
    ofstream fmpt( fmptName.c_str() );
    ofstream fmgt( fmgtName.c_str() );
    
    // one line at the top of the prob output weights file that specifies the number of middles.
    
    fop << nMiddle << endl;
    
    for( int j=0; j<nMiddle; j++ )
    {
        if( j < nMiddle - 1 )
        {
            fop  << outputProbWeights[j] << endl;
            fopt << outputProbTraces[j]  << endl;
        }
        fog  << outputGammonWeights[j] << endl;
        fogt << outputGammonTraces[j] << endl;
        for( int k=0; k<98; k++ )
        {
            fm  << middleWeights[j][k] << endl;
            fmpt << middleProbTraces[j][k]  << endl;
            fmgt << middleGammonTraces[j][k]  << endl;
        }
    }
    fog  << outputGammonWeights[nMiddle] << endl;
    fogt << outputGammonTraces[nMiddle]  << endl;
    
    fop.close();
    fog.close();
    fopt.close();
    fogt.close();
    fm.close();
    fmpt.close();
    fmgt.close();
}

void writeExp2WeightsToFiles( const vector<double>& outputProbWeights, const vector<double>& outputGammonWinWeights, const vector<double>& outputGammonLossWeights, 
                             const vector< vector<double> >& middleWeights, const string& fileSuffix )
{
    // get the dimensions from the vectors
    
    int nMiddle = (int) middleWeights.size();
    
    // write the weights and traces to files
    
    string path     = "/Users/mghiggins/bgtdres";
    string fopName  = path + "/weightsOutProb_" + fileSuffix + ".txt";
    string fowName  = path + "/weightsOutGammonWin_" + fileSuffix + ".txt";
    string folName  = path + "/weightsOutGammonLoss_" + fileSuffix + ".txt";
    string fmName   = path + "/weightsMiddle_" + fileSuffix + ".txt";
    
    ofstream fop( fopName.c_str() );
    ofstream fow( fowName.c_str() );
    ofstream fol( folName.c_str() );
    ofstream fm( fmName.c_str() );
    
    // one line at the top of the prob output weights file that specifies the number of middles.
    
    fop << nMiddle << endl;
    
    for( int j=0; j<nMiddle; j++ )
    {
        fop << outputProbWeights[j] << endl;
        fow << outputGammonWinWeights[j] << endl;
        fol << outputGammonLossWeights[j] << endl;
        for( int k=0; k<197; k++ )
            fm  << middleWeights[j][k] << endl;
    }
    fop << outputProbWeights[nMiddle] << endl;
    fow << outputGammonWinWeights[nMiddle] << endl;
    fol << outputGammonLossWeights[nMiddle] << endl;
    
    fop.close();
    fow.close();
    fol.close();
    fm.close();
}

void writeExp3WeightsToFiles( const vector<double>& outputProbWeights, const vector<double>& outputGammonWinWeights, const vector<double>& outputBgWinWeights,
                              const vector< vector<double> >& middleWeights, const string& fileSuffix )
{
    // get the dimensions from the vectors
    
    int nMiddle = (int) middleWeights.size();
    
    // write the weights and traces to files
    
    string path     = "/Users/mghiggins/bgtdres";
    string fopName  = path + "/weightsOutProb_" + fileSuffix + ".txt";
    string fogName  = path + "/weightsOutGammonWin_" + fileSuffix + ".txt";
    string fobName  = path + "/weightsOutBgWin_" + fileSuffix + ".txt";
    string fmName   = path + "/weightsMiddle_" + fileSuffix + ".txt";
    
    ofstream fop( fopName.c_str() );
    ofstream fog( fogName.c_str() );
    ofstream fob( fobName.c_str() );
    ofstream fm( fmName.c_str() );
    
    // one line at the top of the prob output weights file that specifies the number of middles.
    
    fop << nMiddle << endl;
    
    for( int j=0; j<nMiddle; j++ )
    {
        if( j<nMiddle-1 ) fop << outputProbWeights[j] << endl;
        fog << outputGammonWinWeights[j] << endl;
        fop << outputBgWinWeights[j] << endl;
        for( int k=0; k<99; k++ )
            fm  << middleWeights[j][k] << endl;
    }
    fog << outputGammonWinWeights[nMiddle] << endl;
    fob << outputBgWinWeights[nMiddle] << endl;
    
    fop.close();
    fog.close();
    fob.close();
    fm.close();
}

void writeExp4WeightsToFiles( const vector<double>& outputProbWeights, const vector<double>& outputGammonWinWeights, const vector<double>& outputGammonLossWeights,
                              const vector< vector<double> >& middleWeights, const string& fileSuffix )
{
    // get the dimensions from the vectors
    
    int nMiddle = (int) middleWeights.size();
    
    // write the weights and traces to files
    
    string path     = "/Users/mghiggins/bgtdres";
    string fopName  = path + "/weightsOutProb_" + fileSuffix + ".txt";
    string fowName  = path + "/weightsOutGammonWin_" + fileSuffix + ".txt";
    string folName  = path + "/weightsOutGammonLoss_" + fileSuffix + ".txt";
    string fmName   = path + "/weightsMiddle_" + fileSuffix + ".txt";
    
    ofstream fop( fopName.c_str() );
    ofstream fow( fowName.c_str() );
    ofstream fol( folName.c_str() );
    ofstream fm( fmName.c_str() );
    
    // one line at the top of the prob output weights file that specifies the number of middles.
    
    fop << nMiddle << endl;
    
    for( int j=0; j<nMiddle; j++ )
    {
        fop << outputProbWeights[j] << endl;
        fow << outputGammonWinWeights[j] << endl;
        fol << outputGammonLossWeights[j] << endl;
        for( int k=0; k<199; k++ )
            fm  << middleWeights[j][k] << endl;
    }
    fop << outputProbWeights[nMiddle] << endl;
    fow << outputGammonWinWeights[nMiddle] << endl;
    fol << outputGammonLossWeights[nMiddle] << endl;
    
    fop.close();
    fow.close();
    fm.close();
}

void readExpWeightsFromFile( vector<double>& outputProbWeights, vector<double>& outputGammonWeights, 
                            vector< vector<double> >& middleWeights, 
                            vector<double>& outputProbTraces, vector<double>& outputGammonTraces, 
                            vector< vector<double> >& middleProbTraces, vector< vector<double> >& middleGammonTraces, 
                            const string& fileSuffix )
{
    string path     = "/Users/mghiggins/bgtdres";
    string fopName  = path + "/weightsOutProb_" + fileSuffix + ".txt";
    string fogName  = path + "/weightsOutGammon_" + fileSuffix + ".txt";
    string foptName = path + "/tracesOutProb_" + fileSuffix + ".txt";
    string fogtName = path + "/tracesOutGammon_" + fileSuffix + ".txt";
    string fmName   = path + "/weightsMiddle_" + fileSuffix + ".txt";
    string fmptName = path + "/tracesMiddleProb_" + fileSuffix + ".txt";
    string fmgtName = path + "/tracesMiddleGammon_" + fileSuffix + ".txt";
    
    ifstream fop( fopName.c_str() );
    ifstream fog( fogName.c_str() );
    ifstream fopt( foptName.c_str() );
    ifstream fogt( fogtName.c_str() );
    ifstream fm( fmName.c_str() );
    ifstream fmpt( fmptName.c_str() );
    ifstream fmgt( fmgtName.c_str() );
    
    // start by loading the number of middle weights from the prob output weights file
    
    string line;
    getline( fop, line );
    int nMiddle = atoi( line.c_str() );
    
    // resize all the vectors appropriately
    
    outputProbWeights.resize( nMiddle - 1 );
    outputGammonWeights.resize( nMiddle + 1 );
    outputProbTraces.resize( nMiddle - 1 );
    outputGammonTraces.resize( nMiddle + 1 );
    middleWeights.resize( nMiddle );
    middleProbTraces.resize( nMiddle );
    middleGammonTraces.resize( nMiddle );
    
    int i, j;
    
    for( i=0; i<nMiddle; i++ )
    {
        middleWeights[i].resize(98);
        middleProbTraces[i].resize(98);
        middleGammonTraces[i].resize(98);
    }
    
    // load the output weights and traces for the prob and gammon nodes
    
    for( i=0; i<nMiddle-1; i++ )
    {
        getline( fop, line );
        outputProbWeights[i] = atof( line.c_str() );
        getline( fopt, line );
        outputProbTraces[i] = atof( line.c_str() );
    }
    
    for( i=0; i<nMiddle+1; i++ )
    {
        getline( fog, line );
        outputGammonWeights[i] = atof( line.c_str() );
        getline( fogt, line );
        outputGammonTraces[i] = atof( line.c_str() );
    }
    
    // load the middle weights and traces
    
    for( i=0; i<nMiddle; i++ )
        for( j=0; j<98; j++ )
        {
            getline( fm, line );
            middleWeights[i][j] = atof( line.c_str() );
            getline( fmpt, line );
            middleProbTraces[i][j] = atof( line.c_str() );
            getline( fmgt, line );
            middleGammonTraces[i][j] = atof( line.c_str() );
        }
}

vector<int> points;
vector<int> steps;

class worker
{
public:
    worker( long i, strategytdbase& s1, strategy& s2, long n, long initSeed ) : i(i), s1(s1), s2(s2), n(n), initSeed(initSeed) {};
    
    void operator()()
    {
        game g( &s1, &s2, (int)(i+initSeed) );
        g.setTurn( ((int) i)%2 );
        g.stepToEnd();
        double s = g.winnerScore();
        if( g.winner() == 0 )
            points[i] = s;
        else
            points[i] = -s;
        steps[i] = g.nSteps;
    }
    
private:
    long i;
    strategytdbase& s1;
    strategy& s2;
    long n;
    long initSeed;
};

double playParallel( strategytdbase& s1, strategy& s2, long n, long initSeed, long displayIndex, const string& fileSuffix )
{
    using namespace boost;
    
    s1.learning = false;
    
    // run each game in its own thread
    
    if( points.size() < n ) 
    {
        points.resize(n);
        steps.resize(n);
    }
    
    thread_group ts;
    for( long i=0; i<n; i++ ) ts.create_thread( worker( i, s1, s2, n, initSeed ) );
    ts.join_all();
    
    double ppg=0, w0=0, q=0, avgSteps=0, ns=0, ng=0, nb=0;
    int p, ap;
    for( long i=0; i<n; i++ ) 
    {
        p  = points[i];
        ap = abs( p );
     
        ppg += p;
        q += ap;
        if( p > 0 ) w0 += 1;
        if( ap == 1 ) ns += 1;
        if( ap == 2 ) ng += 1;
        if( ap == 3 ) nb += 1;
        avgSteps += steps[i];
    }
    ppg /= n;
    w0  /= n;
    q   /= n;
    ns  /= n;
    ng  /= n;
    nb  /= n;
    avgSteps /= n;
    
    cout << "Average ppg        = " << ppg << endl;
    cout << "Prob of P1 winning = " << w0 * 100 << endl;
    cout << "Average abs ppg    = " << q << endl;
    cout << "Frac single        = " << ns << endl;
    cout << "Frac gammon        = " << ng << endl;
    cout << "Frac backgammon    = " << nb << endl;
    cout << "Average steps/game = " << avgSteps << endl;
    cout << endl;
    
    // write out the results to a file
    
    if( fileSuffix != "nowrite" )
    {
        string path  = "/Users/mghiggins/bgtdres";
        string fName = path + "/td_comparisonresults_" + fileSuffix + ".csv";
        ofstream f;
        if( displayIndex == 0 )
            f.open( fName.c_str(), fstream::trunc ); // start a new file
        else
            f.open( fName.c_str(), fstream::app ); // append to an existing one
        
        // write the data
        
        f << displayIndex << "," << w0*100 << "," << avgSteps << "," << ppg << "," << q << "," << ns << "," << ng << "," << nb << endl;
        
        f.close();
    }
    
    return ppg;
}

double playSerial( strategytdbase& s1, strategy& s2, long n, long initSeed, long displayIndex, const string& fileSuffix, bool returnPpg )
{
    s1.learning = false;
    
    float w0=0;
    float p=0, q=0;
    float avgSteps=0;
    float ns=0, ng=0, nb=0;
    int   s;
    
    for( long i=0; i<n; i++ )
    {
        if( (i+1)%1 == 0 ) cout << "Run " << i+1 << endl;
        
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
    
    cout << "Average ppg        = " << p << endl;
    cout << "Prob of P1 winning = " << w0 * 100 << endl;
    cout << "Average abs ppg    = " << q << endl;
    cout << "Frac single        = " << ns << endl;
    cout << "Frac gammon        = " << ng << endl;
    cout << "Frac backgammon    = " << nb << endl;
    cout << "Average steps/game = " << avgSteps << endl;
    cout << endl;
    
    // write out the results to a file
    
    if( fileSuffix != "nowrite" )
    {
        string path  = "/Users/mghiggins/bgtdres";
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
    
    if( returnPpg )
        return p;
    else
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
    bool trackWeights=false;
    bool writeWeights=true;
    double alphaMin=0.0025;
    double betaMin=0.0025;
    double backstepFrac=0.8;
    
    strategyPubEval s2;
    
    s1.alpha = alpha0;
    s1.beta  = beta0;
    
    //playSerial( s1, s2, 200, 1, 0, "std" + fileSuffix );
    playParallel( s1, s2, 200, 1, 0, "std" + fileSuffix );
    
    for( long i=0; i<2000000; i++ )
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
            winFrac = playSerial( s1, s2, 200, 1, i+1, "std" + fileSuffix );   
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

void sim2( int nMiddle, double alpha0, double beta0, const string& fileSuffix )
{
    // try out the expanded TD learning strategy, which includes nodes for probability of win and gammon, instead of
    // just probability of win.
    
    strategytdexp s1( nMiddle );
    /*
    vector<double> outputProbWeights, outputGammonWeights, outputProbTraces, outputGammonTraces;
    vector< vector<double> > middleExpWeights, middleProbTraces, middleGammonTraces;
    
    readExpWeightsFromFile( outputProbWeights, outputGammonWeights, middleExpWeights, outputProbTraces, outputGammonTraces, middleProbTraces, middleGammonTraces, "exp_std" + fileSuffix );
    strategytdexp s1( outputProbWeights, outputGammonWeights, middleExpWeights, outputProbTraces, outputGammonTraces, middleProbTraces, middleGammonTraces, 0.1, 0.1, 0 );
    */
    
    double winFrac, maxWinFrac=0;
    long iMax;
    vector<double> maxOutputProbWeights, maxOutputGammonWeights, maxOutputProbTraces, maxOutputGammonTraces;
    vector< vector<double> > maxMiddleWeights, maxMiddleProbTraces, maxMiddleGammonTraces;
    bool trackWeights=false;
    bool writeWeights=true;
    double alphaMin=0.0025;
    double betaMin=0.0025;
    double backstepFrac=0.8;
    
    strategyPubEval s2;
    
    s1.alpha = alpha0;
    s1.beta  = beta0;
    
    playSerial( s1, s2, 200, 1, 0, "exp_std" + fileSuffix );
    
    for( long i=0; i<2000000; i++ )
    {
        s1.learning = true;
        
        game g( &s1, &s1, (int) (i+1) );
        g.setTurn( (int) i%2 );
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
            vector<double> opw = s1.getOutputProbWeights();
            vector<double> ogw = s1.getOutputGammonWeights();
            vector< vector<double> > mw = s1.getMiddleWeights();
            double opv=0, ogv=0, mv=0;
            for( int j=0; j<s1.nMiddle; j++ ) 
            {
                if( j < s1.nMiddle-1 )
                    opv  += fabs(opw[j]);
                ogv  += fabs(ogw[j]);
                for( int k=0; k<98; k++ )
                    mv += fabs(mw[j][k]);
            }
            ogv += fabs(ogw[nMiddle]);
            
            opv  /= s1.nMiddle-1;
            ogv  /= s1.nMiddle+1;
            mv   /= s1.nMiddle * 98;
            cout << (i+1) << "   " << g.nSteps << " " << opv << "  " << ogv << "  " << mv << endl;
            
            if( writeWeights ) writeExpWeightsToFiles( opw, ogw, mw, s1.getOutputProbTraces(), s1.getOutputGammonTraces(), s1.getMiddleProbTraces(), s1.getMiddleGammonTraces(), "exp_std" + fileSuffix );
        }
        
        if( (i+1)%1000 == 0 ) 
        {
            winFrac = playSerial( s1, s2, 200, 1, i+1, "exp_std" + fileSuffix );   
            if( winFrac >= maxWinFrac - 1e-5 )
            {
                maxWinFrac = winFrac;
                iMax = i;
                
                // remember the weights we used at this point
                
                maxOutputProbWeights   = s1.getOutputProbWeights();
                maxOutputGammonWeights = s1.getOutputGammonWeights();
                maxMiddleWeights       = s1.getMiddleWeights();
                maxOutputProbTraces    = s1.getOutputProbTraces();
                maxOutputGammonTraces  = s1.getOutputGammonTraces();
                maxMiddleProbTraces    = s1.getMiddleProbTraces();
                maxMiddleGammonTraces  = s1.getMiddleGammonTraces();
                
                cout << "*********\n";
                cout << "Rolling best performance!\n";
                cout << endl;
                
                if( writeWeights ) writeExpWeightsToFiles( maxOutputProbWeights, maxOutputGammonWeights, maxMiddleWeights, 
                                                           maxOutputProbTraces, maxOutputGammonTraces,
                                                           maxMiddleProbTraces, maxMiddleGammonTraces,
                                                           "exp_max" + fileSuffix );
            }
            
            if( trackWeights && winFrac < maxWinFrac - 0.1 && s1.alpha > alphaMin )
            {
                // we seem to have gone astray - back it up and reduce alpha/beta a bit
                
                double newAlpha = s1.alpha * backstepFrac;
                double newBeta  = s1.beta  * backstepFrac;
                if( newAlpha < alphaMin ) newAlpha = alphaMin;
                if( newBeta  < betaMin ) newBeta  = betaMin;
                
                s1 = strategytdexp( maxOutputProbWeights, maxOutputGammonWeights, maxMiddleWeights, 
                                    maxOutputProbTraces, maxOutputGammonTraces, 
                                    maxMiddleProbTraces, maxMiddleGammonTraces, 
                                    newAlpha, newBeta, s1.lambda );
                cout << "**********\n";
                cout << "Reduced alpha/beta and backed up to world as it was at step " << iMax+1 << endl;
                cout << "New alpha = " << newAlpha << endl;
                cout << "New beta  = " << newBeta << endl;
                cout << endl;
            }
        }
    }
}

void sim3( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix )
{
    // try out the expanded TD learning strategy, which includes nodes for probability of win and gammon, instead of
    // just probability of win.
    
    strategytdexp2 s1( "benchmark", "exp_max" + srcSuffix, true );
    strategytdexp s2( "benchmark", "exp_max" + srcSuffix ); // opponent
    s2.learning = false;
    
    s1.alpha = alpha0;
    s1.beta  = beta0;
    
    double maxPpg = -100;
    
    playSerial( s1, s2, 200, 1, 0, "exp2_std" + fileSuffix, true );
    
    for( long i=0; i<2000000; i++ )
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
            vector<double> opw = s1.getOutputProbWeights();
            vector<double> ogw = s1.getOutputGammonWinWeights();
            vector< vector<double> > mw = s1.getMiddleWeights();
            double opv=0, ogv=0, mv=0;
            for( int j=0; j<s1.nMiddle; j++ ) 
            {
                opv  += fabs(opw[j]);
                ogv  += fabs(ogw[j]);
                for( int k=0; k<197; k++ )
                    mv += fabs(mw[j][k]);
            }
            opv += fabs(opw[nMiddle]);
            ogv += fabs(ogw[nMiddle]);
            
            opv  /= s1.nMiddle+1;
            ogv  /= s1.nMiddle+1;
            mv   /= s1.nMiddle * 197;
            cout << (i+1) << "   " << g.nSteps << " " << opv << "  " << ogv << "  " << mv << endl;
            
            writeExp2WeightsToFiles( s1.getOutputProbWeights(), s1.getOutputGammonWinWeights(), s1.getOutputGammonLossWeights(), s1.getMiddleWeights(), "exp2_std" + fileSuffix );
        }
        
        if( (i+1)%1000 == 0 )
        {
            double ppg = playSerial( s1, s2, 200, 1, i+1, "exp2_std" + fileSuffix, true );
            if( ppg > maxPpg )
            {
                cout << "***** Rolling best ppg = " << ppg << " vs previous max " << maxPpg << "*****\n";
                maxPpg = ppg;
                writeExp2WeightsToFiles( s1.getOutputProbWeights(), s1.getOutputGammonWinWeights(), s1.getOutputGammonLossWeights(), s1.getMiddleWeights(), "exp2_max" + fileSuffix );
            }
            board b;
            vector<double> mids( s1.getMiddleValues( s1.getInputValues( b, true ) ) );
            double pw = s1.getOutputProbValue( mids );
            cout << "Prob of win = " << pw << endl;
            cout << "Prob of gammon win = " << pw * s1.getOutputGammonWinValue( mids, b ) << endl;
            cout << "Prob of gammon loss = " << ( 1 - pw ) * s1.getOutputGammonLossValue( mids, b ) << endl;
            cout << endl;
        }
    }
}

void sim4( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix )
{
    // try out the expanded TD learning strategy, which includes nodes for probability of win and gammon, instead of
    // just probability of win.
    
    strategytdexp3 s1( "benchmark", "exp_max" + srcSuffix, true );
    //strategytdexp3 s1( nMiddle );
    strategytdexp s2( "benchmark", "exp_max" + srcSuffix ); // opponent
    //strategytdexp s2( "benchmark", "exp_maxexp_80_0.1_0.1" ); // opponent
    s2.learning = false;
    //strategyPubEval s2;
    
    s1.alpha = alpha0;
    s1.beta  = beta0;
    
    double maxPpg = -100;
    
    playParallel( s1, s2, 500, 1, 0, "exp3_std" + fileSuffix );
    playParallel( s1, s1, 500, 1, 0, "nowrite" );
    
    board b;
    vector<double> mids( s1.getMiddleValues( s1.getInputValues( b, true ) ) );
    double pw  = s1.getOutputProbValue( mids );
    double pwg = s1.getOutputGammonWinValue( mids );
    double pwb = s1.getOutputBackgammonWinValue( mids );
    double plg = s1.getOutputGammonLossValue( mids );
    double plb = s1.getOutputBackgammonLossValue( mids );
    if( !s1.useBg )
        pwb = plb = 0;
    cout << "Network probs       = " << pw << "   " << pwg << "   " << pwb << endl;
    cout << "Prob of win         = " << pw << endl;
    cout << "Prob of gammon win  = " << pw * pwg * ( 1 - pwb ) << endl;
    cout << "Prob of gammon loss = " << ( 1 - pw ) * plg * ( 1 - plb ) << endl;
    cout << "Prob of bg win      = " << pw * pwg * pwb << endl;
    cout << "Prob of bg loss     = " << ( 1 - pw ) * plg * plb << endl;
    cout << endl;

    int nw=0, ng=0, nb=0;
    
    for( long i=0; i<2000000; i++ )
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
        
        if( g.winner() == 0 ) nw += 1;
        if( g.winnerScore() == 2 ) ng += 1;
        if( g.winnerScore() == 3 ) nb += 1;
        
        if( (i+1)%100 == 0 ) 
        {
            double fw = nw/100.;
            double fg = ng/100.;
            double fb = nb/100.;
            nw = 0;
            ng = 0;
            nb = 0;
            
            vector<double> opw = s1.getOutputProbWeights();
            vector<double> ogw = s1.getOutputGammonWinWeights();
            vector< vector<double> > mw = s1.getMiddleWeights();
            double opv=0, ogv=0, mv=0;
            for( int j=0; j<s1.nMiddle; j++ ) 
            {
                if( j<s1.nMiddle-1 ) opv  += fabs(opw.at(j));
                ogv  += fabs(ogw.at(j));
                for( int k=0; k<99; k++ )
                    mv += fabs(mw.at(j).at(k));
            }
            ogv += fabs(ogw.at(nMiddle));
            
            opv  /= s1.nMiddle-1;
            ogv  /= s1.nMiddle+1;
            mv   /= s1.nMiddle * 99;
            cout << (i+1) << "   " << fw << "   " << fg << "   " << fb << "   " << g.nSteps << " " << opv << "  " << ogv << "  " << mv << endl;
            
            writeExp3WeightsToFiles( s1.getOutputProbWeights(), s1.getOutputGammonWinWeights(), s1.getOutputBackgammonWinWeights(), s1.getMiddleWeights(), "exp3_std" + fileSuffix );
        }
        
        if( (i+1)%1000 == 0 )
        {
            double ppg = playParallel( s1, s2, 500, 1, i+1, "exp3_std" + fileSuffix );
            if( ppg > maxPpg )
            {
                cout << "***** Rolling best ppg = " << ppg << " vs previous max " << maxPpg << "*****\n";
                maxPpg = ppg;
                writeExp3WeightsToFiles( s1.getOutputProbWeights(), s1.getOutputGammonWinWeights(), s1.getOutputBackgammonWinWeights(), s1.getMiddleWeights(), "exp3_max" + fileSuffix );
            }
            playParallel( s1, s1, 500, 1, i+1, "nowrite" );
            board b;
            vector<double> mids( s1.getMiddleValues( s1.getInputValues( b, true ) ) );
            pw  = s1.getOutputProbValue( mids );
            pwg = s1.getOutputGammonWinValue( mids );
            pwb = s1.getOutputBackgammonWinValue( mids );
            plg = s1.getOutputGammonLossValue( mids );
            plb = s1.getOutputBackgammonLossValue( mids );
            if( !s1.useBg )
                pwb = plb = 0;
            cout << "Network probs       = " << pw << "   " << pwg << "   " << pwb << endl;
            cout << "Prob of win         = " << pw << endl;
            cout << "Prob of gammon win  = " << pw * pwg * ( 1 - pwb ) << endl;
            cout << "Prob of gammon loss = " << ( 1 - pw ) * plg * ( 1 - plb ) << endl;
            cout << "Prob of bg win      = " << pw * pwg * pwb << endl;
            cout << "Prob of bg loss     = " << ( 1 - pw ) * plg * plb << endl;
            cout << endl;
        }
    }
}

void sim5( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix )
{
    // try out the expanded TD learning strategy, which includes nodes for probability of win and gammon, instead of
    // just probability of win.
    
    //strategytdexp4 s1( "benchmark", "exp2_max" + srcSuffix, true );
    strategytdexp4 s1( nMiddle );
    strategytdexp2 s2( "benchmark", "exp2_max" + srcSuffix, false ); // opponent
    s2.learning = false;
    //strategyPubEval s2;
    
    s1.alpha = alpha0;
    s1.beta  = beta0;
    
    double maxPpg = -100;
    
    playSerial( s1, s2, 200, 1, 0, "exp3_std" + fileSuffix, true );
    
    board b;
    vector<double> mids( s1.getMiddleValues( s1.getInputValues( b, 0 ) ) );
    double pw = s1.getOutputProbValue( mids );
    double gw = s1.getOutputGammonWinValue( mids, b );
    double gl = s1.getOutputGammonLossValue( mids, b );
    double bw = s1.getOutputBackgammonWinValue( mids, b );
    double bl = s1.getOutputBackgammonLossValue( mids, b );
    cout << "Prob of win         = " << pw << endl;
    cout << "Prob of gammon win  = " << pw * gw << endl;
    cout << "Prob of gammon loss = " << ( 1 - pw ) * gl << endl;
    cout << "Prob of bg win      = " << pw * gw * bw << endl;
    cout << "Prob of bg loss     = " << ( 1 - pw ) * gl * bl << endl;
    cout << endl;
    
    int nw=0, ng=0, nb=0;
    
    for( long i=0; i<2000000; i++ )
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
        
        if( g.winner() == 0 ) nw += 1;
        if( g.winnerScore() == 2 ) ng += 1;
        if( g.winnerScore() == 3 ) nb += 1;
        
        if( (i+1)%100 == 0 ) 
        {
            double fw = nw/100.;
            double fg = ng/100.;
            double fb = nb/100.;
            nw = 0;
            ng = 0;
            nb = 0;
            
            vector<double> opw = s1.getOutputProbWeights();
            vector<double> ogw = s1.getOutputGammonWinWeights();
            vector< vector<double> > mw = s1.getMiddleWeights();
            double opv=0, ogv=0, mv=0;
            for( int j=0; j<s1.nMiddle; j++ ) 
            {
                opv  += fabs(opw.at(j));
                ogv  += fabs(ogw.at(j));
                for( int k=0; k<199; k++ )
                    mv += fabs(mw.at(j).at(k));
            }
            opv += fabs(opw.at(nMiddle));
            ogv += fabs(ogw.at(nMiddle));
            
            opv  /= s1.nMiddle+1;
            ogv  /= s1.nMiddle+1;
            mv   /= s1.nMiddle * 199;
            cout << (i+1) << "   " << fw << "   " << fg << "    " << fb << "    " << g.nSteps << " " << opv << "  " << ogv << "  " << mv << endl;
            
            writeExp4WeightsToFiles( s1.getOutputProbWeights(), s1.getOutputGammonWinWeights(), s1.getOutputGammonLossWeights(), s1.getMiddleWeights(), "exp4_std" + fileSuffix );
        }
        
        if( (i+1)%1000 == 0 )
        {
            double ppg = playSerial( s1, s2, 200, 1, i+1, "exp3_std" + fileSuffix, true );
            if( ppg > maxPpg )
            {
                cout << "***** Rolling best ppg = " << ppg << " vs previous max " << maxPpg << "*****\n";
                maxPpg = ppg;
                writeExp4WeightsToFiles( s1.getOutputProbWeights(), s1.getOutputGammonWinWeights(), s1.getOutputGammonLossWeights(), s1.getMiddleWeights(), "exp4_max" + fileSuffix );
            }
            board b;
            vector<double> mids( s1.getMiddleValues( s1.getInputValues( b, 0 ) ) );
            double pw = s1.getOutputProbValue( mids );
            double gw = s1.getOutputGammonWinValue( mids, b );
            double gl = s1.getOutputGammonLossValue( mids, b );
            double bw = s1.getOutputBackgammonWinValue( mids, b );
            double bl = s1.getOutputBackgammonLossValue( mids, b );
            cout << "Prob of win         = " << pw << endl;
            cout << "Prob of gammon win  = " << pw * gw << endl;
            cout << "Prob of gammon loss = " << ( 1 - pw ) * gl << endl;
            cout << "Prob of bg win      = " << pw * gw * bw << endl;
            cout << "Prob of bg loss     = " << ( 1 - pw ) * gl * bl << endl;
            cout << endl;
        }
    }
}

board referenceBoard( int index );

board referenceBoard( int index )
{
    if( index == 0 )
        return board(); // starting position
    else if( index == 1 )
    {
        board bw; // board where the player will most likely win but no gammon
        bw.setBornIn( 10 );
        bw.setOtherBornIn( 7 );
        for( int pos=0; pos<24; pos++ )
        {
            bw.setChecker( pos, 0 );
            bw.setOtherChecker( pos, 0 );
        }
        bw.setChecker( 0, 2 );
        bw.setChecker( 4, 3 );
        bw.setOtherChecker( 23, 4 );
        bw.setOtherChecker( 22, 3 );
        bw.setOtherChecker( 18, 1 );
        return bw;
    }
    else if( index == 2 )
    {
        board bl; // board where the player will most likely lose but no gammon
        bl.setBornIn( 0 );
        bl.setOtherBornIn( 11 );
        for( int pos=0; pos<24; pos++ )
        {
            bl.setChecker( pos, 0 );
            bl.setOtherChecker( pos, 0 );
        }
        bl.setChecker( 0, 2 );
        bl.setChecker( 3, 3 );
        bl.setChecker( 4, 3 );
        bl.setChecker( 5, 4 );
        bl.setChecker( 7, 1 );
        bl.setChecker( 8, 2 );
        bl.setOtherChecker( 23, 2 );
        bl.setOtherChecker( 21, 2 );
        return bl;
    }
    else if( index == 3 )
    {
        board bg; // board where player will win & gammon, probably, but not backgammon
        bg.setBornIn( 10 );
        for( int pos=0; pos<24; pos++ )
        {
            bg.setChecker( pos, 0 );
            bg.setOtherChecker( pos, 0 );
        }
        bg.setChecker( 0, 2 );
        bg.setChecker( 5, 3 );
        bg.setOtherChecker( 23, 6 );
        bg.setOtherChecker( 20, 5 );
        bg.setOtherChecker( 13, 2 );
        bg.setOtherChecker( 12, 2 );
        return bg;
    }
    else if( index == 4 )
    {
        board bb; // board where player will win & probably backgammon
        bb.setBornIn( 13 );
        for( int pos=0; pos<24; pos++ )
        {
            bb.setChecker( pos, 0 );
            bb.setOtherChecker( pos, 0 );
        }
        bb.setChecker( 3, 2 );
        bb.setOtherChecker( 23, 6 );
        bb.setOtherChecker( 20, 5 );
        bb.setOtherChecker( 13, 2 );
        bb.setOtherChecker( 4, 2 );
        return bb;
    }
    else if( index == 5 )
    {
        board bgl; // board where player will probably lose a gammon, but not backgammon
        bgl.setOtherBornIn( 12 );
        for( int pos=0; pos<24; pos++ )
        {
            bgl.setChecker( pos, 0 );
            bgl.setOtherChecker( pos, 0 );
        }
        bgl.setOtherChecker( 23, 2 );
        bgl.setOtherChecker( 22, 1 );
        bgl.setChecker( 1, 6 );
        bgl.setChecker( 4, 5 );
        bgl.setChecker( 13, 2 );
        bgl.setChecker( 12, 2 );
        return bgl;
    }
    else if( index == 6 )
    {
        board bbl; // board where player will probably lose a backgammon
        bbl.setOtherBornIn( 13 );
        for( int pos=0; pos<24; pos++ )
        {
            bbl.setChecker( pos, 0 );
            bbl.setOtherChecker( pos, 0 );
        }
        bbl.setOtherChecker( 21, 2 );
        bbl.setChecker( 1, 6 );
        bbl.setChecker( 4, 5 );
        bbl.setChecker( 13, 2 );
        bbl.setChecker( 23, 2 );
        return bbl;
    }
    else if( index == 7 )
    {
        board b;
        for( int pos=0; pos<24; pos++ )
        {
            b.setChecker( pos, 0 );
            b.setOtherChecker( pos, 0 );
        }
        for( int pos=2; pos<6; pos++ ) b.setChecker( pos, 2 );
        b.setChecker( 12, 5 );
        b.setChecker( 19, 1 );
        b.setChecker( 22, 1 );
        b.setOtherChecker( 0, 2 );
        b.setOtherChecker( 11, 5 );
        b.setOtherChecker( 15, 1 );
        b.setOtherChecker( 16, 2 );
        b.setOtherChecker( 18, 3 );
        b.setOtherChecker( 20, 2 );
        b.setOtherChecker( 20, 2 );
        return b;
    }
    
    throw "invalid index";
}

void dispBoard( int ind, bool flipped, const strategytdmult& s, const board& b );
void dispBoards( const strategytdmult& s );

void dispBoard( int ind, bool flipped, const strategytdmult& s, const board& b )
{
    string eval( s.evaluator( b ) );
    string netName;
    if( eval == "bearoff" )
        netName = "race";
    else
        netName = eval;
    
    vector<double> mids = s.getMiddleValues( s.getInputValues( b, netName ), netName );
    double pw  = s.getOutputProbValue( mids, netName );
    double pg  = s.getOutputGammonValue( mids, netName );
    double pgl = s.getOutputGammonLossValue( mids, netName );
    double pb  = s.getOutputBackgammonValue( mids, netName );
    double pbl = s.getOutputBackgammonLossValue( mids, netName );
    
    cout << "Reference board " << ind;
    if( flipped ) 
        cout << "*";
    else
        cout << " ";
    cout << ": " << pw << "; ( " << pg << ", " << pgl << " ); ( " << pb << ", " << pbl << " )";
    if( eval == "bearoff" )
    {
        double bpw = s.bearoffProbabilityWin( b );
        double bpg = s.bearoffProbabilityGammon( b );
        double bpgl = s.bearoffProbabilityGammonLoss( b );
        cout << " --> bearoff (pwin,pgamwin,pgamloss) = ( " << bpw << ", " << bpg << ", " << bpgl << " )";
    }
    cout << endl;
}

void dispBoards( const strategytdmult& s )
{
    for( int ind=0; ind<7; ind++ ) 
    {
        board b( referenceBoard(ind) );
        board fb( b );
        fb.setPerspective( ( b.perspective() + 1 ) % 2 );
        dispBoard( ind, false, s, b );
        dispBoard( ind, true, s, fb );
    }
    cout << endl;
}

void sim6( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix )
{
    // print out the reference boards
    
    for( int ind=0; ind<7; ind++ )
    {
        cout << "Reference board " << ind << endl;
        board b( referenceBoard( ind ) );
        b.print();
        cout << endl;
    }
    
    // try out the TD strategy with multiple networks
    
    strategytdmult s1( "benchmark", "mult_stdmult_80_0.1_0.1"  );
    //strategytdmult s1( nMiddle );
    strategytdoriggam s2( "benchmark", "gam_stdgam_80_0.1_0.1" );
    s2.learning = false;
    //strategyPubEval s2;
    
    s1.alpha = alpha0;
    s1.beta  = beta0;
    
    /*
    board rb( referenceBoard( 2 ) );
    rb.print();
    set<board> moves( possibleMoves( rb, 5, 5 ) );
    for( set<board>::iterator it=moves.begin(); it!=moves.end(); it++ )
    {
        cout << "Possible board:\n";
        it->print();
        board flippedBoard( (*it) );
        flippedBoard.setPerspective( 1 - it->perspective() );
        cout << "Calculated equity = " << -s1.bearoffValue( flippedBoard ) << endl;
        cout << "Prob of loss      = " << s1.bearoffProbabilityWin( flippedBoard ) << endl;
        cout << "Prob of gammon    = " << s1.bearoffProbabilityGammon( flippedBoard ) << endl;
        cout << endl;
    }
    rb = s1.preferredBoard( rb, moves );
    cout << "Preferred board:\n";
    rb.print();
    
    return;
    */
    
    double maxPpg = -100;
    long maxInd=-1;
    
    playParallel( s1, s2, 400, 1, 0, "mult_std" + fileSuffix );
    dispBoards( s1 );
    
    int nw=0, ng=0, nb=0, ns=0;
    
    for( long i=0; i<20000000; i++ )
    {
        if( i==200000 or i==1000000 or i==10000000 )
        {
            s1.alpha *= 1/5.;
            s1.beta  *= 1/5.;
            cout << "\n\n***** Dropped alpha and beta by a factor of 5*****\n";
            cout << "alpha = " << s1.alpha << endl;
            cout << "beta  = " << s1.beta  << endl;
            cout << endl;
        }
        s1.learning = true;
        
        game g( &s1, &s1, (int) (i+1) );
        g.setTurn( (int) i%2 );
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
        
        if( g.winner() == 0 ) nw += 1;
        if( g.winnerScore() == 2 ) ng += 1;
        if( g.winnerScore() == 3 ) nb += 1;
        ns += g.nSteps;
        
        if( (i+1)%100 == 0 ) 
        {
            double fw = nw/100.;
            double fg = ng/100.;
            double fb = nb/100.;
            double as = ns/100.;
            nw = 0;
            ng = 0;
            nb = 0;
            ns = 0;
            
            cout << (i+1) << "   " << fw << "   " << fg << "   " << fb << "   " << as << endl;
            
            s1.writeWeights( "mult_std" + fileSuffix );
        }
        
        if( (i+1)%1000 == 0 )
        {
            cout << endl;
            double ppg = playParallel( s1, s2, 400, 1, i+1, "mult_std" + fileSuffix );
            if( ppg > maxPpg )
            {
                cout << "***** Rolling best ppg = " << ppg << " vs previous max " << maxPpg << "*****\n";
                maxPpg = ppg;
                maxInd = i;
                s1.writeWeights( "mult_max" + fileSuffix );
            }
            else
                cout << "Prev best " << maxPpg << " at index " << maxInd+1 << endl;
            cout << endl;
            
            dispBoards( s1 );
        }
    }
}

void sim7( int nMiddle, double alpha0, double beta0 )
{
    // compare simple networks: one that includes the symmetry on perspective flipping, and one 
    // that doesn't.
    
    strategytdorig s1( nMiddle );
    strategytdorigsym s1s( nMiddle );
    strategytdexp2 s2( "benchmark", "exp2_maxexp_80_0.1_0.1", false ); // opponent
    s2.learning = false;
    //strategyPubEval s2;
    
    s1.alpha = alpha0;
    s1.beta  = beta0;
    
    // define some reference boards for checking sensibleness of network probability predictions
    
    board bs; // starting board
    cout << "Starting board:\n";
    bs.print();
    cout << endl;
    board bw; // board where the player will most likely win
    bw.setBornIn( 10 );
    bw.setOtherBornIn( 0 );
    for( int pos=0; pos<24; pos++ )
    {
        bw.setChecker( pos, 0 );
        bw.setOtherChecker( pos, 0 );
    }
    bw.setChecker( 0, 2 );
    bw.setChecker( 4, 3 );
    bw.setOtherChecker( 23, 7 );
    bw.setOtherChecker( 22, 4 );
    bw.setOtherChecker( 19, 2 );
    bw.setOtherChecker( 17, 2 );
    cout << "Winning board:\n";
    bw.print();
    cout << endl;
    board bl; // board where the player will most likely lose
    bl.setBornIn( 2 );
    bl.setOtherBornIn( 9 );
    for( int pos=0; pos<24; pos++ )
    {
        bl.setChecker( pos, 0 );
        bl.setOtherChecker( pos, 0 );
    }
    bl.setChecker( 0, 2 );
    bl.setChecker( 3, 3 );
    bl.setChecker( 4, 6 );
    bl.setChecker( 5, 2 );
    bl.setOtherChecker( 23, 4 );
    bl.setOtherChecker( 22, 2 );
    cout << "Losing board:\n";
    bl.print();
    cout << endl;
    
    // make sure that the symmetry relationship holds for that strategy
    
    double prob1 = s1s.getOutput( s1s.getMiddleValues( s1s.getInputValues( bw, true ) ) );
    cout << "sym board prob for winning board = " << prob1 << endl;
    board bwf( bw );
    bwf.setPerspective( 1 );
    double prob2 = s1s.getOutput( s1s.getMiddleValues( s1s.getInputValues( bwf, false ) ) );
    cout << "sym board value for flipped winning board = " << prob2 << endl;
    cout << "Sum (should be 1) = " << prob1 + prob2 << endl;
    
    // do the initial runs using the random-weight networks as a benchmark starting point
    
    double maxPpg = -100, maxPpgs = -100;
    
    cout << "Normal:\n";
    playParallel( s1, s2, 1000, 1, 0, "normal" );
    cout << "Prob of win from initial board = " << s1.getOutput( s1.getMiddleValues( s1.getInputValues( bs ) ) ) << endl;
    cout << "Prob of win from winning board = " << s1.getOutput( s1.getMiddleValues( s1.getInputValues( bw ) ) ) << endl;
    cout << "Prob of win from losing board  = " << s1.getOutput( s1.getMiddleValues( s1.getInputValues( bl ) ) ) << endl;
    
    cout << "\nSymmetric:\n";
    playParallel( s1s, s2, 1000, 1, 0, "sym" );
    cout << "Prob of win from initial board = " << s1s.getOutput( s1s.getMiddleValues( s1s.getInputValues( bs, true ) ) ) << endl;
    cout << "Prob of win from winning board = " << s1s.getOutput( s1s.getMiddleValues( s1s.getInputValues( bw, true ) ) ) << endl;
    cout << "Prob of win from losing board  = " << s1s.getOutput( s1s.getMiddleValues( s1s.getInputValues( bl, true ) ) ) << endl;
    
    cout << "\nNormal vs Symmetric:\n";
    s1s.learning = false;
    playParallel( s1, s1s, 1000, 1, 0, "compete" );
    cout << endl;
    
    // do the training
    
    int nw=0, ng=0, nb=0, ns=0;
    int nws=0, ngs=0, nbs=0, nss=0;
    
    for( long i=0; i<2000000; i++ )
    {
        s1.learning  = true;
        s1s.learning = true;
        
        game g( &s1, &s1, (int) (i+1) );
        g.stepToEnd();
        if( g.winner() == 0 ) nw += 1;
        if( g.winnerScore() == 2 ) ng += 1;
        if( g.winnerScore() == 3 ) nb += 1;
        ns += g.nSteps;

        game gs( &s1s, &s1s, (int) (i+1) );
        gs.stepToEnd();
        if( gs.winner() == 0 ) nws += 1;
        if( gs.winnerScore() == 2 ) ngs += 1;
        if( gs.winnerScore() == 3 ) nbs += 1;
        nss += gs.nSteps;

        if( (i+1)%100 == 0 ) 
        {
            double fw = nw/100.;
            double fg = ng/100.;
            double fb = nb/100.;
            double as = ns/100.;
            nw = 0;
            ng = 0;
            nb = 0;
            ns = 0;
            double fws = nws/100.;
            double fgs = ngs/100.;
            double fbs = nbs/100.;
            double ass = nss/100.;
            nws = 0;
            ngs = 0;
            nbs = 0;
            nss = 0;
            
            cout << (i+1) << "   Normal   " << fw  << "   " << fg  << "   " << fb  << "   " << as  << endl;
            cout << (i+1) << "   Symmet   " << fws << "   " << fgs << "   " << fbs << "   " << ass << endl;
        }
        
        if( (i+1)%1000 == 0 )
        {
            cout << "Normal\n";
            double ppg = playParallel( s1, s2, 1000, 1, i+1, "normal" );
            if( ppg > maxPpg )
            {
                cout << "***** Normal: Rolling best ppg = " << ppg << " vs previous max " << maxPpg << "*****\n";
                maxPpg = ppg;
            }
            cout << "Prob of win from initial board = " << s1.getOutput( s1.getMiddleValues( s1.getInputValues( bs ) ) ) << endl;
            cout << "Prob of win from winning board = " << s1.getOutput( s1.getMiddleValues( s1.getInputValues( bw ) ) ) << endl;
            cout << "Prob of win from losing board  = " << s1.getOutput( s1.getMiddleValues( s1.getInputValues( bl ) ) ) << endl;
            
            cout << "\nSymmetric:\n";
            double ppgs = playParallel( s1s, s2, 1000, 1, i+1, "sym" );
            if( ppgs > maxPpgs )
            {
                cout << "***** Symmetric: Rolling best ppg = " << ppgs << " vs previous max " << maxPpgs << "*****\n";
                maxPpgs = ppgs;
            }
            cout << "Prob of win from initial board = " << s1s.getOutput( s1s.getMiddleValues( s1s.getInputValues( bs, true ) ) ) << endl;
            cout << "Prob of win from winning board = " << s1s.getOutput( s1s.getMiddleValues( s1s.getInputValues( bw, true ) ) ) << endl;
            cout << "Prob of win from losing board  = " << s1s.getOutput( s1s.getMiddleValues( s1s.getInputValues( bl, true ) ) ) << endl;
            
            cout << "\nNormal vs Symmetric:\n";
            s1s.learning = false;
            playParallel( s1, s1s, 1000, 1, i+1, "compete" );
            cout << endl;
        }
    }
}

void dispBoardGam( int ind, bool flipped, const strategytdoriggam& s, const board& b );
void dispBoardsGam( const strategytdoriggam& s );

void dispBoardGam( int ind, bool flipped, const strategytdoriggam& s, const board& b )
{
    vector<double> mids = s.getMiddleValues( s.getInputValues( b ) );
    double pw  = s.getOutputWin( mids );
    double pg  = s.getOutputGammon( mids );
    double pgl = s.getOutputGammonLoss( mids );
    
    cout << "Reference board " << ind;
    if( flipped ) 
        cout << "*";
    else
        cout << " ";
    cout << ": " << pw << "; ( " << pg << ", " << pgl << " )\n";
}

void dispBoardsGam( const strategytdoriggam& s )
{
    for( int ind=0; ind<7; ind++ ) 
    {
        board b( referenceBoard(ind) );
        board fb( b );
        fb.setPerspective( ( b.perspective() + 1 ) % 2 );
        dispBoardGam( ind, false, s, b );
        dispBoardGam( ind, true, s, fb );
    }
    cout << endl;
}

void sim8( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix )
{
    // train strategytdoriggam
    
    // print out the reference boards
    
    for( int ind=0; ind<7; ind++ )
    {
        cout << "Reference board " << ind << endl;
        board b( referenceBoard( ind ) );
        b.print();
        cout << endl;
    }
    
    // try out the TD strategy with multiple networks
    
    strategytdoriggam s1( nMiddle );
    strategytdexp2 s2( "benchmark", "exp2_max" + srcSuffix, false ); // opponent
    s2.learning = false;
    //strategyPubEval s2;
    
    s1.alpha = alpha0;
    s1.beta  = beta0;
    
    double maxPpg = -100;
    long maxInd=-1;
    
    playParallel( s1, s2, 1000, 1, 0, "gam_std" + fileSuffix );
    dispBoardsGam( s1 );
    
    int nw=0, ng=0, nb=0, ns=0;
    
    for( long i=0; i<20000000; i++ )
    {
        if( i==200000 or i==1000000 or i==10000000 )
        {
            s1.alpha *= 1/5.;
            s1.beta  *= 1/5.;
            cout << "\n\n***** Dropped alpha and beta by a factor of 5*****\n";
            cout << "alpha = " << s1.alpha << endl;
            cout << "beta  = " << s1.beta  << endl;
            cout << endl;
        }
        s1.learning = true;
        game g( &s1, &s1, (int) (i+1) );
        g.setTurn( (int) i%2 );
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
        
        if( g.winner() == 0 ) nw += 1;
        if( g.winnerScore() == 2 ) ng += 1;
        if( g.winnerScore() == 3 ) nb += 1;
        ns += g.nSteps;
        
        if( (i+1)%100 == 0 ) 
        {
            double fw = nw/100.;
            double fg = ng/100.;
            double fb = nb/100.;
            double as = ns/100.;
            nw = 0;
            ng = 0;
            nb = 0;
            ns = 0;
            
            cout << (i+1) << "   " << fw << "   " << fg << "   " << fb << "   " << as << endl;
            
            s1.writeWeights( "gam_std" + fileSuffix );
        }
        
        if( (i+1)%1000 == 0 )
        {
            cout << endl;
            double ppg = playParallel( s1, s2, 1000, 1, i+1, "gam_std" + fileSuffix );
            if( ppg > maxPpg )
            {
                cout << "***** Rolling best ppg = " << ppg << " vs previous max " << maxPpg << "*****\n";
                maxPpg = ppg;
                maxInd = i;
                s1.writeWeights( "gam_max" + fileSuffix );
            }
            else
                cout << "Prev best " << maxPpg << " at index " << maxInd+1 << endl;
            cout << endl;
            
            dispBoardsGam( s1 );
        }
    }
}

void dispBoardBg( int ind, bool flipped, const strategytdorigbg& s, const board& b );
void dispBoardsBg( const strategytdorigbg& s );

void dispBoardBg( int ind, bool flipped, const strategytdorigbg& s, const board& b )
{
    vector<double> mids = s.getMiddleValues( s.getInputValues( b ) );
    double pw  = s.getOutputWin( mids );
    double pg  = s.getOutputGammon( mids );
    double pgl = s.getOutputGammonLoss( mids );
    double pb  = s.getOutputBackgammon( mids );
    double pbl = s.getOutputBackgammonLoss( mids );
    
    cout << "Reference board " << ind;
    if( flipped ) 
        cout << "*";
    else
        cout << " ";
    cout << ": " << pw << "; ( " << pg << ", " << pgl << " ); ( " << pb << ", " << pbl << " )\n";
}

void dispBoardsBg( const strategytdorigbg& s )
{
    for( int ind=0; ind<7; ind++ ) 
    {
        board b( referenceBoard(ind) );
        board fb( b );
        fb.setPerspective( ( b.perspective() + 1 ) % 2 );
        dispBoardBg( ind, false, s, b );
        dispBoardBg( ind, true, s, fb );
    }
    cout << endl;
}


void sim9( int nMiddle, double alpha0, double beta0, const string& fileSuffix, const string& srcSuffix )
{
    // train strategytdorigbg
    
    // print out the reference boards
    
    for( int ind=0; ind<7; ind++ )
    {
        cout << "Reference board " << ind << endl;
        board b( referenceBoard( ind ) );
        b.print();
        cout << endl;
    }
    
    // try out the TD strategy with multiple networks
    
    strategytdorigbg s1( nMiddle );
    strategytdorigbg s2( "benchmark", "bg_std" + srcSuffix ); // opponent
    s2.learning = false;
    //strategyPubEval s2;
    
    s1.alpha = alpha0;
    s1.beta  = beta0;
    
    double maxPpg = -100;
    long maxInd=-1;
    
    playParallel( s1, s2, 1000, 1, 0, "bg_std" + fileSuffix );
    dispBoardsBg( s1 );
    
    int nw=0, ng=0, nb=0, ns=0;
    
    for( long i=0; i<20000000; i++ )
    {
        if( i==200000 or i==1000000 or i==10000000 )
        {
            s1.alpha *= 1/5.;
            s1.beta  *= 1/5.;
            cout << "\n\n***** Dropped alpha and beta by a factor of 5*****\n";
            cout << "alpha = " << s1.alpha << endl;
            cout << "beta  = " << s1.beta  << endl;
            cout << endl;
        }
        s1.learning = true;
        game g( &s1, &s1, (int) (i+1) );
        g.setTurn( (int) i%2 );
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
        
        if( g.winner() == 0 ) nw += 1;
        if( g.winnerScore() == 2 ) ng += 1;
        if( g.winnerScore() == 3 ) nb += 1;
        ns += g.nSteps;
        
        if( (i+1)%100 == 0 ) 
        {
            double fw = nw/100.;
            double fg = ng/100.;
            double fb = nb/100.;
            double as = ns/100.;
            nw = 0;
            ng = 0;
            nb = 0;
            ns = 0;
            
            cout << (i+1) << "   " << fw << "   " << fg << "   " << fb << "   " << as << endl;
            
            s1.writeWeights( "bg_std" + fileSuffix );
        }
        
        if( (i+1)%1000 == 0 )
        {
            cout << endl;
            double ppg = playParallel( s1, s2, 1000, 1, i+1, "bg_std" + fileSuffix );
            if( ppg > maxPpg )
            {
                cout << "***** Rolling best ppg = " << ppg << " vs previous max " << maxPpg << "*****\n";
                maxPpg = ppg;
                maxInd = i;
                s1.writeWeights( "bg_max" + fileSuffix );
            }
            else
                cout << "Prev best " << maxPpg << " at index " << maxInd+1 << endl;
            cout << endl;
            
            dispBoardsBg( s1 );
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
            w0 = playSerial( s1, s2, 200, i+1, i+1, "nowrite" );
        }
        catch( string& err )
        {
            cout << "ERROR: " << err << endl;
        }
    }
}

void test2()
{
    // load the weights and traces from the saved files
    
    vector<double> outputWeights, outputTraces;
    vector< vector<double> > middleWeights, middleTraces;
    
    readWeightsFromFile( outputWeights, middleWeights, outputTraces, middleTraces, "std40_01_01" );
    
    // construct the TD strategy
    
    strategytd s2( outputWeights, middleWeights, outputTraces, middleTraces, 0.1, 0.1, 0 );
    s2.learning = false;
    
    // construct the expanded TD strategy by comparison
    
    vector<double> outputProbWeights, outputGammonWeights, outputProbTraces, outputGammonTraces;
    vector< vector<double> > middleExpWeights, middleProbTraces, middleGammonTraces;
    
    readExpWeightsFromFile( outputProbWeights, outputGammonWeights, middleExpWeights, outputProbTraces, outputGammonTraces, middleProbTraces, middleGammonTraces, "exp_maxexp_80_0.1_0.1" );
    strategytdexp s1( outputProbWeights, outputGammonWeights, middleExpWeights, outputProbTraces, outputGammonTraces, middleProbTraces, middleGammonTraces, 0.1, 0.1, 0 );
    s1.learning = false;
    
    readExpWeightsFromFile( outputProbWeights, outputGammonWeights, middleExpWeights, outputProbTraces, outputGammonTraces, middleProbTraces, middleGammonTraces, "exp_maxexp_20_0.1_0.1" );
    strategytdexp s4( outputProbWeights, outputGammonWeights, middleExpWeights, outputProbTraces, outputGammonTraces, middleProbTraces, middleGammonTraces, 0.1, 0.1, 0 );
    s4.learning = false;
    
    strategyPubEval s3;
    
    playSerial( s1, s3, 200, 1, 0, "nowrite" );
    
    //game g( &s1, &s1, 2 );
    //g.verbose = true;
    //g.stepToEnd();
    //cout << g.winner() << "  " << g.winnerScore() << endl;
}

void test3()
{
    // check that flipping the perspective flips the probabilities properly for the TD 
    // strategy that uses three outputs instead of 1.
    
    strategyPubEval s0;
    strategytdexp s1;
    
    game g(&s0,&s0,1);
    for( int i=0; i<24; i++ ) g.step();
    g.getBoard().print();
    
    board b( g.getBoard() );
    
    double probWin = s1.getOutputProbValue( s1.getMiddleValues( s1.getInputValues( b ) ) );
    double probCondGammon = s1.getOutputGammonValue( s1.getMiddleValues( s1.getInputValues( b ) ) );
    double probCondGammonLoss = s1.getOutputGammonLossValue( s1.getMiddleValues( s1.getInputValues( b ) ) );
    
    double probWinGammon = probWin * probCondGammon;
    double probLoseGammon = ( 1 - probWin ) * probCondGammonLoss;
    double probLose = 1 - probWin;
    
    cout << "prob win         = " << probWin << endl;
    cout << "prob lose        = " << probLose << endl;
    cout << "prob win gammon  = " << probWinGammon << endl;
    cout << "prob lose gammon = " << probLoseGammon << endl;
    cout << endl;
    
    // when we flip perspective we should have win/lose probabilities switch places
    
    b.setPerspective( 1 );
    probWin = s1.getOutputProbValue( s1.getMiddleValues( s1.getInputValues( b ) ) );
    probCondGammon = s1.getOutputGammonValue( s1.getMiddleValues( s1.getInputValues( b ) ) );
    probCondGammonLoss = s1.getOutputGammonLossValue( s1.getMiddleValues( s1.getInputValues( b ) ) );
    
    probWinGammon = probWin * probCondGammon;
    probLoseGammon = ( 1 - probWin ) * probCondGammonLoss;
    probLose = 1 - probWin;
    
    cout << "prob win         = " << probWin << endl;
    cout << "prob lose        = " << probLose << endl;
    cout << "prob win gammon  = " << probWinGammon << endl;
    cout << "prob lose gammon = " << probLoseGammon << endl;
}

void test4()
{
    // try out playing against a human
    
    //strategytdexp3 s1( "", "exp3_maxexp_20_0.1_0.1", false );
    //strategytdexp s1( "benchmark", "exp_maxexp_80_0.1_0.1" );
    strategytdmult s1( "benchmark", "mult_stdmult_80_0.1_0.1" );
    //strategytdoriggam s1( "", "gam_maxgam_80_0.1_0.1" );
    s1.learning = false;
    strategyply s1a( 1, 8, s1 );
    
    //strategyPubEval s1;
    strategyhuman s2;
    
    vector<int> scores;
    int n=10;
    scores.resize(n,0);
    double pw, pgw, pgl, pbw, pbl;
    
    for( int i=0; i<n; i++ )
    {
        cout << "NEW GAME - game # " << i+1 << endl;
        game g(&s2,&s1a,i+585);
        g.verbose = true;
        g.setTurn(i%2);
        g.getBoard().print();
        while( !g.gameOver() )
        {
            // get the network probability of white winning & gammoning
            
            board b( g.getBoard() );
            b.setPerspective( g.turn() );
            
            string eval( s1.evaluator( b ) );
            cout << "Evaluator = " << eval << endl;
            if( eval == "done" )
            {
                
            }
            else if( eval == "bearoff" )
            {
                pw  = s1.bearoffProbabilityWin( b );
                pgw = s1.bearoffProbabilityGammon( b );
                pgl = s1.bearoffProbabilityGammonLoss( b );
                pbw = 0;
                pbl = 0;
            }
            else
            {
                vector<double> middles = s1.getMiddleValues( s1.getInputValues( b, eval ), eval );
                
                pw  = s1.getOutputProbValue( middles, eval );
                pgw = s1.getOutputGammonValue( middles, eval );
                pgl = s1.getOutputGammonLossValue( middles, eval );
                pbw = s1.getOutputBackgammonValue( middles, eval );
                pbl = s1.getOutputBackgammonLossValue( middles, eval );
            }
            if( g.turn() == 1 )
            {
                double temp;
                pw = 1 - pw;
                temp = pgw;
                pgw  = pgl;
                pgl  = temp;
                temp = pbw;
                pbw  = pbl;
                pbl  = pbw;
            }
            double equity = ( pw - pgw ) + 2 * ( pgw - pbw ) + 3 * pbw - ( 1 - pw - pgl ) - 2 * ( pgl - pbl ) - 3 * pbl;
            cout << "White equity                     = " << equity << endl;
            cout << "Probability of white win         = " << pw  << endl;
            cout << "Probability of white gammon win  = " << pgw << endl;
            cout << "Probability of white gammon loss = " << pgl << endl;
            cout << "Probability of white bg win      = " << pbw << endl;
            cout << "Probability of white bg loss     = " << pbl << endl;
            cout << "White pips                       = " << g.getBoard().pips() << endl;
            cout << "Black pips                       = " << g.getBoard().otherPips() << endl;
            g.step();
        }
        
        cout << endl;
        cout << "GAME OVER\n";
        
        if( g.winner() == 0 )
        {
            cout << "You won " << g.winnerScore() << " points\n";
            scores.at(i) = g.winnerScore();
        }
        else
        {
            cout << "You lost " << g.winnerScore() << " points\n";
            scores.at(i) = -g.winnerScore();
        }
    }
    
    double ppg=0;
    for( int i=0; i<n; i++ ) 
    {
        cout << "Game " << i+1 << " had score " << scores.at(i) << endl;
        ppg += scores.at(i);
    }
    ppg/=n;
    
    cout << endl;
    cout << "Average ppg = " << ppg << endl;
}

struct bandv
{
    board b;
    double val;
};

bool bandvComp( const bandv& v1, const bandv& v2 );
bool bandvComp( const bandv& v1, const bandv& v2 ) { return v1.val > v2.val; }

void testOrigGam()
{
    // try out playing against a human
    
    strategytdorigbg s1( "", "bg_maxbg_120_0.1_0.1" );
    strategytdorigbg s2( "benchmark", "bg_stdbg_80_0.1_0.1" );
    //strategytdoriggam s2( "benchmark", "gam_maxgam_80_0.1_0.1" );
    //strategytdmult s2( "benchmark", "mult_maxmult_80_0.1_0.1" );
    s1.learning = false;
    s2.learning = false;
    //strategyply s2( 2, 5, s1 );
    
    //strategyPubEval s2;
    
    double avgVal=0;
    for( int i=0; i<10; i++ )
        avgVal += playParallel( s1, s2, 1000, 1001 + i*1000, 0, "nowrite" );
    avgVal /= 10.;
    cout << "Average equity = " << avgVal << endl;
    
    //playSerial( s1, s2, 800, 2779, 0, "nowrite" );
    
    /*
    board b( referenceBoard( 4 ) );
    b.print();
    cout << s1.getOutputBackgammonLossValue( s1.getMiddleValues( s1.getInputValues( b ), "contact" ), "contact" ) << endl;
    b.setPerspective(1-b.perspective());
    cout << "Regular strategy = " << -s1.boardValue( b ) << endl;
    cout << "Ply strategy     = " << -s2.boardValue( b ) << endl;
    
    int nRuns=10000;
    //double rollVal = rolloutBoardValue( b, s1, nRuns, 12100 );
    double rollVal = rolloutBoardValueParallel( b, s1, nRuns, 20000, 16 );
    cout << "Rollout          = " << -rollVal << endl;
    
    //strategyPubEval s1;
    strategyhuman s2;
    
    vector<int> scores;
    int n=10;
    scores.resize(n,0);
    
    double pw, pgw, pgl, pbw, pbl;
    
    for( int i=0; i<n; i++ )
    {
        cout << "NEW GAME - game # " << i+1 << endl;
        game g(&s2,&s1,i+3985);
        g.verbose = true;
        g.setTurn(i%2);
        g.getBoard().print();
        while( !g.gameOver() )
        {
            // get the network probability of white winning & gammoning
            
            board b( g.getBoard() );
            b.setPerspective( g.turn() );
            
            vector<double> middles = s1.getMiddleValues( s1.getInputValues( b ) );
            
            pw  = s1.getOutputWin( middles );
            pgw = s1.getOutputGammon( middles );
            pgl = s1.getOutputGammonLoss( middles );
            pbw = s1.getOutputBackgammon( middles );
            pbl = s1.getOutputBackgammonLoss( middles );
            
            if( g.turn() == 1 )
            {
                double temp;
                pw = 1 - pw;
                temp = pgw;
                pgw  = pgl;
                pgl  = temp;
                temp = pbw;
                pbw = pbl;
                pbl = temp;
            }
            double equity = ( pw - pgw ) + 2 * ( pgw - pbw ) + 3 * pbw - ( 1 - pw - pgl ) - 2 * ( pgl - pbl ) - 3 * pbl;
            cout << "White equity                     = " << equity << endl;
            cout << "Probability of white win         = " << pw  << endl;
            cout << "Probability of white gammon win  = " << pgw << endl;
            cout << "Probability of white gammon loss = " << pgl << endl;
            cout << "Probability of white bg win      = " << pbw << endl;
            cout << "Probability of white bg loss     = " << pbl << endl;
            cout << "White pips                       = " << g.getBoard().pips() << endl;
            cout << "Black pips                       = " << g.getBoard().otherPips() << endl;
            g.step();
        }
        
        cout << endl;
        cout << "GAME OVER\n";
        
        if( g.winner() == 0 )
        {
            cout << "You won " << g.winnerScore() << " points\n";
            scores.at(i) = g.winnerScore();
        }
        else
        {
            cout << "You lost " << g.winnerScore() << " points\n";
            scores.at(i) = -g.winnerScore();
        }
    }
    
    double ppg=0;
    for( int i=0; i<n; i++ ) 
    {
        cout << "Game " << i+1 << " had score " << scores.at(i) << endl;
        ppg += scores.at(i);
    }
    ppg/=n;
    
    cout << endl;
    cout << "Average ppg = " << ppg << endl;
    */
}

void printWeights3( const string& srcSuffix )
{
    strategytdexp3 s1( "", "exp3_max" + srcSuffix, false );
    strategytdexp  s2( "benchmark", "exp_max" + srcSuffix ); // reference case
    
    // prob output
    
    int nMiddle = s1.nMiddle;
    int i, j;
    cout.precision( 3 );
    vector<double> prob1( s1.getOutputProbWeights() );
    cout.setf( ios::left );
    cout << "Prob1: ";
    for( i=0; i<nMiddle-1; i++ )
    {
        cout.setf( ios::right );
        cout.setf( ios::fixed );
        cout.width( 9 );
        cout << prob1.at(i);
    }
    cout << endl;
    vector<double> prob2( s2.getOutputProbWeights() );
    cout.setf( ios::left );
    cout << "Prob2: ";
    for( i=0; i<nMiddle-1; i++ )
    {
        cout.setf( ios::right );
        cout.setf( ios::fixed );
        cout.width( 9 );
        cout << prob2.at(i);
    }
    cout << endl << endl;
    
    // gammon output
    
    vector<double> gam1( s1.getOutputGammonWinWeights() );
    cout.setf( ios::left );
    cout << "Gam1:  ";
    for( i=0; i<nMiddle+1; i++ )
    {
        cout.setf( ios::right );
        cout.setf( ios::fixed );
        cout.width( 9 );
        cout << gam1.at(i);
    }
    cout << endl;
    vector<double> gam2( s2.getOutputGammonWeights() );
    cout.setf( ios::left );
    cout << "Gam2:  ";
    for( i=0; i<nMiddle+1; i++ )
    {
        cout.setf( ios::right );
        cout.setf( ios::fixed );
        cout.width( 9 );
        cout << gam2.at(i);
    }
    cout << endl << endl;
    
    // middle weights
    
    vector< vector<double> > mid1( s1.getMiddleWeights() );
    vector< vector<double> > mid2( s2.getMiddleWeights() );
    
    for( i=0; i<nMiddle; i++ )
    {
        cout.setf( ios::left );
        cout << "Mid1";
        cout.setf( ios::right );
        cout.width( 4 );
        cout << i;
        for( j=0; j<99; j++ )
        {
            cout.setf( ios::right );
            cout.setf( ios::fixed );
            cout.width( 9 );
            cout << mid1.at(i).at(j);
        }
        cout << endl;
        cout << "Mid2";
        cout.setf( ios::right );
        cout.width( 4 );
        cout << i;
        for( j=0; j<99; j++ )
        {
            cout.setf( ios::right );
            cout.setf( ios::fixed );
            cout.width( 9 );
            if( j<98 )
                cout << mid2.at(i).at(j);
            else
                cout << 0.;
        }
        cout << endl << endl;
    }
}


void playBearoff()
{
     /*
     board b;
     
     int i;
     for( i=0; i<24; i++ )
     {
     b.setChecker( i, 0 );
     b.setOtherChecker( i, 0 );
     }
     
     b.setBornIn( 11 );
     b.setOtherBornIn( 13 );
     
     b.setChecker( 5, 4 );
     b.setOtherChecker( 18, 2 );
     
     int nPnts=6;
     
     cout << boardID( b, nPnts ) << endl;
     cout << getBoardPnt( b, nPnts ) << endl;
     */
    
    int nPnts=6;
    int nCheckers=8;
    cout << nElementsTS( nPnts, nCheckers ) << " est elements\n";
    constructBearoff( nPnts, nCheckers );
    //loadBearoffDb( "/Users/mghiggins/bgtdres/bearoff.txt" );
    
    hash_map<string,double> * m = boardPnts();
    
    //for( hash_map<string,double>::iterator it=(*m).begin(); it!=(*m).end(); it++ )
    //    cout << (*it).first << "   " << (*it).second << endl;
    
    if( m ) cout << m->size() << " elements" << endl;
    
    writeBearoffDb( "/Users/mghiggins/bgtdres/bearoff_6_8.txt" );
}

void playBearoffOneSided()
{
    /*
    board b;
    int i;
    for( i=0; i<24; i++ )
    {
        b.setChecker( i, 0 );
        b.setOtherChecker( i, 0 );
    }
    b.setOtherBornIn( 10 );
    b.setBornIn( 0 );
    
    b.setChecker( 1, 10 );
    b.setChecker( 2, 3 );
    b.setChecker( 6, 1 );
    b.setChecker( 10, 1 );
    b.setOtherChecker( 23, 1 );
    b.setOtherChecker( 22, 1 );
    b.setOtherChecker( 19, 3 );
    try
    {
        b.validate();
    }
    catch( ... )
    {
        cout << "Invalid board\n";
        return;
    }
    //b.setPerspective( 1 );
    b.print();
    
    strategytdexp2 s2( "benchmark", "exp2_maxexp_80_0.1_0.1", false ); // opponent
    s2.learning = false;
    
    double ppg = s2.boardValue( b );
    vector<double> mids( s2.getMiddleValues( s2.getInputValues( b, true ) ) );
    double pw  = s2.getOutputProbValue( mids );
    double pg  = s2.getOutputGammonWinValue( mids, b );
    double pgl = s2.getOutputGammonLossValue( mids, b );
    
    int nPnts = 9;
    
    cout << getBoardPntOS( b, nPnts ) << "  " << ppg << endl;
    cout << getProbabilityWin( b, nPnts ) << "   " << pw << endl;
    cout << getProbabilityGammonWin( b, nPnts ) << "    " << pg << endl;
    cout << getProbabilityGammonLoss( b, nPnts ) << "    " << pgl << endl;
    cout << stepsProbs()->size() << endl;
    cout << gamStepsProbs()->size() << endl;
    
    */
    int nPnts=6;
    int nCheckers=15;
    int maxElem=10;
    cout << nElementsOS( nPnts, nCheckers ) << " est elements\n";
    constructBearoffOneSidedParallel( nPnts, nCheckers, maxElem, 10, 5 );
    //loadBearoffDbOneSided( "/Users/mghiggins/bgtdres/benchmark/bearoffOS_9_15.csv" );
    cout << "Regular db size = " << stepsProbs()->size() << endl;
    writeBearoffDbOneSided( "/Users/mghiggins/bgtdres/bearoffOS_6_15.csv" );
    //constructGammonBearoffOneSided( nPnts, maxElem );
    
    //cout << "Gammon db size  = " << gamStepsProbs()->size() << endl;
    //writeGammonBearoffDbOneSided( "/Users/mghiggins/bgtdres/bearoffOSGam_9_15.csv" );
}

void compareBearoff()
{
    // compare one-sided to two-sided bearoff equity predictions
    
    loadBearoffDb( "/Users/mghiggins/bgtdres/benchmark/bearoff_6_9.txt" );
    loadBearoffDbOneSided( "/Users/mghiggins/bgtdres/bearoffOS_6_9.csv" );
    
    hash_map<string,double> * pnts = boardPnts();
    hash_map<string,stepsDistribution> * stepsd = stepsProbs();
    
    cout << "Number of elements in two-sided db = " << pnts->size() << endl;
    cout << "Number of elements in one-sided db = " << stepsd->size() << endl;
    cout << endl;
    
    int nPnts = 6;
    long count = 0, countElem = 0;
    double elemProb = 0;
    double p1, p2, diff;
    double avgDiff=0, avgDiffSq=0;
    double maxDiff=-100, minDiff=100;
    bool go=false;
    
    for( hash_map<string,double>::iterator it=pnts->begin(); it!=pnts->end(); it++ )
    {
        string ID = it->first;
        board b( boardFromID( ID ) );
        p2 = it->second;
        p1 = getBoardPntOS( b, nPnts );
        stepsDistribution dist = getStepsDistribution( b, nPnts );
        if( dist.stepProbs.size() > 8 ) 
        {
            countElem ++;
            vector<stepProbability> pairs( dist.pairsProbOrdered() );
            for( int i=8; i<pairs.size(); i++ ) elemProb += pairs.at(i).prob;
        }
        diff = p1 - p2;
        
        avgDiff += diff;
        avgDiffSq += diff * diff;
        
        go = false;
        
        if( diff > maxDiff ) 
        {
            maxDiff = diff;
            go = true;
        }
        if( diff < minDiff ) 
        {
            minDiff = diff;
            go = true;
        }
        
        count += 1;
        if( count % 1000000 == 0 || go )
        {
            b.print();
            
            cout << "Two-sided bearoff equity = " << p2 << endl;
            cout << "One-sided bearoff equity = " << p1 << endl;
            cout << "Diff                     = " << diff << endl;
            if( fabs( diff ) > 1e-3 ) cout << "**** Diff bigger than 0.001\n";
            if( go ) cout << "*** extreme running value\n";
            cout << endl;
        }
    }
    
    long n( pnts->size() );
    avgDiff /= n;
    avgDiffSq /= n;
    if( countElem > 0 ) elemProb /= countElem;
    
    double diffSD = sqrt( avgDiffSq - avgDiff * avgDiff );
    double elemFrac = ( (float) countElem ) / n;
    
    cout << "Average diff = " << avgDiff << endl;
    cout << "Diff std dev = " << diffSD << endl;
    cout << "Max diff     = " << maxDiff << endl;
    cout << "Min diff     = " << minDiff << endl;
    cout << "Frac > 8 ps  = " << elemFrac << endl;
    cout << "Prob > 8 ps  = " << elemProb << endl;
}

void testHittingShots()
{
    board b( referenceBoard(7) );
    /*
    for( int i=0; i<24; i++ )
    {
        b.setChecker( i, 0 );
        b.setOtherChecker( i, 0 );
    }
    b.setBornIn(14);
    b.setChecker(6,1);
    b.setOtherBornIn(13);
    b.incrementOtherHit();
    b.setOtherChecker( 2, 1 );
    */
    b.validate();
    b.print();
    
    set<roll> hittingRolls( hittingShots( b, true ) );
    cout << hittingRolls.size() << " hitting rolls\n";
    cout << "Prob of a hit = " << hittingProb( hittingRolls ) << endl;
    cout << endl;
    
    for( set<roll>::iterator it=hittingRolls.begin(); it!=hittingRolls.end(); it++ )
        cout << "( " << it->die1 << ", " << it->die2 << " )\n";
    
}