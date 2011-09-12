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
    
    sim5( nMiddle, alpha0, beta0, fileSuffix, srcSuffix );
    return 0;
}
/*
int main( int argc, char * argv [] )
{
    rollTest();
}

int main( int argc, char * argv [] )
{
    test4();
}
*/