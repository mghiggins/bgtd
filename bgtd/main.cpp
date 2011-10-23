//
//  main.cpp
//  bgtd
//
//  Created by Mark Higgins on 7/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <string>
#include <iostream>
#include <sstream>
#include <cmath>
#include "runners.h"
#include "tests.h"
#include "randomc.h"
/*
int main( int argc, char * argv [] )
{
    int nMiddle;
    double alpha0, beta0;
    string fileSuffix, srcSuffix;
    
    // set default values for parameters
    
    nMiddle = 20;
    alpha0  = 0.1;
    beta0   = 0.1;
    
    // override them if params are passed into the command line
    
    cout << argc << endl;
    
    if( argc > 1 )
        nMiddle = atoi( argv[1] );
    if( argc > 2 )
        alpha0 = atof( argv[2] );
    if( argc > 3 )
        beta0 = atof( argv[3] );
    if( argc > 4 )
        fileSuffix = argv[4];
    else
    {
        stringstream ss;
        ss << "exp_" << nMiddle << "_" << alpha0 << "_" << beta0;
        fileSuffix = ss.str();
    }
    if( argc > 5 )
        srcSuffix = argv[5];
    else
    {
        stringstream ss;
        ss << "exp_" << nMiddle << "_0.1_0.1";
        srcSuffix = ss.str();
    }
    
    cout << "nMiddle = " << nMiddle << endl;
    cout << "alpha0  = " << alpha0 << endl;
    cout << "beta0   = " << beta0 << endl;
    cout << "suffix  = " << fileSuffix << endl;
    cout << "src     = " << srcSuffix << endl;
    
    sim4( nMiddle, alpha0, beta0, fileSuffix, srcSuffix );
    return 0;
}

int main( int argc, char * argv [] )
{
    rollTest();
}

int main( int argc, char * argv [] )
{
    test4();
}

*/

#include "bearofffns.h"

long factorial(int n)
{
    double prod=1;
    for( int i=1; i<=n; i++ ) prod *= i;
    return prod;
}

long estElements( int nPnts, int nCheckers )
{
    long e = factorial(nPnts+nCheckers)/factorial(nPnts)/factorial(nCheckers) - 1;
    return e*e;
}

int main( int argc, char * argv [] )
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
    int nCheckers=9;
    cout << estElements( nPnts, nCheckers ) << " est elements\n";
    constructBearoff( nPnts, nCheckers );
    //loadBearoffDb( "/Users/mghiggins/bgtdres/bearoff.txt" );
    
    hash_map<string,double> * m = boardPnts();
    
    //for( hash_map<string,double>::iterator it=(*m).begin(); it!=(*m).end(); it++ )
    //    cout << (*it).first << "   " << (*it).second << endl;
    
    if( m ) cout << m->size() << " elements" << endl;
    
    writeBearoffDb( "/Users/mghiggins/bgtdres/bearoff_6_10.txt" );
}
