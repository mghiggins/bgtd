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
#include "strategytdexp.h"
#include "strategytdexp2.h"
#include "strategytdexp3.h"
#include "strategytdexp4.h"
#include "strategypubeval.h"
#include "strategyhuman.h"
#include "game.h"

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

void writeExp3WeightsToFiles( const vector<double>& outputProbWeights, const vector<double>& outputGammonWinWeights, 
                              const vector< vector<double> >& middleWeights, const string& fileSuffix )
{
    // get the dimensions from the vectors
    
    int nMiddle = (int) middleWeights.size();
    
    // write the weights and traces to files
    
    string path     = "/Users/mghiggins/bgtdres";
    string fopName  = path + "/weightsOutProb_" + fileSuffix + ".txt";
    string fowName  = path + "/weightsOutGammonWin_" + fileSuffix + ".txt";
    string fmName   = path + "/weightsMiddle_" + fileSuffix + ".txt";
    
    ofstream fop( fopName.c_str() );
    ofstream fow( fowName.c_str() );
    ofstream fm( fmName.c_str() );
    
    // one line at the top of the prob output weights file that specifies the number of middles.
    
    fop << nMiddle << endl;
    
    for( int j=0; j<nMiddle; j++ )
    {
        if( j<nMiddle-1 ) fop << outputProbWeights[j] << endl;
        fow << outputGammonWinWeights[j] << endl;
        for( int k=0; k<99; k++ )
            fm  << middleWeights[j][k] << endl;
    }
    fow << outputGammonWinWeights[nMiddle] << endl;
    
    fop.close();
    fow.close();
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
        game g( &s1, &s2, (int)(i+initSeed) );
        //g.setTurn( ((int) i)%2 );
        g.setTurn(1);
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
    cout << "Prob of TD winning = " << w0 * 100 << endl;
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
    
    playSerial( s1, s2, 200, 1, 0, "std" + fileSuffix );
    
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
    //strategytdexp3 s1;
    strategytdexp s2( "benchmark", "exp_max" + srcSuffix ); // opponent
    s2.learning = false;
    //strategyPubEval s2;
    
    s1.alpha = alpha0;
    s1.beta  = beta0;
    
    double maxPpg = -100;
    
    playSerial( s1, s2, 200, 1, 0, "exp3_std" + fileSuffix, true );
    
    board b;
    vector<double> mids( s1.getMiddleValues( s1.getInputValues( b, true ) ) );
    double pw = s1.getOutputProbValue( mids );
    cout << "Prob of win = " << pw << endl;
    cout << "Prob of gammon win = " << pw * s1.getOutputGammonWinValue( mids, b ) << endl;
    cout << "Prob of gammon loss = " << ( 1 - pw ) * s1.getOutputGammonLossValue( mids, b ) << endl;
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
            
            writeExp3WeightsToFiles( s1.getOutputProbWeights(), s1.getOutputGammonWinWeights(), s1.getMiddleWeights(), "exp3_std" + fileSuffix );
        }
        
        if( (i+1)%1000 == 0 )
        {
            double ppg = playSerial( s1, s2, 200, 1, i+1, "exp3_std" + fileSuffix, true );
            if( ppg > maxPpg )
            {
                cout << "***** Rolling best ppg = " << ppg << " vs previous max " << maxPpg << "*****\n";
                maxPpg = ppg;
                writeExp3WeightsToFiles( s1.getOutputProbWeights(), s1.getOutputGammonWinWeights(), s1.getMiddleWeights(), "exp3_max" + fileSuffix );
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
    
    strategytdexp2 s1( "benchmark", "exp2_maxexp_80_0.1_0.1", false );
    //strategytdexp s1( "benchmark", "exp_maxexp_80_0.1_0.1" );
    s1.learning = false;
    
    //strategyPubEval s1;
    strategyhuman s2;
    
    vector<int> scores;
    int n=10;
    scores.resize(n,0);
    double pw, pgw, pgl;
    
    for( int i=0; i<n; i++ )
    {
        cout << "NEW GAME - game # " << i+1 << endl;
        game g(&s2,&s1,i+925);
        g.verbose = true;
        g.setTurn(i%2);
        g.getBoard().print();
        while( !g.gameOver() )
        {
            // get the network probability of white winning & gammoning
            
            pw  = s1.getOutputProbValue( s1.getMiddleValues( s1.getInputValues( g.getBoard(), true ) ) );
            pgw = pw * s1.getOutputGammonWinValue( s1.getMiddleValues( s1.getInputValues( g.getBoard(), true ) ), g.getBoard() );
            pgl = ( 1 - pw ) * s1.getOutputGammonLossValue( s1.getMiddleValues( s1.getInputValues( g.getBoard(), true ) ), g.getBoard() );
            //pgw = pw * s1.getOutputGammonValue( s1.getMiddleValues( s1.getInputValues( g.getBoard() ) ) );
            //pgl = ( 1 - pw ) * s1.getOutputGammonLossValue( s1.getMiddleValues( s1.getInputValues( g.getBoard() ) ) );
            cout << "Probability of white win         = " << pw  << endl;
            cout << "Probability of white gammon win  = " << pgw << endl;
            cout << "Probability of white gammon loss = " << pgl << endl;
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
