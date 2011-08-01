//
//  main.cpp
//  bgtd
//
//  Created by Mark Higgins on 7/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "runners.h"
#include <string>
#include <iostream>
#include <cmath>

int main( int argc, char * argv [] )
{
    int nMiddle;
    double alpha0, beta0;
    string fileSuffix;
    
    // set default values for parameters
    
    nMiddle = 40;
    alpha0  = 0.5;
    beta0   = 0.5;
    fileSuffix = "";
    
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
    
    cout << "nMiddle = " << nMiddle << endl;
    cout << "alpha0  = " << alpha0 << endl;
    cout << "beta0   = " << beta0 << endl;
    cout << "suffix  = " << fileSuffix << endl;
    
    sim1( nMiddle, alpha0, beta0, fileSuffix );
    //test1();
    return 0;
}