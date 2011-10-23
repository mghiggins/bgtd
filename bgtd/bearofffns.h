//
//  bearofffns.h
//  bgtd
//
//  Created by Mark Higgins on 10/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_bearofffns_h
#define bgtd_bearofffns_h

#import <hash_map.h>
#import "board.h"

// define a hash fn for string so that hash_map can use strings as keys

namespace __gnu_cxx                                                                                 
{                                                                                             
    template<> struct hash< std::string >                                                       
    {                                                                                           
        size_t operator()( const std::string& x ) const                                           
        {                                                                                         
            return hash< const char* >()( x.c_str() );                                              
        }                                                                                         
    };                                                                                          
}   

// boardID returns a unique ID for the board (from the perspective of 
// the board, assuming the player with perspective has the dice), given
// that we're doing two-sided bearoff for nPnts points.

string boardID( const board& b, int nPnts );

// boardPnts returns a map of board ID->expected number of points the
// player with the perspective will win (negative if he expects to lose).
// This map is filled in either by loading from a file or dynamically.

hash_map<string,double> * boardPnts();

// getBoardPnt calculates the expected number of points the player with
// perpective will win by doing a full rollout. Uses existing elements
// in the boardPnts hash if they're there, otherwise it recursively fills
// them in. Always fills in the point in the hash for this board.

double getBoardPnt( const board& b, int nPnts );

// constructBearoff constructs the bearoff database out to nPnts points
// and nCheckers max checkers. Results are stored in the local board ID
// hash.

void constructBearoff( int nPnts, int nCheckers );

// writeBearoffDb writes the data in the internal hash map to a file

void writeBearoffDb( const string& fileName );

// loadBearoffDb reads the bearoff data from a file into the internal hash map

void loadBearoffDb( const string& fileName );


#endif
