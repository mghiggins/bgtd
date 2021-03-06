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
        pipCount += checks[i] * (i+1);
    pipCount += 25 * hit();
    
    return pipCount;
}

int board::otherPips() const
{
    int pipCount=0;
    vector<int> checks = otherCheckers();
    if( checks.size() != 24 ) throw string( "Pips: Number of other checker spots in vector is not 24" );
    
    for( int i=0; i<24; i++ )
        pipCount += checks[i] * (24-i);
    pipCount += 25 * otherHit();
    
    return pipCount;
}

long board::pipsq() const
{
    int pipCount=0;
    vector<int> checks = checkers();
    if( checks.size() != 24 ) throw string( "Pipsq: Number of checker spots in vector is not 24" );
    
    for( int i=0; i<24; i++ )
        pipCount += checks[i] * (i+1) * (i+1);
    pipCount += 25 * 25 * hit();
    
    return pipCount;
}

long board::otherPipsq() const
{
    int pipCount=0;
    vector<int> checks = otherCheckers();
    if( checks.size() != 24 ) throw string( "Pipsq: Number of other checker spots in vector is not 24" );
    
    for( int i=0; i<24; i++ )
        pipCount += checks[i] * (24-i) * (24-i);
    pipCount += 25 * 25 * otherHit();
    
    return pipCount;
}

int board::checker( int pos ) const
{
    if( pos < 0 || pos > 23 )
        throw string("Invalid position");
    
    if( persp == 0 )
        return checkers0[pos];
    else
        return checkers1[23-pos];
}

int board::otherChecker( int pos ) const
{
    if( pos < 0 || pos > 23 )
        throw string("Invalid position");
    
    if( persp == 0 )
        return checkers1[pos];
    else
        return checkers0[23-pos];
}

bool board::noBackgammon() const
{
    if( bornIn() > 0 ) return true; // if the player has borne in any checkers, impossible to be backgammoned
    if( hit() > 0 ) return false;   // if the player has pieces hit, it's possible to be backgammoned
    
    // if it's not a race, still possible to get hit & sent back
    if( not isRace() ) return false;
    
    // if the player has any checkers in the opponent's home box, it's possible to be backgammoned
    for( int i=18; i<24; i++ )
        if( checker(i) > 0 )
            return false;
    
    // otherwise it's no longer possible to be backgammoned
    return true;
}

bool board::otherNoBackgammon() const
{
    if( otherBornIn() > 0 ) return true; // if the player has borne in any checkers, impossible to be backgammoned
    if( otherHit() > 0 ) return false;   // if the player has pieces hit, it's possible to be backgammoned
    
    // if it's not a race, still possible to get hit & sent back
    if( not isRace() ) return false;
    
    // if the player has any checkers in the opponent's home board, it's possible to be backgammoned
    for( int i=0; i<6; i++ )
        if( otherChecker(i) > 0 )
            return false;
    
    // otherwise it's no longer possible to be backgammoned
    return true;
}

bool board::isRace() const
{
    // if the game is over, it counts as a race
    
    if( bornIn() == 15 or otherBornIn() == 15 ) return true;
    
    // if either player is hit, it's not a race
    
    if( hit() > 0 or otherHit() > 0 ) return false;
    
    // find the index for the furthest of the player's pieces
    
    int i;
    for( i=23; i>=0; i-- )
        if( checker(i) > 0 ) break;
    
    // then check if there are any opponent pieces before that one
    
    for( int j=0; j<i; j++ )
        if( otherChecker(j) > 0 ) return false;
    
    // if not, it's a race
    
    return true;
}

void board::setChecker( int pos, int num )
{
    if( pos < 0 || pos > 23 )
        throw string("Invalid position");
    
    if( persp == 0 )
        checkers0[pos] = num;
    else
        checkers1[23-pos] = num;
}

