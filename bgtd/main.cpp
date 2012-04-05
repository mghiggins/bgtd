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
    
    nMiddle = 120;
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
        ss << "mult26_" << nMiddle << "_" << alpha0 << "_" << beta0;
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
*/


int main( int argc, char * argv [] )
{
    //testOrigGam();
    //playEscapes();
    //createBenchmarks();
    //trainBenchmarks();
    //testBenchmark();
    //test4();
    //testDoubleHittingShots();
    //testMatchEquity();
    testCubefulMoney();
    //testMatch();
    //estimateJumpVol();
}

