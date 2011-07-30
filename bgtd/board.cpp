//
//  board.cpp
//  bgtd
//
//  Created by Mark Higgins on 7/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <algorithm>
#include <iostream>
#include <string>
#include "board.h"

using namespace std;

board::board()
{
    // set up a default board
    
    persp   = 0;
    hit0    = 0;
    hit1    = 0;
    bornIn0 = 0;
    bornIn1 = 0;
    
    checkers0.resize(24,0);
    checkers1.resize(24,0);
    
    checkers0[5]  = 5;
    checkers0[7]  = 3;
    checkers0[12] = 5;
    checkers0[23] = 2;
    
    checkers1[0]  = 2;
    checkers1[11] = 5;
    checkers1[16] = 3;
    checkers1[18] = 5;
    
    if( checkers0.size() != 24 ) throw string( "Regular constructor: checkers0 size is wrong" );
    if( checkers1.size() != 24 ) throw string( "Regular constructor: checkers1 size is wrong" );
}

board::board( const board& srcBoard )
{
    // copy the source board to this one
    
    persp     = srcBoard.persp;
    checkers0 = srcBoard.checkers0;
    checkers1 = srcBoard.checkers1;
    hit0      = srcBoard.hit0;
    hit1      = srcBoard.hit1;
    bornIn0   = srcBoard.bornIn0;
    bornIn1   = srcBoard.bornIn1;
    
    if( checkers0.size() != 24 ) throw string( "Copy constructor: checkers0 size is wrong" );
    if( checkers1.size() != 24 ) throw string( "Copy constructor: checkers1 size is wrong" );
}

board& board::operator=( const board& srcBoard )
{
    if( &srcBoard != this )
    {
        persp     = srcBoard.persp;
        checkers0 = srcBoard.checkers0;
        checkers1 = srcBoard.checkers1;
        hit0      = srcBoard.hit0;
        hit1      = srcBoard.hit1;
        bornIn0   = srcBoard.bornIn0;
        bornIn1   = srcBoard.bornIn1;
        
        if( checkers0.size() != 24 ) throw string( "operator=: checkers0 size is wrong" );
        if( checkers1.size() != 24 ) throw string( "operator=: checkers1 size is wrong" );
    }
    
    return *this;
}

board::~board()
{
}

int board::perspective() const
{
    return persp;
}

void board::setPerspective( int newPerspective )
{
    if( newPerspective == 0 || newPerspective == 1 )
        persp = newPerspective;
    else
        throw string("newPerspective must be 0 or 1");
}

vector<int> board::checkers() const
{
    vector<int> checks;
    if( persp == 0 )
        checks = checkers0;
    else
    {
        checks = checkers1;
        reverse( checks.begin(), checks.end() );
    }
    
    return checks;
}

vector<int> board::otherCheckers() const
{
    vector<int> checks;
    
    if( persp == 0 )
        checks = checkers1;
    else
    {
        checks = checkers0;
        reverse( checks.begin(), checks.end() );
    }
    
    return checks;
}

int board::hit() const
{
    if( persp == 0 )
        return hit0;
    else
        return hit1;
}

int board::otherHit() const
{
    if( persp == 0 )
        return hit1;
    else
        return hit0;
}

int board::bornIn() const
{
    if( persp == 0 )
        return bornIn0;
    else
        return bornIn1;
}

int board::otherBornIn() const
{
    if( persp == 0 )
        return bornIn1;
    else
        return bornIn0;
}

int board::pips() const
{
    int pipCount=0;
    vector<int> checks = checkers();
    if( checks.size() != 24 ) throw string( "Pips: Number of checker spots in vector is not 24" );
    
    for( int i=0; i<24; i++ )
        pipCount += checks.at(i) * (i+1);
    pipCount += 25 * hit();
    
    return pipCount;
}

int board::otherPips() const
{
    int pipCount=0;
    vector<int> checks = otherCheckers();
    if( checks.size() != 24 ) throw string( "Pips: Number of other checker spots in vector is not 24" );
    
    for( int i=0; i<24; i++ )
        pipCount += checks.at(i) * (24-i);
    pipCount += 25 * otherHit();
    
    return pipCount;
}