void board::setOtherChecker( int pos, int num )
{
    if( pos < 0 || pos > 23 )
        throw string("Invalid position");
    
    if( persp == 0 )
        checkers1[pos] = num;
    else
        checkers0[23-pos] = num;
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

void board::setBornIn( int n )
{
    if( persp == 0 )
        bornIn0 = n;
    else
        bornIn1 = n;
}

void board::setOtherBornIn( int n )
{
    if( persp == 0 )
        bornIn1 = n;
    else
        bornIn0 = n;
}

void board::setHit( int hit )
{
    if( persp == 0 )
        hit0 = hit;
    else
        hit1 = hit;
}

void board::setOtherHit( int hit )
{
    if( persp == 0 )
        hit1 = hit;
    else
        hit0 = hit;
}

board::board( const string& brdRepr )
{
    // deserialize the board from the string representation
    
    persp = 0;
    
    bornIn0 = static_cast<int>( brdRepr[0] ) - 65;
    bornIn1 = static_cast<int>( brdRepr[1] ) - 65;
    hit0    = static_cast<int>( brdRepr[2] ) - 65;
    hit1    = static_cast<int>( brdRepr[3] ) - 65;
    
    checkers0.resize(24,0);
    checkers1.resize(24,0);
    
    int nCheck;
    
    for( int i=0; i<24; i++ )
    {
        nCheck = static_cast<int>( brdRepr[4+i] ) - 65;
        if( nCheck > 0 )
            checkers0[i] = nCheck;
        else
            checkers1[i] = -nCheck;
    }
}

string board::repr() const
{
    vector<int> checks( checkers() );
    vector<int> otherChecks( otherCheckers() );
    
    string r;
    r += static_cast<char>( bornIn()+65 );
    r += static_cast<char>( otherBornIn()+65 );
    r += static_cast<char>( hit() + 65 );
    r += static_cast<char>( otherHit() + 65 );
    
    for( int i=0; i<24; i++ )
        r += static_cast<char>( checks[i] - otherChecks[i] + 65 );
    
    return r;
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

bool board::equalsFlipped( const board& otherBoard ) const
{
    if( hit0 != otherBoard.hit1 ) return false;
    if( hit1 != otherBoard.hit0 ) return false;
    if( bornIn0 != otherBoard.bornIn1 ) return false;
    if( bornIn1 != otherBoard.bornIn0 ) return false;
    
    int i;
    for( i=0; i<24; i++ )
    {
        if( checkers0[i] != otherBoard.checkers1[23-i] ) return false;
        if( checkers1[i] != otherBoard.checkers0[23-i] ) return false;
    }
    
    return true;
}

bool board::operator<( const board& otherBoard ) const
{
    // there's no real way to determine whether a board is "smaller" than another
    // board, but we want a simple way of consistently defining this so we can
    // order boards in sets and the like. We use a fairly arbitrary definition based
    // on the layout.
    
    if( persp < otherBoard.persp ) return true;
    if( persp > otherBoard.persp ) return false;
    
    if( bornIn0 < otherBoard.bornIn0 ) return true;
    if( bornIn0 > otherBoard.bornIn0 ) return false;

    if( bornIn1 > otherBoard.bornIn1 ) return false;
    if( bornIn1 < otherBoard.bornIn1 ) return true;
    
    if( hit0 > otherBoard.hit0 ) return true;
    if( hit0 < otherBoard.hit0 ) return false;
    
    if( hit1 < otherBoard.hit1 ) return false;
    if( hit1 > otherBoard.hit1 ) return true;
    
    for( int i=0; i<24; i++ )
    {
        if( checkers0[i] < otherBoard.checkers0[i] ) return true;
        if( checkers0[i] > otherBoard.checkers0[i] ) return false;
        if( checkers1[i] > otherBoard.checkers1[i] ) return true;
        if( checkers1[i] < otherBoard.checkers1[i] ) return false;
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

void board::setFromJosephID( const string& ID )
{
    if( ID.size() != 20 ) throw string( "The Joseph ID must have 20 characters" );
    
    // first translate the 20-character ID into a 10-bit gnubg position key
    
    unsigned char bits[10];
    for( int i=0; i<10; i++ ) 
        bits[i] = ((ID[2*i+0] - 'A') << 4) +  (ID[2*i+1] - 'A');
	
    // start with an empty board
    
    for( int i=0; i<24; i++ )
    {
        checkers0[i] = 0;
        checkers1[i] = 0;
    }
    bornIn0 = bornIn1 = 0;
    hit0 = hit1 = 0;
    persp = 0;
    
    // translate the 10-bit key into the board layout. This is largely copying the
    // gnubg code in positionid.c, function oldPositionFromKey.
    
    const unsigned char* a;
    int i=0, j=0, k;
    for( a=bits; a<bits+10; a++ )
    {
        unsigned char cur = *a;
        for( k=0; k<8; k++ )
        {
            if( ( cur & 0x1 ) )
            {
                if( i>=2 or j>=25 ) throw string( "Invalid state - something went wrong in parsing position key" );
                if( j<24 )
                {
                    if( i == 0 )
                        checkers1[23-j] ++;
                    else
                        checkers0[j] ++;
                }
                else
                {
                    if( i == 0 )
                        hit1 ++;
                    else
                        hit0 ++;
                }
            }
            else
            {
                if( ++j == 25 )
                {
                    i++;
                    j = 0;
                }
            }
            cur >>= 1;
        }
    }
    
    // set the number borne in correctly
    
    int tot0=hit0, tot1=hit1;
    for( int i=0; i<24; i++ )
    {
        tot0 += checkers0[i];
        tot1 += checkers1[i];
    }
    bornIn0 = 15 - tot0;
    bornIn1 = 15 - tot1;
}