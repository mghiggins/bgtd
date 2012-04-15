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
#include "doublefns.h"
#include "doublestratsimple.h"
#include "doublestratjanowski.h"
#include "doublestratjanowskistate.h"
#include "doublestratjumpconst.h"
#include "doublestratmatch.h"
#include "match.h"
#include "matchequitytable.h"

void dispBoard( int ind, bool flipped, strategytdmult& s, const board& b );
void dispBoards( strategytdmult& s );
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
        //g.setTurn( ((int) i)%2 );
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

double playParallel( strategytdbase& s1, strategy& s2, long n, long initSeed, long displayIndex, const string& fileSuffix, int nBuckets )
{
    using namespace boost;
    
    s1.learning = false;
    
    if( n % nBuckets != 0 ) throw string( "Number of runs must be a multiple of nBuckts" );
    long nRuns = n / nBuckets;
    
    double ppg=0, w0=0, q=0, avgSteps=0, ns=0, ng=0, nb=0;
    
    for( int bkt=0; bkt<nBuckets; bkt++ )
    {
        cout << "o";
        cout.flush();
        
        // run each game in its own thread
        
        if( points.size() < nRuns ) 
        {
            points.resize(nRuns);
            steps.resize(nRuns);
        }
        
        thread_group ts;
        for( long i=0; i<nRuns; i++ ) ts.create_thread( worker( i, s1, s2, nRuns, initSeed + nRuns * bkt ) );
        ts.join_all();
        
        int p, ap;
        for( long i=0; i<nRuns; i++ ) 
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
    }
    cout << endl;
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

runStats playParallelGen( strategy& s1, strategy& s2, long n, long initSeed, int nBuckets, bool varReduc )
{
    using namespace boost;
    
    if( varReduc )
        if( n % (2*nBuckets) != 0 ) throw string( "n must be a multiple of 2*nBuckets" );
    else
        if( n % nBuckets != 0 ) throw string( "n must be a multiple of nBuckets" );
    
    long nRuns = n / nBuckets;
    if( varReduc ) nRuns /= 2;
    double ppg=0, w0=0, q=0, avgSteps=0, ns=0, ng=0, nb=0;
    double avgAvgPpg=0, avgPpgSq=0;
    
    vector<bool> orders;
    orders.push_back(true);
    if( varReduc ) orders.push_back(false);

    for( int bkt=0; bkt<nBuckets; bkt++ )
    {
        if( nBuckets > 1 ) cout << bkt+1 << " ";
        cout.flush();
        
        double subAvgPpg=0;
        
        for( vector<bool>::iterator it=orders.begin(); it!=orders.end(); it++ )
        {
            // run each game in its own thread
            
            if( points.size() < nRuns ) 
            {
                points.resize(nRuns);
                steps.resize(nRuns);
            }
            
            thread_group ts;
            for( long i=0; i<nRuns; i++ )
            {
                if( (*it) )
                    ts.create_thread( worker( i, s1, s2, nRuns, initSeed+bkt*nRuns ) );   
                else
                    ts.create_thread( worker( i, s2, s1, nRuns, initSeed+bkt*nRuns ) );   
            }
            ts.join_all();
            
            int p, ap;
            for( long i=0; i<nRuns; i++ ) 
            {
                p  = points[i];
                if( !(*it) ) p *= -1;
                ap = abs( p );
                
                ppg += p;
                subAvgPpg += p;
                q += ap;
                if( p > 0 ) w0 += 1;
                if( ap == 1 ) ns += 1;
                if( ap == 2 ) ng += 1;
                if( ap == 3 ) nb += 1;
                avgSteps += steps[i];
            }
        }
        
        subAvgPpg /= n/nBuckets;
        avgAvgPpg += subAvgPpg;
        avgPpgSq  += subAvgPpg * subAvgPpg;
    }
    if( nBuckets > 1 ) cout << endl;
    
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
    
    if( nBuckets > 2 )
    {
        avgAvgPpg /= nBuckets;
        avgPpgSq  /= nBuckets;
        
        cout << "Std dev of bkt ppg = " << sqrt( avgPpgSq - avgAvgPpg*avgAvgPpg ) << endl;
        cout << "Std err of ppg     = " << sqrt( ( avgPpgSq - avgAvgPpg*avgAvgPpg ) / ( nBuckets - 1 ) ) << endl << endl;
    }

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

void dispBoard( int ind, bool flipped, strategytdmult& s, const board& b )
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
    else
    {
        gameProbabilities probs( rolloutBoardProbabilitiesParallel(b, s, 4000, 1, 20, false) );
        cout << " --> rollout: " << probs.probWin << "; ( " << probs.probGammonWin << ", " << probs.probGammonLoss << " ); ( " << probs.probBgWin << ", " << probs.probBgLoss << " )";
                                
    }
    cout << endl;
}

void dispBoards( strategytdmult& s )
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
    // try out the TD strategy with multiple networks
    
    //strategytdmult s1( nMiddle, true, true, true );
    //strategytdmult s1( nMiddle, false, false );
    strategytdmult s1( "benchmark", "player32" );
    
    s1.alpha = alpha0;
    s1.beta  = beta0;
    
    int nThreads=16;
    vector< vector<benchmarkData> > dataSetContact( loadBenchmarkData( "/Users/mghiggins/bgtdres/benchdb/contact.bm", nThreads ) );
    vector< vector<benchmarkData> > dataSetCrashed( loadBenchmarkData( "/Users/mghiggins/bgtdres/benchdb/crashed.bm", nThreads ) );
    vector< vector<benchmarkData> > dataSetRace( loadBenchmarkData( "/Users/mghiggins/bgtdres/benchdb/race.bm", nThreads ) );
    
    long minContactInd=-1, minCrashedInd=-1, minRaceInd=-1;
    
    double minContactER = gnuBgBenchmarkER( s1, dataSetContact );
    double minCrashedER = gnuBgBenchmarkER( s1, dataSetCrashed );
    double minRaceER    = gnuBgBenchmarkER( s1, dataSetRace );
    
    double contactER, crashedER, raceER;
    
    string path  = "/Users/mghiggins/bgtdres";
    string fName = path + "/td_comparisonresults_" + fileSuffix + ".csv";
    
    {
        ofstream f;
        f.open( fName.c_str(), fstream::trunc ); // start a new file
        f << 0 << "," << minContactER << "," << minCrashedER << "," << minRaceER << endl;
        f.close();
    }
    
    int nw=0, ng=0, nb=0, ns=0;
    
    for( long i=0; i<20000000; i++ )
    {
        //if( i==300000 or i==800000 or i==1500000 )
        if( i==1000000 or i==3000000 )
        {
            s1.alpha *= 1./sqrt(10);
            s1.beta  *= 1./sqrt(10);
            cout << "\n\n***** Dropped alpha and beta by a factor of sqrt(10)*****\n";
            cout << "alpha = " << s1.alpha << endl;
            cout << "beta  = " << s1.beta  << endl;
            cout << endl;
        }
        s1.learning = true;
        
        game g( &s1, &s1, (int) (i+10000000) );
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
            contactER = gnuBgBenchmarkER( s1, dataSetContact );
            crashedER = gnuBgBenchmarkER( s1, dataSetCrashed );
            raceER    = gnuBgBenchmarkER( s1, dataSetRace );
            
            {
                ofstream f;
                f.open( fName.c_str(), fstream::app ); // append to an existing one
                f << i+1 << "," << contactER << "," << crashedER << "," << raceER << endl;
                f.close();
            }

            if( contactER < minContactER )
            {
                cout << "***** Rolling best contact ER = " << contactER << " vs previous min " << minContactER << "*****\n";
                minContactER = contactER;
                minContactInd = i;
                s1.writeWeights( fileSuffix, "contact" );
            }
            else
                cout << "Prev best contact ER " << minContactER << " at index " << minContactInd+1 << endl;
            if( crashedER < minCrashedER )
            {
                cout << "***** Rolling best crashed ER = " << crashedER << " vs previous min " << minCrashedER << "*****\n";
                minCrashedER = crashedER;
                minCrashedInd = i;
                s1.writeWeights( fileSuffix, "crashed" );
            }
            else
                cout << "Prev best crashed ER " << minCrashedER << " at index " << minCrashedInd+1 << endl;
            if( raceER < minRaceER )
            {
                cout << "***** Rolling best race ER = " << raceER << " vs previous min " << minRaceER << "*****\n";
                minRaceER = raceER;
                minRaceInd = i;
                s1.writeWeights( fileSuffix, "race" );
            }
            else
                cout << "Prev best race ER " << minRaceER << " at index " << minRaceInd+1 << endl;
            cout << endl;
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
    
    strategytdmult s1( "benchmark", "player32" );
    //strategytdorigbg s1( "benchmark", "benchmark2" );
    s1.learning = false;
    //strategyPubEval s1;
    
    /*
    board b( "AABA@AAACDAC@AA=FAAA=A<AAAAC" );
    board nextb( "AAAA@AAACDAC@CA=DAAA=A<ABBAB" );
    cout << moveDiff(b, nextb) << endl;
    return;
    */
    strategyhuman s2(s1);
    
    vector<int> scores;
    int n=10;
    scores.resize(n,0);
    double pw, pgw, pgl, pbw, pbl;
    
    for( int i=0; i<n; i++ )
    {
        cout << "NEW GAME - game # " << i+1 << endl;
        game g(&s2,&s1,i+787);
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
            cout << "Snowie ER                        = " << s2.SnowieER() << endl;
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
    strategytdmult s1( "", "player3_120" );
    //strategytdmult s2( "benchmark", "player24" );
    //strategytdorigbg s2( "benchmark", "benchmark2" );
    strategyPubEval s2;
    s1.learning = false;
    //s2.learning = false;
    
    int nTot=100000;
    int nBkt=100;
    int nStep=nTot/nBkt;
    
    double avgVal=0, avgValSq=0, fracWin=0, fracWinSq=0;
    for( int i=0; i<nBkt; i++ )
    {
        cout << "Bucket " << i << endl;
        runStats stats = playParallelGen( s1, s2, nStep, 1 + i*nStep );
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

void testDoubleHittingShots()
{
    board b;
    b.setFromJosephID("GGGGOIAAEDGCLHANGAAC");
    //b.setPerspective(1);
    b.print();
    
    set<roll> shots( doubleHittingShots(b, true, false,18,23) );
    for( set<roll>::iterator i=shots.begin(); i!=shots.end(); i++ )
        cout << "( " << i->die1 << ", " << i->die2 << " )" << endl;
    cout << doubleHittingProb(b, true, false, 18, 23) << endl;
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
    bool trainCrashed=true;
    bool trainRace=true;
    
    //strategytdmult s1( "benchmark", "player24" );
    //strategytdmult s1( "", "player31_120" );
    //strategytdmult s1( 120, true, true, true, true );
    strategytdmult s1( "", "player35" );
    
    string playerName( "player35sl" );
    string stdName = "std" + playerName;
    cout << "Player name = " << playerName << endl;
    
    int seed = 1;
    
    double alpha;
    double alphaMax=1, alphaMin=0.01;
    
    vector<boardAndRolloutProbs> statesContact, statesCrashed, statesRace;
    
    statesContact = gnuBgBenchmarkStates( "/Users/mghiggins/bgtdres/benchdb/contact-train-data" );
    if( trainCrashed ) statesCrashed = gnuBgBenchmarkStates( "/Users/mghiggins/bgtdres/benchdb/crashed-train-data" );
    if( trainRace) statesRace = gnuBgBenchmarkStates( "/Users/mghiggins/bgtdres/benchdb/race-train-data" );
    
    int nThreads=16;
    vector< vector<benchmarkData> > dataSetContact, dataSetCrashed, dataSetRace;
    dataSetContact = loadBenchmarkData( "/Users/mghiggins/bgtdres/benchdb/contact.bm", nThreads );
    if( trainCrashed ) dataSetCrashed = loadBenchmarkData( "/Users/mghiggins/bgtdres/benchdb/crashed.bm", nThreads );
    if( trainRace ) dataSetRace = loadBenchmarkData( "/Users/mghiggins/bgtdres/benchdb/race.bm", nThreads );
    
    cout << "Starting stats:\n";
    double minContactER, minCrashedER, minRaceER;
    //minContactER = gnuBgBenchmarkER( s1, dataSetContact );
    //if( trainCrashed ) minCrashedER = gnuBgBenchmarkER( s1, dataSetCrashed );
    //if( trainRace ) minRaceER = gnuBgBenchmarkER( s1, dataSetRace );
    cout << endl;
    s1.writeWeights( playerName ); // write the initial set of weights as the best ones; we'll overwrite network by network later
    
    string fileName = "/Users/mghiggins/bgtdres/sl_comparison_results_" + playerName + ".csv";
    ofstream fr( fileName.c_str(), fstream::trunc );
    fr << 0 << "," << minContactER;
    if( trainCrashed ) fr << "," << minCrashedER;
    if( trainRace ) fr << "," << minRaceER;
    fr << endl;
    fr.close();
    
    double contactER, crashedER, raceER;
    int minContactInd=-1, minCrashedInd=-1, minRaceInd=-1;
    
    double lastContactER = minContactER;
    int bestLag=0;
    
    alpha = alphaMax;
    
    for( int i=0; i<30000; i++ )
    {
        if( ( alpha > 0.1 and bestLag >= 2 ) or ( alpha <= 0.1 and bestLag >= 3 ) )
        {
            alpha /= sqrt(10.);
            if( alpha < alphaMin-1e-6 ) alpha = alphaMax;
            cout << "alpha changed to " << alpha << endl;
            bestLag = 0;
        }
        
        cout << endl << "***** Iteration " << i+1 << " ******" << endl;
        cout << "Alpha = " << alpha << endl << endl;
        
        s1.alpha = s1.beta = alpha;
        /*
        trainMultGnuBg( s1, statesContact, seed );
        if( trainCrashed ) trainMultGnuBg( s1, statesCrashed, seed );
        if( trainRace ) trainMultGnuBg( s1, statesRace, seed );
        */
        cout << "Starting training...\n";
        trainMultGnuBgParallel( s1, statesContact, 20 );
        cout << "Done contact training\n";
        if( trainCrashed ) 
        {
            trainMultGnuBgParallel( s1, statesCrashed, 20 );
            cout << "Done crashed training\n";
        }
        if( trainRace ) 
        {
            trainMultGnuBgParallel( s1, statesRace, 20 );
            cout << "Done race training\n";
        }
        
        contactER = gnuBgBenchmarkER( s1, dataSetContact );
        if( trainCrashed ) crashedER = gnuBgBenchmarkER( s1, dataSetCrashed );
        if( trainRace ) raceER = gnuBgBenchmarkER( s1, dataSetRace );
        cout << endl;
        
        fr.open( fileName.c_str(), fstream::app );
        fr << i+1 << "," << contactER;
        if( trainCrashed ) fr << "," << crashedER;
        if( trainRace ) fr << "," << raceER;
        fr << endl;
        fr.close();
        
        s1.writeWeights( stdName );
        
        if( contactER < minContactER )
        {
            cout << "**** Best contact ER " << contactER << " vs previous best " << minContactER << endl;
            minContactER = contactER;
            minContactInd = i;
            s1.writeWeights( playerName, "contact" );
            // also write crashed & race weights if we're not training those so we have a full set
            if( !trainCrashed ) s1.writeWeights( playerName, "crashed" );
            if( !trainRace ) s1.writeWeights( playerName, "race" );
        }
        else
            cout << "Previous best contact ER " << minContactER << " at index " << minContactInd + 1 << endl;
        if( trainCrashed )
        {
            if( crashedER < minCrashedER )
            {
                cout << "**** Best crashed ER " << crashedER << " vs previous best " << minCrashedER << endl;
                minCrashedER = crashedER;
                minCrashedInd = i;
                s1.writeWeights( playerName, "crashed" );
            }
            else
                cout << "Previous best crashed ER " << minCrashedER << " at index " << minCrashedInd + 1 << endl;
        }
        if( trainRace )
        {
            if( raceER < minRaceER )
            {
                cout << "**** Best race ER " << raceER << " vs previous best " << minRaceER << endl;
                minRaceER = raceER;
                minRaceInd = i;
                s1.writeWeights( playerName, "race" );
            }
            else
                cout << "Previous best race ER " << minRaceER << " at index " << minRaceInd + 1 << endl;
        }
        
        if( contactER > lastContactER )
        {
            seed ++;
            cout << "Reseeding - seed now = " << seed << "\n";
            bestLag ++;
        }
        else
            bestLag = 0;
        
        lastContactER = contactER;
    }
    
}

void testBenchmark()
{
    // plot GNUbg benchmark error rate against reference player game performance
    
    strategytdmult s0( "benchmark", "player34" );
    strategytdmult sf( "benchmark", "player32q" );
    strategyply s1(1,8,0.1,s0,sf);
    
    //dispBoards(s1);
    
    //playParallelGen(s1, s2, 10000, 1, 50);
    //return;
    
    hash_map<string,int> ctx;
    //ctx[ "cube" ] = 2;
    //ctx[ "cubeOwner" ] = 1;
    
    int nThreads=16;
    vector< vector<benchmarkData> > dataSetContact( loadBenchmarkData( "/Users/mghiggins/bgtdres/benchdb/contact.bm", nThreads ) );
    vector< vector<benchmarkData> > dataSetCrashed( loadBenchmarkData( "/Users/mghiggins/bgtdres/benchdb/crashed.bm", nThreads ) );
    vector< vector<benchmarkData> > dataSetRace( loadBenchmarkData( "/Users/mghiggins/bgtdres/benchdb/race.bm", nThreads ) );
    
    gnuBgBenchmarkER( s1, dataSetContact, &ctx );
    gnuBgBenchmarkER( s1, dataSetCrashed, &ctx );
    gnuBgBenchmarkER( s1, dataSetRace, &ctx );
    
    //playParallelGen( s1, s2, 40000, 1, 40);
    //playParallelGen( s1, s3, 40000, 1, 40);
}

void testMatchEquity()
{
    /*
    strategytdmult s0( "benchmark", "player32" );
    strategytdmult sf( "benchmark", "player32q" );
    strategyply strat( 1, 8, 0.1, s0, sf );
    
    writeCrawfordFirstDoubleStateProbDb( "/Users/mghiggins/bgtdres/benchdb/matcheq_postC_single.txt", strat, true );
    writeCrawfordFirstDoubleStateProbDb( "/Users/mghiggins/bgtdres/benchdb/matcheq_postC.txt", strat, false );
    */
    /*
    vector<stateData> singleData( loadCrawfordFirstDoubleStateProbDb("/Users/mghiggins/bgtdres/benchdb/matcheq_postC_single.txt") );
    vector<stateData> data( loadCrawfordFirstDoubleStateProbDb("/Users/mghiggins/bgtdres/benchdb/matcheq_postC.txt") );
    
    double pw3=0, pw4=0, pw5=0;
    for( int i=0; i<singleData.size(); i++ )
    {
        pw3 += data.at(i).stateProb * data.at(i).stateGameProbs.probGammonWin;
        pw4 += data.at(i).stateProb * data.at(i).stateGameProbs.probGammonLoss;
        pw5 += data.at(i).stateProb * data.at(i).stateGameProbs.probBgWin;
    }
    
    cout << pw3 << "," << pw4 << "," << pw5 << endl;
    double gamProb = 0.5 * ( pw3 + pw4 );
    //double gamProb=0.1;
    cout << "Gammon probability = " << gamProb << endl;
    
    writeMatchEquityTable(gamProb, singleData, data, "/Users/mghiggins/bgtdres/benchdb/MET.txt" );
    */
    loadMatchEquityTable( "/Users/mghiggins/bgtdres/benchdb/MET.txt" );
    
    for( int i=0; i<12; i++ )
        cout << 100-(matchEquityPostCrawfordCached(i+1)+1)/2.*100 << ",";
    cout << endl << endl;
    
    for( int i=0; i<12; i++ ) 
    {
        for( int j=i+1; j<12; j++ )
            cout << (matchEquityCached(i+1,j+1)+1)/2*100 << ",";
        cout << endl;
    }
    cout << endl;
}

vector<int> pntsCubeful;
vector<int> scoresCubeful;
vector<int> cubesCubeful;
vector<int> stepsCubeful;

class workerCubeful
{
public:
    workerCubeful( long i, strategy& s1, strategy& s2, long n, long initSeed ) : i(i), s1(s1), s2(s2), n(n), initSeed(initSeed) {};
    
    void operator()()
    {
        game g( &s1, &s2, (int)(i+initSeed) );
        g.setTurn( ((int) i)%2 );
        //g.setContextValue( "singleGame", 1 );
        g.stepToEnd();
        double s = g.winnerScore();
        if( g.winner() == 0 )
            pntsCubeful[i] = s;
        else
            pntsCubeful[i] = -s;
        if( g.getCubedOutPlayer() == -1 )
            scoresCubeful[i] = s/g.getCube();
        else
            scoresCubeful[i] = -1; // flag that we cashed out
        cubesCubeful[i] = g.getCube();
        stepsCubeful[i] = g.nSteps;
    }
    
private:
    long i;
    strategy& s1;
    strategy& s2;
    long n;
    long initSeed;
};

runStats playParallelCubeful( strategy& s1, strategy& s2, long n, long initSeed, int nBuckets, bool varReduc, int bktPrintInterval )
{
    using namespace boost;
    
    if( varReduc )
        if( n % (2*nBuckets) != 0 ) throw string( "n must be a multiple of nBuckets*2" );
    else
        if( n % nBuckets != 0 ) throw string( "n must be a multiple of nBuckets" );
    
    long nRuns = n / nBuckets;
    if( varReduc ) nRuns /= 2;
    double ppg=0, w0=0, q=0, avgSteps=0, avgCube=0, ns=0, ng=0, nb=0, nc=0;
    double avgAvgPpg=0, avgPpgSq=0;
    int count=0;
    
    vector<double> cubeFracs(7,0);
    
    if( nBuckets > 1 ) cout << nBuckets << " buckets\n";
    
    vector<bool> orders;
    orders.push_back(true);
    if( varReduc )
        orders.push_back(false);
    
    for( int bkt=0; bkt<nBuckets; bkt++ )
    {
        if( nBuckets > 1 and (bkt+1)%bktPrintInterval == 0 )
            cout << bkt+1 << "; " << ppg/count << "; " << w0/count << "; " << avgCube/count << endl;
        
        double subAvgPpg=0;
        for( vector<bool>::iterator it=orders.begin(); it != orders.end(); it++ )
        {
            // run each game in its own thread
            
            if( pntsCubeful.size() < nRuns ) 
            {
                pntsCubeful.resize(nRuns);
                scoresCubeful.resize(nRuns);
                cubesCubeful.resize(nRuns);
                stepsCubeful.resize(nRuns);
            }
            
            thread_group ts;
            for( long i=0; i<nRuns; i++ ) 
            {
                if( (*it) )
                    ts.create_thread( workerCubeful( i, s1, s2, nRuns, initSeed+bkt*nRuns ) );
                else
                    ts.create_thread( workerCubeful( i, s2, s1, nRuns, initSeed+bkt*nRuns ) );
            }
            ts.join_all();
            
            int p, score;
            for( long i=0; i<nRuns; i++ ) 
            {
                p  = pntsCubeful[i];
                score = scoresCubeful[i];
                if( !(*it) ) p *= -1; // flipped perspective
                
                ppg += p;
                subAvgPpg += p;
                if( p > 0 ) w0 += 1;
                if( score == -1 )
                {
                    q += cubesCubeful[i];
                    nc += 1;
                }
                else
                {
                    q += score;
                    if( score == 1 ) ns += 1;
                    if( score == 2 ) ng += 1;
                    if( score == 3 ) nb += 1;
                }
                avgSteps += stepsCubeful[i];
                avgCube  += cubesCubeful[i];
                count++;
                
                if( cubesCubeful[i] == 1 )
                    cubeFracs[0] ++;
                else if( cubesCubeful[i] == 2 )
                    cubeFracs[1] ++;
                else if( cubesCubeful[i] == 4 )
                    cubeFracs[2] ++;
                else if( cubesCubeful[i] == 8 )
                    cubeFracs[3] ++;
                else if( cubesCubeful[i] == 16 )
                    cubeFracs[4] ++;
                else if( cubesCubeful[i] == 32 )
                    cubeFracs[5] ++;
                else if( cubesCubeful[i] == 64 )
                    cubeFracs[6] ++;
                else
                    throw string( "Invalid cube value" );
            }
        }
        subAvgPpg /= n/nBuckets;
        avgAvgPpg += subAvgPpg;
        avgPpgSq  += subAvgPpg * subAvgPpg;
    }
    if( nBuckets > 1 ) cout << endl;
    
    ppg /= n;
    w0  /= n;
    q   /= n;
    nc  /= n;
    ns  /= n;
    ng  /= n;
    nb  /= n;
    avgSteps /= n;
    avgCube  /= n;
    
    cout << "Average ppg        = " << ppg << endl;
    cout << "Prob of P1 winning = " << w0 * 100 << endl;
    cout << "Average score      = " << q << endl;
    cout << "Frac cashed        = " << nc << endl;
    cout << "Frac single        = " << ns << endl;
    cout << "Frac gammon        = " << ng << endl;
    cout << "Frac backgammon    = " << nb << endl;
    cout << "Average steps/game = " << avgSteps << endl;
    cout << "Average cube       = " << avgCube << endl;
    cout << endl;
    if( nBuckets > 2 )
    {
        avgAvgPpg /= nBuckets;
        avgPpgSq  /= nBuckets;
        
        cout << "Std dev of bkt ppg = " << sqrt( avgPpgSq - avgAvgPpg*avgAvgPpg ) << endl;
        cout << "Std err of ppg     = " << sqrt( ( avgPpgSq - avgAvgPpg*avgAvgPpg ) / ( nBuckets - 1 ) ) << endl << endl;
    }
    
    for( int i=0; i<7; i++ )
        cout << "Cube = " << pow((float)2,(float)i) << ": " << cubeFracs[i]/n << endl;
    
    runStats stats;
    stats.ppg        = ppg;
    stats.fracWin    = w0;
    stats.avgSteps   = avgSteps;
    stats.fracSingle = ns;
    stats.fracGammon = ng;
    stats.fracBg     = nb;
    stats.avgCube    = avgCube;
    
    return stats;
}

void testCubefulMoney()
{
    strategytdmult s1( "benchmark", "player34" );
    strategytdmult s2( "benchmark", "player34" );
    //strategyPubEval s2;
    doublestratjanowski ds( 0.7 );
    //doublestratjanowski ds2( 0.7 );
    //doublestratjumpconst ds( 0.091, false );
    s1.setDoublingStrategy(&ds);
    s2.setDoublingStrategy(&ds);
    s1.useCubefulEquity = true;
    s2.useCubefulEquity = true;
    
    int nRuns = 1000;
    int nBuckets = nRuns/10;
    int seed = 1;
    
    playParallelCubeful( s1, s2, nRuns, seed, nBuckets, true, 1 );
}

vector<int> pntsMatch;
vector<int> gamesMatch;

class workerMatch
{
public:
    workerMatch( int i, bool flip, int target, strategytdmult& s1, strategytdmult& s2, const matchequitytable& MET, int initSeed, double cubeLifeIndex ) : i(i), flip(flip), target(target), s1(s1), s2(s2), MET(MET), initSeed(initSeed), cubeLifeIndex(cubeLifeIndex) {};
    
    void operator()()
    {
        doublestratmatch dsm( MET, cubeLifeIndex );
        
        doublestratmatch dsj( MET, 0.7 );
        //doublestratjanowski dsj(cubeLifeIndex);
        if( flip )
        {
            s1.setDoublingStrategy(&dsm);
            s2.setDoublingStrategy(&dsj);
        }
        else
        {
            s1.setDoublingStrategy(&dsj);
            s2.setDoublingStrategy(&dsm);
        }
        
        s1.useCubefulEquity = true;
        s2.useCubefulEquity = true;
        match m( target, &s1, &s2, i+initSeed );
        dsm.setMatch(&m);
        dsj.setMatch(&m);
        try
        {
            m.stepToEnd();
        }
        catch( const string& e )
        {
            cout << "Exception: " << e << endl;
            throw string( "done" );
        }
        
        if( m.winner() == 0 )
            pntsMatch.at(i) = 1;
        else
            pntsMatch.at(i) = -1;
        gamesMatch.at(i) = m.nGames;
    }
    
private:
    int i;
    bool flip;
    int target;
    strategytdmult s1; // need a separate instance, not a reference; otherwise we try to set different doubling strategy pointers onto the same strategy instance in multiple threads
    strategytdmult s2; // ditto
    const matchequitytable& MET;
    int initSeed;
    double cubeLifeIndex;
};

void testMatch()
{
    using namespace boost;
    
    strategytdmult s1( "benchmark", "player34" );
    strategytdmult s2(s1);
    
    matchequitytable MET( "/Users/mghiggins/bgtdres/benchdb/MET.txt" );
    
    int n=120000;
    int nBuckets=n/1000;
    if( n % (nBuckets*2) != 0 ) throw string( "nRuns must be a multiple of nBuckets*2" );
    int seed=2;
    
    bool varReduc=true;
    
    int nRuns = n / nBuckets;
    if( varReduc ) nRuns/=2;
    bool runParallel=true;
    
    cout << nBuckets << " buckets to process\n";
    
    vector<bool> orders;
    orders.push_back(true);
    if( varReduc ) orders.push_back(false);
    
    vector<double> xs;
    xs.push_back(0.6);
    xs.push_back(0.625);
    xs.push_back(0.65);
    xs.push_back(0.675);
    xs.push_back(0.725);
    xs.push_back(0.75);
    xs.push_back(0.775);
    xs.push_back(0.8);
    
    vector<int> targets;
    targets.push_back(3);
    targets.push_back(5);
    targets.push_back(7);
    //targets.push_back(9);
    
    for( int targetIndex=0; targetIndex<targets.size(); targetIndex++ )
    {
        for( int xind=0; xind<xs.size(); xind++ )
        {
            cout << "Length = " << targets.at(targetIndex) << "; cube life index = " << xs.at(xind ) << endl;
            
            int count=0;
            double ppm=0, winFrac=0, avgGames=0;
            double avgAvgPpm=0, avgPpmSq = 0;
            
            for( int bkt=0; bkt<nBuckets; bkt++ )
            {
                if( nBuckets > 1 and bkt%10 == 0 )
                    cout << bkt+1 << "; " << ppm/count << "; " << avgGames/count << endl;
                
                double subAvgPpm = 0;
                
                for( vector<bool>::const_iterator it=orders.begin(); it!=orders.end(); it++ )
                {
                    // run each match in its own thread
                    
                    if( pntsMatch.size() < nRuns ) 
                    {
                        pntsMatch.resize(nRuns);
                        gamesMatch.resize(nRuns);
                    }
                    
                    if( runParallel )
                    {
                        thread_group ts;
                        for( int i=0; i<nRuns; i++ ) 
                            ts.create_thread( workerMatch( i, (*it), targets.at(targetIndex), s1, s2, MET, seed+bkt*nRuns, xs.at(xind) ) );
                        ts.join_all();
                    }
                    else
                    {
                        for( int i=0; i<nRuns; i++ )
                        {
                            cout << "  Serial " << i << endl;
                            workerMatch w( i, (*it), targets.at(targetIndex), s1, s2, MET, seed+bkt*nRuns, xs.at(xind) );
                            w();
                        }
                    }
                    
                    for( int i=0; i<nRuns; i++ )
                    {
                        double p=pntsMatch.at(i);
                        if( !(*it) ) p *= -1;
                        ppm += p;
                        if( p > 0 ) winFrac++;
                        subAvgPpm += p;
                        avgGames += gamesMatch.at(i);
                        count++;
                    }
                }
                
                subAvgPpm /= n/nBuckets;
                avgAvgPpm += subAvgPpm;
                avgPpmSq += subAvgPpm * subAvgPpm;
            }
            if( nBuckets > 1 ) cout << endl;

            ppm /= n;
            winFrac /= n;
            avgGames /= n;
            cout << "Match length    = " << targets.at(targetIndex) << endl;
            cout << "Cube life index = " << xs.at(xind) << endl;
            cout << "Average player match equity = " << ppm << endl;
            cout << "Odds of win                 = " << winFrac*100 << endl;
            cout << "Average # of games          = " << avgGames << endl;
            
            avgAvgPpm /= nBuckets;
            avgPpmSq  /= nBuckets;
            double stdErrAvgPpm = sqrt( ( avgPpmSq - avgAvgPpm*avgAvgPpm ) / ( nBuckets - 1 ) );
            cout << "Std err on avg ppm          = " << stdErrAvgPpm << endl;
        }
    }
}

vector< vector<double> > jumpVolResults;

class workerJumpVol
{
public:
    workerJumpVol( int i, int initSeed, strategytdmult& strat, bool doTwoStep, const string& evalName ) : i(i), initSeed(initSeed), strat(strat), doTwoStep(doTwoStep), evalName(evalName) {};
    
    void operator()()
    {
        game g(&strat,&strat,i+initSeed);
        double prob, newProb;
        bool doingCalc=false, startCalc=false, isLow, isGood;
        
        vector<double> dProbs;
        
        while( !g.gameOver() )
        {
            g.step();
            if( g.turn() == 0 )
            {
                // get game probabilities before the dice are thrown
                
                board b(g.getBoard());
                b.setPerspective(1);
                gameProbabilities probs( strat.boardProbabilities(b).flippedProbs() );
                
                if( doingCalc )
                {
                    newProb = probs.probWin;
                    dProbs.push_back(newProb-prob);
                }
                
                prob = probs.probWin;
                
                if( ( prob > 0.2 and prob < 0.35 ) or ( prob > 0.65 and prob < 0.8 ) )
                //if( prob > 0.2 and prob < 0.8 )
                {
                    if( doTwoStep and !startCalc )
                    {
                        // check whether it's filtered in based on evaluator game state
                        isGood = false;
                        if( evalName == "" )
                            isGood = true;
                        else
                        {
                            string eval( strat.evaluator(b) );
                            if( evalName == "race" and ( eval == "race" or eval == "bearoff" ) )
                                isGood = true;
                            if( evalName == "contact" and ( eval == "contact" or eval == "crashed" ) )
                                isGood = true;
                        }
                        if( isGood )
                        {
                            startCalc = true;
                            isLow = prob < 0.5;
                        }
                    }
                    else
                    {
                        if( !doTwoStep or ( prob > 0.5 and isLow ) or ( prob < 0.5 and !isLow ) )
                            doingCalc = true;
                    }
                }
                else if( doingCalc )
                    startCalc = doingCalc = false;
            }
        }
        
        jumpVolResults.at(i) = dProbs;
    }
    
private:
    int i;
    int initSeed;
    strategytdmult& strat;
    bool doTwoStep;
    string evalName;
};

void estimateJumpVol()
{
    using namespace boost;
    
    // run cubeless games and calculate moves in prob across 2 plies if 
    // the game goes from a take-ish sort of game to a cash-ish sort of game
    
    strategytdmult strat("benchmark","player33");
    
    int n=100000;
    int nRuns=500;
    int nBuckets=n/nRuns;
    bool doTwoStep=false;
    string evalName="";
    
    double avgAbsJump=0, avgJumpSq=0, avgJump4=0, jump;
    jumpVolResults.resize(nRuns);
    int count=0;
    int i, j;
    
    cout << nBuckets << " buckets to process\n\n";
    
    for( int bkt=0; bkt<nBuckets; bkt++ )
    {
        if( bkt > 0 ) cout << bkt << ": " << avgAbsJump/count << "; " << sqrt(avgJumpSq/count) << "; " << avgJump4/count / pow( avgJumpSq/count, 2 ) << "; " << count << endl;
        
        for( i=0; i<nRuns; i++ ) jumpVolResults.at(i).resize(0);
        
        thread_group ts;
        for( i=0; i<nRuns; i++ ) ts.create_thread( workerJumpVol(i,1+bkt*nRuns,strat,doTwoStep,evalName) );
        ts.join_all();
        
        for( i=0; i<nRuns; i++ )
        {
            for( j=0; j<jumpVolResults.at(i).size(); j++ )
            {
                count++;
                jump = jumpVolResults.at(i).at(j);
                avgAbsJump += fabs( jump );
                avgJumpSq += jump*jump;
                avgJump4 += pow(jump,4);
            }
        }
    }
    
    avgAbsJump /= count;
    avgJump4 /= count;
    avgJumpSq /= count;
    cout << "Average absolute jump size = " << avgAbsJump << endl;
    cout << "Kurtosis normalized        = " << avgJump4/avgJumpSq/avgJumpSq << endl;
    cout << "Jump std dev               = " << sqrt( avgJumpSq ) << endl;
    cout << "Number of points           = " << count << endl;
}

void testContactClustering()
{
    // set up the strategy we'll use for the training
    
    strategytdmult strat( "benchmark", "player34" );
    /*
    board b0("AAAABBAAAAAAAA@BCCACCCA::AAC");
    set<board> moves( possibleMoves(b0,6,2) );
    //board b = strat.preferredBoard(b0, moves);
    board b(b0);
    b.print();
    vector<double> inps( strat.getInputValues(b, "contact") );
    int NI = inps.size();
    cout << "Hitting prob opp    = " << inps[NI-6] << endl;
    cout << "Hitting prob player = " << inps[NI-5] << endl;
    cout << "Escape far player   = " << inps[NI-4] << endl;
    cout << "Escape close player = " << inps[NI-3] << endl;
    cout << "Escape far opp      = " << inps[NI-2] << endl;
    cout << "Escape close opp    = " << inps[NI-1] << endl;
    
    return;
    */
    // tries to identify what input features lead to poor performance vs GNUbg contact benchmark set
    
    int nTrain=1000;
    int nTest=10000;
    int seedTrain=5;
    int seedTest=2;
    double equityThreshold=100e-3;
    int nMiddle=10;

    vector< vector<benchmarkData> > dataSetContact( loadBenchmarkData( "/Users/mghiggins/bgtdres/benchdb/contact.bm", 1 ) );
    const vector<benchmarkData>& data(dataSetContact.at(0));
    int nData=(int)data.size();
    
    // generate the training data. For each element in the training set, calculate the benchmark error as well as strategy
    // inputs.
    
    CRandomMersenne trainRng(seedTrain);
    vector<int> trainIndexes(nTrain);
    vector<double> equityErrors(nTrain);
    vector< vector<double> > trainInputs(nTrain);
    
    int i, j, index, bestIndex;
    bool foundIndex, foundBoard;
    double fracWorst=0;
    
    cout << "Gathering training data\n";
    
    for( i=0; i<nTrain; i++ )
    {
        // get the random index - make sure we don't duplicate
        
        while( true )
        {
            index = trainRng.IRandom(0,nData);
            foundIndex = false;
            for( j=0; j<trainIndexes.size(); j++ )
            {
                if( trainIndexes[j] == index )
                {
                    foundIndex = true;
                    break;
                }
            }
            if( !foundIndex )
                break;
        }
        
        trainIndexes[i] = index;
        
        // get the element
        
        const benchmarkData& elem( data.at(index) );
        
        // calculate the optimal move
        
        set<board> moves( possibleMoves(elem.startBoard, elem.die1, elem.die2 ) );
        board bestMove( strat.preferredBoard(elem.startBoard, moves) );
        
        // calculate the equity error
        
        if( bestMove.equalsFlipped( elem.bestBoard ) )
        {
            equityErrors[i] = 0;
            bestIndex = 0;
        }
        else
        {
            foundBoard = false;
            for( j=0; j<elem.otherBoards.size(); j++ )
                if( elem.otherBoards.at(j).equalsFlipped( bestMove ) )
                {
                    foundBoard = true;
                    equityErrors[i] = elem.bestEquity - elem.otherEquities[j];
                    bestIndex = j+1;
                    break;
                }
            if( !foundBoard )
            {
                equityErrors[i] = elem.bestEquity - elem.otherEquities[elem.otherEquities.size()-1];
                fracWorst++;
                bestIndex = (int)elem.otherBoards.size() + 1;
                //cout << i << ": Best equity = " << elem.bestEquity << "; worst equity = " << elem.otherEquities[elem.otherEquities.size()-1] << endl;
            }
        }
        
        // track the network inputs
        
        trainInputs[i] = strat.getInputValues( elem.startBoard, "contact" );
        
        if( true and equityErrors[i] > equityThreshold )
        {
            cout << "Starting board:\n";
            elem.startBoard.print();
            cout << elem.startBoard.repr() << endl;
            cout << "Roll: ( " << elem.die1 << ", " << elem.die2 << " )\n";
            cout << endl;
            board bestBoard( elem.bestBoard );
            bestBoard.setPerspective(1);
            cout << "Best move:   " << moveDiff( elem.startBoard, bestBoard ) << endl;
            cout << "Chosen move: " << moveDiff( elem.startBoard, bestMove ) << endl;
            if( bestIndex <= elem.otherBoards.size() )
                cout << "  Found at index " << bestIndex << endl;
            else
                cout << "  Not found\n";
            cout << "Equity merr: " << equityErrors[i]*1e3 << endl;
            cout << endl << endl;
        }
    }
    
    // calculate some stats
    
    double avgErr=0;
    double avgErrSq=0;
    double fracZero=0;
    double fracGt=0, avgGt=0;
    for( i=0; i<nTrain; i++ )
    {
        avgErr += equityErrors[i];
        avgErrSq += equityErrors[i]*equityErrors[i];
        if( equityErrors[i] == 0 ) fracZero ++;
        if( equityErrors[i] > equityThreshold ) 
        {
            fracGt ++;
            avgGt += equityErrors[i];
        }
    }
    avgErr /= nTrain;
    avgErrSq /= nTrain;
    fracZero /= nTrain;
    fracGt /= nTrain;
    fracWorst /= nTrain;
    avgGt /= nTrain;
    
    cout << "Average equity milli-error = " << avgErr * 1e3 << endl;
    cout << "Std dev = " << sqrt(avgErrSq-avgErr*avgErr)*1e3 << endl;
    cout << "Fraction zero = " << fracZero * 100 << "%\n";
    cout << "Fraction > " << equityThreshold*1e3 << " = " << fracGt * 100 << "%\n";
    cout << "Cont > " << equityThreshold*1e3 << " = " << avgGt * 1e3 << endl;
    cout << "Fraction worst = " << fracWorst * 100 << "%\n";
    
    return;
    
    double alpha=0.1;
    int nSteps=3000;
    int nInputs=(int)trainInputs[0].size();
    CRandomMersenne rngWeights(1);
    double sum, estOut, trueOut, bit, bit2;
    int k, l, count1, count1Right, count0, count0Right;
    
    // train a neural network to find errors
    
    double midWeight;
    vector<double> middleWeights(nMiddle+1);
    vector< vector<double> > inputWeights(nMiddle);
    for( i=0; i<nMiddle; i++ ) inputWeights.at(i).resize(nInputs+1);
    
    // initialize to random weights
    
    for( i=0; i<nMiddle; i++ )
    {
        middleWeights.at(i) = rngWeights.IRandom(-100, 100)/1000.;
        for( j=0; j<nInputs+1; j++ )
            inputWeights.at(i).at(j) = rngWeights.IRandom(-100, 100)/1000.;
    }
    middleWeights.at(nMiddle) = rngWeights.IRandom(-100, 100)/1000.;
    
    // train the network
    
    for( i=0; i<nSteps; i++ )
    {
        if( i > 0 and i%1000 == 0 and alpha > 0.00101 )
        {
            alpha /= sqrt(10);
            cout << "Epoch " << i << ": alpha changed to " << alpha << endl;
        }
        
        for( j=0; j<nTrain; j++ )
        {
            // get the estimated output based on the weights
            
            vector<double> middles(nMiddle);
            
            vector<double>& inputs( trainInputs.at(j) );
            for( k=0; k<nMiddle; k++ )
            {
                sum=0;
                for( l=0; l<nInputs; l++ ) sum += inputWeights[k][l] * inputs[l];
                sum += inputWeights[k][nInputs];
                middles[k] = 1./(1+exp(-sum));
            }
            
            sum=0;
            for( l=0; l<nMiddle; l++ ) sum += middleWeights[l] * middles[l];
            sum += middleWeights[nMiddle];
            
            estOut = 1./(1+exp(-sum));
            trueOut = equityErrors[j] > equityThreshold ? 1 : 0;
            
            // adjust all the weights
            
            bit = alpha * ( trueOut - estOut ) * estOut * ( 1 - estOut );
            for( k=0; k<nMiddle; k++ ) 
            {
                midWeight = middleWeights[k];
                middleWeights[k] += bit * middles[k];
                bit2 = bit * middles[k] * ( 1 - middles[k] ) * midWeight;
                for( l=0; l<nInputs; l++ )
                    inputWeights[k][l] += bit2 * inputs[l];
                inputWeights[k][nInputs] += bit2;
            }
            middleWeights[nInputs] += bit;
        }
        
        // every so often check the performance
        
        if( i % 250 == 0 or i == nSteps-1 )
        {
            count0 = count1 = count0Right = count1Right = 0;
            for( j=0; j<nTrain; j++ )
            {
                vector<double> middles(nMiddle);
                
                vector<double>& inputs( trainInputs.at(j) );
                for( k=0; k<nMiddle; k++ )
                {
                    sum=0;
                    for( l=0; l<nInputs; l++ ) sum += inputWeights[k][l] * inputs[l];
                    sum += inputWeights[k][nInputs];
                    middles[k] = 1./(1+exp(-sum));
                }
                
                sum=0;
                for( l=0; l<nMiddle; l++ ) sum += middleWeights[l] * middles[l];
                sum += middleWeights[nMiddle];
                
                estOut = 1./(1+exp(-sum));
                trueOut = equityErrors[j] > equityThreshold ? 1 : 0;
                
                if( trueOut == 1 )
                {
                    count1 ++;
                    if( estOut > 0.5 ) count1Right ++;
                }
                else
                {
                    count0 ++;
                    if( estOut <= 0.5 ) count0Right ++;
                }
            }
            cout << "Step " << i+1 << endl;
            cout << "   +1: " << count1Right << " of " << count1 << endl;
            cout << "    0: " << count0Right << " of " << count0 << endl;
            cout << endl;
        }
    }
    
    // generate the testing samples
    
    cout << "Testing data\n";
    
    CRandomMersenne testRng(seedTest);
    vector<int> testIndexes(nTrain);
    
    count1 = count0 = count1Right = count0Right = 0;
    double equityError;
    
    for( i=0; i<nTest; i++ )
    {
        // get the random index - make sure we don't duplicate with other testing indexes or training data
        
        while( true )
        {
            index = testRng.IRandom(0,nData);
            foundIndex = false;
            for( j=0; j<testIndexes.size(); j++ )
            {
                if( testIndexes[j] == index )
                {
                    foundIndex = true;
                    break;
                }
            }
            for( j=0; j<nTrain; j++ )
            {
                if( trainIndexes[j] == index )
                {
                    foundIndex = true;
                    break;
                }
            }
            if( !foundIndex )
                break;
        }
        
        testIndexes[i] = index;
        
        // get the element
        
        const benchmarkData& elem( data.at(index) );
        
        // calculate the optimal move
        
        set<board> moves( possibleMoves(elem.startBoard, elem.die1, elem.die2 ) );
        board bestMove( strat.preferredBoard(elem.startBoard, moves) );
        
        // calculate the equity error
        
        if( bestMove.equalsFlipped( elem.bestBoard ) )
            equityError = 0;
        else
        {
            foundBoard = false;
            for( j=0; j<elem.otherBoards.size(); j++ )
                if( elem.otherBoards.at(j).equalsFlipped( bestMove ) )
                {
                    foundBoard = true;
                    equityError = elem.bestEquity - elem.otherEquities[j];
                    break;
                }
            if( !foundBoard )
                equityError = elem.bestEquity - elem.otherEquities[elem.otherEquities.size()-1];
        }
        
        // calculate the neural net estimate of whether this exceeds the threshold and compare to the true result
        
        vector<double> middles(nMiddle);
        vector<double> inputs( strat.getInputValues(elem.startBoard, "contact") );
        
        for( k=0; k<nMiddle; k++ )
        {
            sum=0;
            for( l=0; l<nInputs; l++ ) sum += inputWeights[k][l] * inputs[l];
            sum += inputWeights[k][nInputs];
            middles[k] = 1./(1+exp(-sum));
        }
        
        sum=0;
        for( l=0; l<nMiddle; l++ ) sum += middleWeights[l] * middles[l];
        sum += middleWeights[nMiddle];
        
        estOut = 1./(1+exp(-sum));
        trueOut = equityError > equityThreshold ? 1 : 0;
        
        if( trueOut == 1 )
        {
            count1 ++;
            if( estOut > 0.5 ) count1Right ++;
        }
        else
        {
            count0 ++;
            if( estOut <= 0.5 ) count0Right ++;
        }
    }
    cout << "+1: " << count1Right << " of " << count1 << endl;
    cout << " 0: " << count0Right << " of " << count0 << endl;
    cout << endl;
}

void testRollout()
{
    board b(referenceBoard(6));
    b.print();
    //return;
    
    strategytdmult s0( "benchmark", "player34" );
    //strategytdorigbg s0( "benchmark", "benchmark2" );
    strategytdmult sf( "benchmark", "player32q" );
    strategyply s(0,8,0.1,s0,sf);
    
    cout << s.boardProbabilities(b) << endl;
    
    double avgProb=0, avgProbSq=0;
    double prob;
    int nTrials=10, nRuns=100;
    for( int i=0; i<nTrials; i++ )
    {
        cout << i << " ";
        cout.flush();
        gameProbabilities probs = rolloutBoardProbabilitiesParallel(b, s, nRuns, i+1, 4, false );
        prob = probs.probBgLoss;
        avgProb += prob;
        avgProbSq += prob * prob;
    }
    cout << endl;
    
    cout << "Average prob            = " << avgProb/nTrials << endl;
    cout << "Regular rollout std dev = " << sqrt( avgProbSq/(nTrials-1) - avgProb*avgProb/nTrials/(nTrials-1) ) << endl;
    cout << endl;
    
    avgProb = avgProbSq = 0;
    for( int i=0; i<10; i++ )
    {
        cout << i << " ";
        cout.flush();
        gameProbabilities probs = rolloutBoardProbabilitiesParallel(b, s, nRuns, i+1, 4, true );
        prob = probs.probBgLoss;
        avgProb += prob;
        avgProbSq += prob * prob;
    }
    cout << endl;
    
    cout << "Average prob            = " << avgProb/nTrials << endl;
    cout << "Varredc rollout std dev = " << sqrt( avgProbSq/(nTrials-1) - avgProb*avgProb/nTrials/(nTrials-1) ) << endl;
    cout << endl;
    
}