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

#ifndef bgtd_board_h
#define bgtd_board_h

#include <vector>

using namespace std;

class board
{
    // The board defines the backgammon board - where the checkers are, how many have been
    // hit or born in, etc. It does not know anything about the game itself - ie whose turn
    // it is, whether the game is over, and so on. That functionality is deferred to the
    // game class.
    
public:
    // constructors
    
    board();
    board( const board& srcBoard );
    board( const string& brdRepr );
    
    // destructor
    
    ~board();
    
    // perspective describes which player the interface is showing relative to.
    // 0 == white, 1 == black.
    
    int perspective() const;
    void setPerspective( int newPerspective );
    
    // interface to get at board details. eg checkers() returns the list of checkers
    // at each point, with 0 == 1 position in the home quadrant and 23 == furthest
    // position in the other side's home quadrant. otherCheckers() returns the other
    // side's checkers, with the same indexing (so 0 still equals the reference person's
    // 1-spot).
    
    vector<int> checkers() const;
    vector<int> otherCheckers() const;
    int hit() const;
    int otherHit() const;
    int bornIn() const;
    int otherBornIn() const;
    
    int pips() const;
    int otherPips() const;
    
    long pipsq() const;      // pips^2 - sum of (# at spot i)*i^2
    long otherPipsq() const;
    
    int checker( int pos ) const;
    int otherChecker( int pos ) const;
    
    // noBackgammon is true if there's no possibility of a backgammon; false otherwise.
    // otherNoBackgammon is the same for the other player.
    
    bool noBackgammon() const;
    bool otherNoBackgammon() const;
    
    // isRace is true if the board's in a race position
    
    bool isRace() const;
    
    void setChecker( int pos, int num );
    void setOtherChecker( int pos, int num );
    
    void setHit( int hit );
    void setOtherHit( int hit );
    
    void incrementHit();
    void decrementHit();
    void incrementOtherHit();
    void decrementOtherHit();
    
    void incrementBornIn();
    
    void setBornIn( int n );
    void setOtherBornIn( int n );
    
    // setFromJosephID is a function that takes a string representation of the
    // board that Joseph Heled came up with - related to the gnubg position key
    
    void setFromJosephID( const string& ID );
    
    // repr is a string representation of the board
    
    string repr() const;
    
    // comparison operators
    
    bool operator==( const board& otherBoard ) const;
    bool operator>( const board& otherBoard ) const;
    bool operator<( const board& otherBoard ) const;
    bool operator<=( const board& otherBoard ) const;
    bool operator>=( const board& otherBoard ) const;
    
    bool equalsFlipped( const board& otherBoard ) const;
    
    // assignment operator
    
    board& operator=( const board& otherBoard );
    
    void print() const;
    
    void validate() const;
    
    // "raw" interface returns the underlying elements without adjusting
    // for board perspective.
    
    vector<int> checkers0Raw() const { return checkers0; }
    vector<int> checkers1Raw() const { return checkers1; }
    int hit0Raw() const { return hit0; }
    int hit1Raw() const { return hit1; }
    int bornIn0Raw() const { return bornIn0; }
    int bornIn1Raw() const { return bornIn1; }
    
private:
    int         persp;
    vector<int> checkers0;
    vector<int> checkers1;
    int         hit0;
    int         hit1;
    int         bornIn0;
    int         bornIn1;
};


#endif
