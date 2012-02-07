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
#include <iostream>
#include <fstream>
#include <cmath>
#include <boost/thread.hpp>
#include "bearofffns.h"
#include "runners.h"
#include "strategytd.h"
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
#include "benchmark.h"

void dispBoard( int ind, bool flipped, const strategytdmult& s, const board& b );
void dispBoards( const strategytdmult& s );
void dispBoardGam( int ind, bool flipped, const strategytdoriggam& s, const board& b );
void dispBoardsGam( const strategytdoriggam& s );
void dispBoardBg( int ind, bool flipped, const strategytdorigbg& s, const board& b );
void dispBoardsBg( const strategytdorigbg& s );

vector<int> points;
vector<int> steps;

class worker
{
public:
    worker( long i, strategy& s1, strategy& s2, long n, long initSeed ) : i(i), s1(s1), s2(s2), n(n), initSeed(initSeed) {};
    
    void operator()()
    {
        game g( &s1, &s2, (int)(i+initSeed) );
        g.setTurn( ((int) i)%2 );
        //g.setContextValue( "singleGame", 1 );
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
    strategy& s1;
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

runStats playParallelGen( strategy& s1, strategy& s2, long n, long initSeed )
{
    using namespace boost;
    
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
    
    runStats stats;
    stats.ppg = ppg;
    stats.fracWin = w0;
    stats.avgSteps = avgSteps;
    stats.fracSingle = ns;
    stats.fracGammon = ng;
    stats.fracBg     = nb;
    
    return stats;
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
        //if( (i+1)%1 == 0 ) cout << "Run " << i+1 << endl;
        
        game g( &s1, &s2, (int)(i+initSeed) );
        g.setTurn( ((int) i)%2 );
        //g.setContextValue( "singleGame", 1 );
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

runStats playSerialGen( strategy& s1, strategy& s2, long n, long initSeed )
{
    float w0=0;
    float p=0, q=0;
    float avgSteps=0;
    float ns=0, ng=0, nb=0;
    int   s;
    
    for( long i=0; i<n; i++ )
    {
        //if( (i+1)%1 == 0 ) cout << "Run " << i+1 << endl;
        
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
    
    runStats stats;
    stats.ppg = p;
    stats.fracWin = w0;
    stats.avgSteps = avgSteps;
    stats.fracSingle = ns;
    stats.fracGammon = ng;
    stats.fracBg     = nb;
    
    return stats;
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
    
    //strategytdmult s1( "benchmark", "mult_stdmult_80_0.02_0.02", false );
    strategytdmult s1( nMiddle, false, false );
    //strategytdmult s2( "benchmark", "mult_stdmult_80_0.02_0.02", false );
    //s2.learning = false;
    strategyPubEval s2;
    
    s1.alpha = alpha0;
    s1.beta  = beta0;
    
    double maxPpg = -100;
    long maxInd=-1;
    
    //playSerial( s1, s2, 200, 1, 0, "mult_std" + fileSuffix, true );
    playParallel( s1, s2, 1000, 1, 0, "std" + fileSuffix );
    dispBoards( s1 );
    
    int nw=0, ng=0, nb=0, ns=0;
    
    for( long i=0; i<20000000; i++ )
    {
        if( i==75000 or i==150000 or i==300000 )
        {
            s1.alpha *= 1./sqrt(10);
            s1.beta  *= 1./sqrt(10);
            cout << "\n\n***** Dropped alpha and beta by a factor of sqrt(10)*****\n";
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
            
            s1.writeWeights( "std" + fileSuffix );
        }
        
        if( (i+1)%1000 == 0 )
        {
            cout << endl;
            //double ppg = playSerial( s1, s2, 200, 1, i+1, "mult_std" + fileSuffix, true );
            double ppg = playParallel( s1, s2, 1000, 1, i+1, "std" + fileSuffix );

            if( ppg > maxPpg )
            {
                cout << "***** Rolling best ppg = " << ppg << " vs previous max " << maxPpg << "*****\n";
                maxPpg = ppg;
                maxInd = i;
                s1.writeWeights( "max" + fileSuffix );
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
    strategytdorigbg s2( "benchmark", "benchmark2" ); // opponent
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
    strategytdorigbg s2( "benchmark", "benchmark2" ); // opponent
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

void test4()
{
    // try out playing against a human
    
    strategytdmult s1( "benchmark", "player24" );
    //strategytdorigbg s1( "benchmark", "benchmark2" );
    s1.learning = false;
    //strategyPubEval s1;
    
    strategyhuman s2;
    
    vector<int> scores;
    int n=10;
    scores.resize(n,0);
    double pw, pgw, pgl, pbw, pbl;
    
    for( int i=0; i<n; i++ )
    {
        cout << "NEW GAME - game # " << i+1 << endl;
        game g(&s1,&s2,i+585);
        g.verbose = true;
        g.setTurn(i%2);
        g.getBoard().print();
        while( !g.gameOver() )
        {
            // get the network probabilities before the roll; need to flip from
            // the usual perspective of boardProbabilities that gets probs after the roll.
            
            board b( g.getBoard() );
            b.setPerspective( 1 - g.turn() ); // opposite to normal
            
            gameProbabilities probs( s1.boardProbabilities( b ) );
            
            // flip prob perspective
            
            pw  = 1 - probs.probWin;
            pgw = probs.probGammonLoss;
            pgl = probs.probGammonWin;
            pbw = probs.probBgLoss;
            pbl = probs.probBgWin;
            
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
    strategytdmult s0( "benchmark", "player24", false );
    //strategytdmult s1( "benchmark", "player24", false );
    strategytdmult sf( "benchmark", "player24q" );
    //strategytdorigbg s0( "benchmark", "benchmark2" );
    //strategytdorigbg sf(5);
    //strategytdmult sf(5,false,false);
    s0.learning = false;
    sf.learning = false;
    //s2.learning = false;
    
    strategyply s1( 1, 3, 0.1, s0, sf );
    strategyply s2( 2, 3, 0.1, s0, sf );
    
    board b( referenceBoard(7) );
    b.print();
    cout << "0-ply:\n" << s0.boardProbabilities(b) << endl;
    cout << "1-ply:\n" << s1.boardProbabilities(b) << endl;
    cout << "2-ply:\n" << s2.boardProbabilities(b) << endl;
    
    //cout << "rollout " << rolloutBoardValueVarReduction( b, s0, 21*100, 1 ) << endl;
    
    int nThreads=20;
    int nRuns=21*nThreads*100;
    int seed=1;
    
    cout << "Total number of paths = " << nRuns << endl << endl;
    
    gameProbabilities val;
    double v1=0, v2=0, bv;
    
    int nTrials=100;
    
    for( int i=0; i<nTrials; i++ )
    {
        cout << "trial " << i+1 << endl;
        val = rolloutBoardProbabilitiesParallel( b, s0, nRuns, seed+i*nRuns, nThreads, false );
        //val = rolloutBoardProbabilities(b, s0, nRuns, &rng1 );
        bv = s0.boardValueFromProbs(val);
        cout << "rollout:\n" << val << endl;
        
        v1 = v1 + bv;
        v2 = v2 + bv * bv;
        
        cout << "Avg = " << v1/(i+1) << endl;
        cout << "SD  = " << sqrt( v2/(i+1) - v1 * v1/(i+1)/(i+1) ) << endl;
        cout << endl;
    }
    
    return;
    
    /*
    //strategyPubEval s2;
    strategytdmult s2( "benchmark", "player24" );
    s2.learning = false;
    
    int nTot=100000;
    int nBkt=100;
    int nStep=nTot/nBkt;
    
    double avgVal=0, avgValSq=0, fracWin=0, fracWinSq=0;
    for( int i=0; i<nBkt; i++ )
    {
        cout << "Bucket " << i << endl;
        runStats stats = playParallelGen( s1, s2, nStep, 1001 + i*nStep );
        //runStats stats = playSerialGen( s1, s2, nStep, 1001 + i*nStep );
        
        avgVal    += stats.ppg;
        avgValSq  += stats.ppg * stats.ppg;
        fracWin   += stats.fracWin;
        fracWinSq += stats.fracWin * stats.fracWin;
        
        cout << "Average equity = " << avgVal/(i+1) << endl;
        cout << "Std dev equity = " << sqrt( fabs( avgValSq/(i+1) - avgVal * avgVal/(i+1)/(i+1) ) ) << endl;
        cout << "Std error      = " << sqrt( fabs( avgValSq/(i+1) - avgVal * avgVal/(i+1)/(i+1) ) / (i+1) ) << endl;
        cout << endl;
        cout << "Frac win       = " << fracWin/(i+1)*100 << endl;
        cout << "Std dev win    = " << sqrt( fabs( fracWinSq/(i+1) - fracWin * fracWin/(i+1)/(i+1) ) )*100 << endl;
        cout << "Std error      = " << sqrt( fabs( fracWinSq/(i+1) - fracWin * fracWin/(i+1)/(i+1) ) / (i+1) )*100 << endl;
        cout << endl;
    }
    */
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
    //board b( referenceBoard(7) );
    board b;
    
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
    
    b.validate();
    b.print();
    
    set<roll> hittingRolls( hittingShots( b, true ) );
    cout << hittingRolls.size() << " hitting rolls\n";
    cout << "Prob of a hit  = " << hittingProb( hittingRolls ) << endl;
    cout << "Prob of a hit2 = " << hittingProb2( b, true ) << endl;
    cout << endl;
    
    for( set<roll>::iterator it=hittingRolls.begin(); it!=hittingRolls.end(); it++ )
        cout << "( " << it->die1 << ", " << it->die2 << " )\n";
    
}

void playEscapes()
{
    constructBlockadeEscapeDb();
    
    board bl( referenceBoard(4) );
    bl.print();
    cout << getBlockadeEscapeCount( bl, 6 ) << endl;
    
}

void createBenchmarks()
{
    strategytdmult s0( "benchmark", "player24" );
    strategytdmult sf( "benchmark", "player24q" );
    setupHittingRolls();
    
    string pathName = "/Users/mghiggins/bgtdres/benchdb";
    int seed = 1;
    int nGames = 1200;
    int nFileBenchmarks = 100;
    int nThreads = 15;
    
    generateBenchmarkPositions( s0, sf, nGames, pathName, nFileBenchmarks, seed, nThreads, 15 );
    //generateBenchmarkPositionsSerial( s0, sf, 10, pathName, nFileBenchmarks, seed, 0 );
    
    int nRuns=3000;
    rolloutBenchmarkPositions( s0, pathName, nFileBenchmarks, nRuns, seed, nThreads, 144 );
}

void trainBenchmarks()
{
    strategytdmult s1( "benchmark", "player24" );
    strategytdmult s2( "benchmark", "player24" );
    s1.learning = s2.learning = false;
    string pathName = "/Users/mghiggins/bgtdres/benchdb";
    
    int seed = 1;
    
    cout << "Starting stats:\n";
    printErrorStatistics(s1, pathName);
    cout << endl;
    
    for( int i=0; i<300; i++ )
    {
        s1.alpha = s1.beta = 0.1;
        
        trainMult( s1, pathName, seed );
        
        cout << "Iteration " << i+1 << ":" << endl;
        printErrorStatistics(s1, pathName);
        cout << endl;
        
        if( i % 20 == 0 )
            playParallelGen(s1, s2, 1000, 2);
    }
    
}