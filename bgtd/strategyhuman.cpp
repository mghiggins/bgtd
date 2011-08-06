//
//  strategyhuman.cpp
//  bgtd
//
//  Created by Mark Higgins on 8/5/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <sstream>
#include "strategyhuman.h"

board strategyhuman::preferredBoard( const board& oldBoard, const set<board>& possibleMoves ) const
{
    // keep cycling until they enter a valid move
    
    string input;
    int count;
    vector<string> moveBits(2);
    
    while( true )
    {
        cout << "Enter move: ";
        
        board newBoard( oldBoard ); // start with the old board
        
        // moves need to be entered as eg "8/3 3/1". The bar
        // is called b and off is called o. 
        
        getline( cin, input );
        stringstream ss1( input );
        string s1;
        bool validMove=true;
        while( getline( ss1, s1, ' ' ) )
        {
            if( s1 == "" ) continue; // allows for multiple spaces btw steps
            
            stringstream ss2(s1);
            string s2;
            count = 0;
            while( getline( ss2, s2, '/' ) )
            {
                count += 1;
                if( count > 2 )
                {
                    cout << "Invalid format - must be XX/YY\n";
                    validMove = false;
                    break;
                }
                moveBits[count-1] = s2;
            }
            if( count < 2 )
            {
                cout << "Invalid format - must be XX/YY\n";
                validMove = false;
            }
            else
            {
                cout << moveBits[0] << '-' << moveBits[1] << endl;
                
                if( moveBits[0] == "b" )
                {
                    if( newBoard.hit() == 0 )
                    {
                        cout << "Cannot move a piece down from the board as there are none there.\n";
                        validMove = false;
                    }
                    else
                        newBoard.decrementHit();
                }
                else
                {
                    int index = atoi( moveBits[0].c_str() );
                    if( index < 1 || index > 24 )
                    {
                        cout << "Invalid index - must be between 1 and 24\n";
                        validMove = false;
                    }
                    else if( newBoard.checker(index-1) == 0 )
                    {
                        cout << "Cannot take a piece from index " << index << " as there are none there.\n";
                        validMove = false;
                    }
                    else
                        newBoard.setChecker( index-1, newBoard.checker(index-1) - 1 );
                }
                if( validMove )
                {
                    if( moveBits[1] == "o" )
                        newBoard.incrementBornIn();
                    else
                    {
                        int index = atoi( moveBits[1].c_str() );
                        if( index < 1 || index > 24 )
                        {
                            cout << "Invalid index - must be between 1 and 24\n";
                            validMove = false;
                        }
                        else
                        {
                            if( newBoard.otherChecker(index-1) > 2 )
                            {
                                cout << "Cannot move to index " << index << " as there is an opponent point there\n";
                                validMove = false;
                            }
                            else
                            {
                                newBoard.setChecker( index-1, newBoard.checker(index-1) + 1 );
                                if( newBoard.otherChecker(index-1) == 1 )
                                {
                                    newBoard.setOtherChecker(index-1, 0);
                                    newBoard.incrementOtherHit();
                                }
                            }
                        }
                    }
                }
            }
            if( !validMove ) break; // already printed a message
        }
        
        // check that the new board is in the list of allowed moves
        
        validMove = false;
        for( set<board>::iterator i=possibleMoves.begin(); i!=possibleMoves.end(); i++ )
            if( (*i) == newBoard )
            {
                validMove = true;
                break;
            }
        
        // if the move is valid, return it, otherwise try again
        
        if( validMove ) 
            return newBoard;
        else
            cout << "Proposed move is not a valid move. Please try again.\n";
    }
}