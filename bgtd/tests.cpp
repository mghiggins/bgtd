//
//  tests.cpp
//  bgtd
//
//  Created by Mark Higgins on 7/29/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <set>
#include "tests.h"
#include "board.h"
#include "gamefns.h"

bool rollTest()
{
    // first test: check a 2-1 from the starting posn
    
    cout << "Roll 2-1 from a starting board\n";
    cout << "==============================\n";
    board b;
    cout << "starting board:\n";
    b.print();
    cout << endl;
    cout << "possible boards:\n";
    int d1=1, d2=2;
    set<board> s = possibleMoves( b, d1, d2 );
    for( set<board>::iterator i=s.begin(); i!=s.end(); i++ )
        (*i).print();
    cout << "Number of boards = " << s.size() << endl;
    if( s.size() != 15 ) return false;
    cout << endl << endl;
    
    // next tests: bearing in one side
    
    b.setChecker( 0, 5 );
    b.setChecker( 1, 8 );
    b.setChecker( 3, 2 );
    for( int i=4; i<24; i++ ) b.setChecker( i, 0 );
    b.setOtherChecker( 0, 0 );
    b.setOtherChecker( 11, 7 );
    cout << endl;
    cout << "Roll 6-6 while bearing in\n";
    cout << "=========================\n";
    cout << "starting board:\n";
    b.print();
    cout << endl;
    cout << "possible boards:\n";
    d1 = 6;
    d2 = 6;
    s = possibleMoves( b, d1, d2 );
    for( set<board>::iterator i=s.begin(); i!=s.end(); i++ )
        (*i).print();
    if( s.size() != 1 ) return false;
    cout << endl;
    cout << "Same but rolling 2-1\n";
    d1 = 2;
    d2 = 1;
    s = possibleMoves( b, d1, d2 );
    for( set<board>::iterator i=s.begin(); i!=s.end(); i++ )
        (*i).print();
    if( s.size() != 6 ) return false;
    cout << endl;
    cout << "Finishing the game while bearing in\n";
    for( int i=0; i<12; i++ ) b.incrementBornIn();
    b.setChecker( 1, 0 );
    b.setChecker( 3, 0 );
    b.setChecker( 0, 3 );
    cout << endl;
    cout << "Bearing off near the end\n";
    cout << "========================\n";
    cout << "starting board\n";
    b.print();
    cout << "Bearing off with 2-2: possible boards:\n";
    s = possibleMoves( b, 2, 2 );
    for( set<board>::iterator i=s.begin(); i!=s.end(); i++ )
        (*i).print();
    if( s.size() != 1 ) return false;
    cout << endl;
    cout << "Bearing off with 1-1: possible moves:\n";
    s = possibleMoves( b, 2, 2 );
    for( set<board>::iterator i=s.begin(); i!=s.end(); i++ )
        (*i).print();
    if( s.size() != 1 ) return false;
    cout << endl;
    cout << "Bearing off with 6-5: possible moves:\n";
    s = possibleMoves( b, 6, 5 );
    for( set<board>::iterator i=s.begin(); i!=s.end(); i++ )
        (*i).print();
    if( s.size() != 1 ) return false;
    
    
    return true;
}
