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


int main( int argc, char * argv [] )
{
    int nMiddle;
    double alpha0, beta0;
    string fileSuffix, srcSuffix;
    
    // set default values for parameters
    
    nMiddle = 80;
    alpha0  = 0.1;
    beta0   = 1 * alpha0;
    
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
        ss << "mult_" << nMiddle << "_" << alpha0 << "_" << beta0;
        fileSuffix = ss.str();
    }
    if( argc > 5 )
        srcSuffix = argv[5];
    else
    {
        stringstream ss;
        ss << "exp_80_0.1_0.1";
        srcSuffix = ss.str();
    }
    
    cout << "nMiddle = " << nMiddle << endl;
    cout << "alpha0  = " << alpha0 << endl;
    cout << "beta0   = " << beta0 << endl;
    cout << "suffix  = " << fileSuffix << endl;
    cout << "src     = " << srcSuffix << endl;
    
    sim6( nMiddle, alpha0, beta0, fileSuffix, srcSuffix );
    
    return 0;
}

/*
int main( int argc, char * argv [] )
{
    // train strategytdoriggam
    
    int nMiddle;
    double alpha0, beta0;
    string fileSuffix, srcSuffix;
    
    // set default values for parameters
    
    nMiddle = 80;
    alpha0  = 0.1;
    beta0   = 1 * alpha0;
    
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
        ss << "bg_" << nMiddle << "_" << alpha0 << "_" << beta0;
        fileSuffix = ss.str();
    }
    if( argc > 5 )
        srcSuffix = argv[5];
    else
    {
        stringstream ss;
        ss << "bg_80_0.1_0.1";
        srcSuffix = ss.str();
    }
    
    cout << "nMiddle = " << nMiddle << endl;
    cout << "alpha0  = " << alpha0 << endl;
    cout << "beta0   = " << beta0 << endl;
    cout << "suffix  = " << fileSuffix << endl;
    cout << "src     = " << srcSuffix << endl;
    
    sim9( nMiddle, alpha0, beta0, fileSuffix, srcSuffix );
    
    return 0;
}

int main( int argc, char * argv [] )
{
    int nMiddle;
    
    double alpha0, beta0;
    string fileSuffix, srcSuffix;
    
    // set default values for parameters
    
    nMiddle = 40;
    alpha0  = 0.1;
    beta0   = 1 * alpha0;
    
    if( argc > 1 )
        nMiddle = atoi( argv[1] );
    if( argc > 2 )
        alpha0 = atof( argv[2] );
    if( argc > 3 )
        beta0 = atof( argv[3] );

    cout << "nMiddle = " << nMiddle << endl;
    cout << "alpha0  = " << alpha0 << endl;
    cout << "beta0   = " << beta0 << endl;
    
    sim7( nMiddle, alpha0, beta0 );
    return 0;
}

int main( int argc, char * argv [] )
{
    rollTest();
}

int main( int argc, char * argv [] )
{
    testOrigGam();
}

int main( int argc, char * argv [] )
{
    testHittingShots();
}

int main( int argc, char * argv [] )
{
    //compareBearoff();
    playBearoffOneSided();
    return 0;
}

int main( int argc, char * argv [] )
{
    initscr();
    sleep(2);
    timeout(0);
    //while( true )
    //{
        int i = getch();
        //if( i == -1 ) continue;
    printw( "%d\n", i );
        //break;
    //}
    refresh();
    sleep(2);
    endwin();
    return 0;
}
*/
