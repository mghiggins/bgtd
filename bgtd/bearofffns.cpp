//
//  bearofffns.cpp
//  bgtd
//
//  Created by Mark Higgins on 10/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <fstream>
#include <string>
#include "bearofffns.h"
#include "gamefns.h"

hash_map<string,double> * _boardPnts=0;

string boardID( const board& b, int nPnts )
{
    string idStr("");
    for( int i=0; i<nPnts; i++ )
        idStr += static_cast<char>( b.checker(i)+65 );
    for( int i=23; i>23-nPnts; i-- )
        idStr += static_cast<char>( b.otherChecker(i)+65 );
    return idStr;
}

hash_map<string,double> * boardPnts()
{
    return _boardPnts;
}

double getBoardPnt( const board& b, int nPnts )
{
    // initialize the local list if it hasn't been already
    
    if( _boardPnts == 0 )
        _boardPnts = new hash_map<string,double>;
    
    // if we've already calculated this, return it
    
    string id = boardID( b, nPnts );
    hash_map<string,double>::iterator it = _boardPnts->find( id );
    if( it != _boardPnts->end() ) return (*it).second;
    
    // otherwise calculate it
    
    // run through all the possible rolls
    
    double expPnts=0, weight;
    int i, j;
    
    for( i=1; i<7; i++ )
        for( j=1; j<=i; j++ )
        {
            // get the appropriate probability weight for this roll - twice as big for unmatched dice
            
            if( i == j )
                weight = 1./36;
            else
                weight = 1./18;
            
            // get the possible moves
            
            set<board> moves = possibleMoves( b, i, j );
            
            // figure out the expected points after each move and choose the one with the most positive
            
            double maxPnt=-100, pnt;
            for( set<board>::iterator k=moves.begin(); k!=moves.end(); k++ )
            {
                // if all pieces are off, this player wins; record that as a win or a gammon as appropriate.
                // Otherwise go to the lookup table to get the value.
                
                if( (*k).bornIn() == 15 )
                {
                    if( (*k).otherBornIn() == 0 )
                        pnt = 2.;
                    else
                        pnt = 1.;
                }
                else
                {
                    board newBoard( (*k) );
                    newBoard.setPerspective( 1 - b.perspective() );
                    
                    pnt = -1 * getBoardPnt( newBoard, nPnts );
                }
                
                if( pnt > maxPnt )
                    maxPnt = pnt;
            }
            
            expPnts += weight * maxPnt;
        }
    
    (*_boardPnts)[ id ] = expPnts;
    return expPnts;
}

void constructBearoff( int nPnts, int nCheckers )
{
    // just call getBoardPnt for possible nCheckers board layouts
    
    int nChecker1, nChecker2, i, j;
    double ppg;
    vector<int> checkers1(nCheckers);
    vector<int> checkers2(nCheckers);
    
    for( nChecker1=1; nChecker1<=nCheckers; nChecker1++ )
        for( nChecker2=1; nChecker2<=nCheckers; nChecker2++ )
        {
            // run through possible checker layouts on both sides
            
            // cycle through positions of player 1 first
            
            for( i=0; i<nCheckers; i++ ) checkers1.at(i) = 0;
            
            while(true)
            {
                // then through positions of player 2
                
                for( i=0; i<nCheckers; i++ ) checkers2.at(i) = 0;
                
                while( true )
                {
                    // construct a board with the appropriate checker locations
                    
                    board b;
                    for( i=0; i<24; i++ )
                    {
                        b.setChecker( i, 0 );
                        b.setOtherChecker( i, 0 );
                    }
                    for( i=0; i<nChecker1; i++ ) b.setChecker( checkers1.at(i), b.checker( checkers1.at(i) ) + 1 );
                    for( i=0; i<nChecker2; i++ ) b.setOtherChecker( 23-checkers2.at(i), b.otherChecker( 23-checkers2.at(i) ) + 1 );
                    b.setBornIn(15-nChecker1);
                    b.setOtherBornIn(15-nChecker2);
                    
                    // get the expected # of points - do nothing with it - just caches the value in the hash map
                    
                    ppg = getBoardPnt( b, nPnts );
                    
                    // increment the checker positions
                    
                    i=nChecker2-1;
                    while( true )
                    {
                        checkers2.at(i) += 1;
                        if( checkers2.at(i) == nPnts )
                        {
                            if( i == 0 )
                            {
                                i = -1;
                                break;
                            }
                            else
                            {
                                if( checkers2.at(i-1) < nPnts-1 ) 
                                {
                                    for( j=i; j<nChecker2; j++ ) checkers2.at(j) = checkers2.at(i-1)+1;
                                }
                                else
                                {
                                    checkers2.at(i) = nPnts - 1;
                                }
                            }
                        }
                        else
                            break;
                        i -= 1;
                    }
                    
                    if( i == -1 ) break;
                }
                
                // increment the checker positions
                
                i=nChecker1-1;
                while( true )
                {
                    checkers1.at(i) += 1;
                    if( checkers1.at(i) == nPnts )
                    {
                        if( i == 0 )
                        {
                            i = -1;
                            break;
                        }
                        else
                        {
                            if( checkers1.at(i-1) < nPnts-1 ) 
                            {
                                for( j=i; j<nChecker1; j++ ) checkers1.at(j) = checkers1.at(i-1)+1;
                            }
                            else
                            {
                                checkers1.at(i) = nPnts - 1;
                            }
                        }
                    }
                    else
                        break;
                    i -= 1;
                }
                
                if( i == -1 ) break;
            }
            
        }
}

void writeBearoffDb( const string& fileName )
{
    if( !_boardPnts ) return;
    ofstream f( fileName.c_str() );
    for( hash_map<string,double>::iterator it=_boardPnts->begin(); it!=_boardPnts->end(); it++ )
        f << (*it).first << "," << (*it).second << ",";
    
    f.close();
}

void loadBearoffDb( const string& fileName )
{
    if( !_boardPnts ) _boardPnts = new hash_map<string,double>;
    
    ifstream f( fileName.c_str() );
    string bit, id;
    double val;
    
    while( true )
    {
        getline( f, id, ',' );
        if( id == "" ) break;
        getline( f, bit, ',' );
        if( bit == "" ) break;
        val = atof( bit.c_str() );
        
        (*_boardPnts)[ id ] = val;
    }
}