long board::pipsq() const
{
    int pipCount=0;
    vector<int> checks = checkers();
    if( checks.size() != 24 ) throw string( "Pipsq: Number of checker spots in vector is not 24" );
    
    for( int i=0; i<24; i++ )
        pipCount += checks.at(i) * (i+1) * (i+1);
    pipCount += 25 * 25 * hit();
    
    return pipCount;
}

long board::otherPipsq() const
{
    int pipCount=0;
    vector<int> checks = otherCheckers();
    if( checks.size() != 24 ) throw string( "Pipsq: Number of other checker spots in vector is not 24" );
    
    for( int i=0; i<24; i++ )
        pipCount += checks.at(i) * (24-i) * (24-i);
    pipCount += 25 * 25 * otherHit();
    
    return pipCount;
}

int board::checker( int pos ) const
{
    if( pos < 0 || pos > 23 )
        throw string("Invalid position");
    
    if( persp == 0 )
        return checkers0.at(pos);
    else
        return checkers1.at(23-pos);
}

int board::otherChecker( int pos ) const
{
    if( pos < 0 || pos > 23 )
        throw string("Invalid position");
    
    if( persp == 0 )
        return checkers1.at(pos);
    else
        return checkers0.at(23-pos);
}

void board::setChecker( int pos, int num )
{
    if( pos < 0 || pos > 23 )
        throw string("Invalid position");
    
    if( persp == 0 )
        checkers0.at(pos) = num;
    else
        checkers1.at(23-pos) = num;
}

void board::setOtherChecker( int pos, int num )
{
    if( pos < 0 || pos > 23 )
        throw string("Invalid position");
    
    if( persp == 0 )
        checkers1.at(pos) = num;
    else
        checkers0.at(23-pos) = num;
}

void board::incrementHit()
{
    if( persp == 0 )
        hit0 += 1;
    else
        hit1 += 1;
}

void board::decrementHit()
{
    if( persp == 0 )
    {
        if( hit0 == 0 )
            throw string("Cannot decrement - already zero");
        hit0 -= 1;
    }
    else
    {
        if( hit1 == 0 )
            throw string("Cannot decrement - already zero");
        hit1 -= 1;
    }
}

void board::incrementOtherHit()
{
    if( persp == 0 )
        hit1 += 1;
    else
        hit0 += 1;
}

void board::decrementOtherHit()
{
    if( persp == 0 )
    {
        if( hit1 == 0 )
            throw string("Cannot decrement - already zero");
        hit1 -= 1;
    }
    else
    {
        if( hit0 == 0 )
            throw string("Cannot decrement - already zero");
        hit0 -= 1;
    }
}

void board::incrementBornIn()
{
    if( persp == 0 )
        bornIn0 += 1;
    else
        bornIn1 += 1;
}

bool board::operator==( const board& otherBoard ) const
{
    if( persp != otherBoard.persp ) return false;
    if( hit0 != otherBoard.hit0 ) return false;
    if( hit1 != otherBoard.hit1 ) return false;
    if( bornIn0 != otherBoard.bornIn0 ) return false;
    if( bornIn1 != otherBoard.bornIn1 ) return false;
    
    int i;
    for( i=0; i<24; i++ )
    {
        if( checkers0[i] != otherBoard.checkers0[i] ) return false;
        if( checkers1[i] != otherBoard.checkers1[i] ) return false;
    }
    
    return true;
}

bool board::operator<( const board& otherBoard ) const
{
    // there's no real way to determine whether a board is "smaller" than another
    // board, but we want a simple way of consistently defining this so we can
    // order boards in sets and the like. We use a fairly arbitrary definition based
    // on the layout.
    
    if( this->operator==( otherBoard ) ) return false;
    
    if( bornIn0 < otherBoard.bornIn0 ) return true;
    if( bornIn1 > otherBoard.bornIn1 ) return false;
    if( hit0 > otherBoard.hit0 ) return true;
    if( hit1 < otherBoard.hit1 ) return false;
    
    int i;
    
    for( i=0; i<24; i++ )
    {
        if( checkers0[i] < otherBoard.checkers0[i] )
            return true;
        else if( checkers0[i] > otherBoard.checkers0[i] )
            return false;
    }
    for( i=0; i<24; i++ )
    {
        if( checkers1[i] > otherBoard.checkers1[i] )
            return true;
        else if( checkers1[i] < otherBoard.checkers1[i] )
            return false;
    }
    
    return false;
}

bool board::operator>( const board& otherBoard ) const
{
    return !operator<=( otherBoard );
}

bool board::operator<=( const board& otherBoard ) const
{
    return operator==( otherBoard ) || operator<( otherBoard );
}

bool board::operator>=( const board& otherBoard ) const
{
    return !operator<( otherBoard );
}

void board::print() const
{
    cout << '|';
    int i, j;
    for( i=0; i<5; i++ ) cout << "| ";
    cout << "||";
    for( i=0; i<5; i++ ) cout << "| ";
    cout << "||\n";
    for( j=0; j<5; j++ )
    {
        cout << '|';
        for( i=0; i<12; i++ )
        {
            if( checker(23-i) >= j+1 )
                cout << 'o';
            else if( otherChecker(23-i) >= j+1 )
                cout << '*';
            else
                cout << ' ';
            if( i == 5 )
                cout << '|';
            else if( i == 11 )
                cout << "|\n";
            else
                cout << ' ';
        }
    }
    cout << '|';
    for( i=0; i<12; i++ )
    {
        if( checker(23-i) > 5 )
            cout << checker(23-i);
        else if( otherChecker(23-i) > 5 )
            cout << otherChecker(23-i);
        else
            cout << ' ';
        if( i == 5 )
            cout << '|';
        else if( i == 11 )
            cout << "|\n";
        else
            cout << ' ';
    }
    
    // then the bottom half of the board
    
    cout << '|';
    for( i=0; i<12; i++ )
    {
        cout << ' ';
        if( i == 5 )
            cout << '|';
        else if( i == 11 )
            cout << "|\n";
        else
            cout << ' ';
    }
    
    cout << '|';
    
    for( i=0; i<12; i++ )
    {
        if( checker(i) > 5 )
            cout << checker(i);
        else if( otherChecker(i) > 5 )
            cout << otherChecker(i);
        else
            cout << ' ';
        if( i == 5 )
            cout << '|';
        else if( i == 11 )
            cout << "|\n";
        else
            cout << ' ';
    }
    
    for( j=4; j>=0; j-- )
    {
        cout << '|';
        for( i=0; i<12; i++ )
        {
            if( checker(i) >= j+1 )
                cout << 'o';
            else if( otherChecker(i) >= j+1 )
                cout << '*';
            else
                cout << ' ';
            if( i == 5 )
                cout << '|';
            else if( i == 11 )
                cout << "|\n";
            else
                cout << ' ';
        }
    }
    cout << '|';
    for( i=0; i<5; i++ ) cout << "| ";
    cout << "||";
    for( i=0; i<5; i++ ) cout << "| ";
    cout << "||\n";
    
    if( hit() > 0 )
        cout << "o hit = " << hit() << "\n";
    if( otherHit() > 0 )
        cout << "* hit = " << otherHit() << "\n";
    if( bornIn() > 0 )
        cout << "o born in = " << bornIn() << "\n";
    if( otherBornIn() > 0 )
        cout << "* born in = " << otherBornIn() << "\n";
}

void board::validate() const
{
    // checks whether the board is a possible board
    
    int i, count=0, otherCount=0;
    for( i=0; i<24; i++ )
    {
        count += checker(i);
        otherCount += otherChecker(i);
        if( checker(i) > 0 && otherChecker(i) > 0 ) 
            throw string("A point has a checker from either side");
        if( checker(i) < 0 ) throw string("A point has checker < 0");
        if( otherChecker(i) < 0 ) throw string("A point has otherChecker < 0");
    }
    if( hit() < 0 ) throw string("hit is less than zero");
    if( otherHit() < 0 ) throw string("otherHit is less than zero");
    if( bornIn() < 0 ) throw string("bornIn is less than zero");
    if( otherBornIn() < 0 ) throw string("otherBornIn is less than zero");
    count += hit();
    otherCount += otherHit();
    count += bornIn();
    otherCount += otherBornIn();
    
    if( count != 15 ) throw string("count is not 15");
    if( otherCount != 15 ) throw string("otherCount is not 15");
}