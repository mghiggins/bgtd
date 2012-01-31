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

#include <iostream>
#include <sstream>
#include "strategyhuman.h"

board strategyhuman::preferredBoard( const board& oldBoard, const set<board>& possibleMoves, const hash_map<string,int>* context ) const
{
    // if there are no moves or just one move, apply them, but put an input break in
    // so the user sees the move happening.
    
    string input;
    
    if( possibleMoves.size() < 2 )
    {
        cout << "No choice on move. Hit enter to continue.\n";
        getline( cin, input );
        if( possibleMoves.size() == 0 )
            return oldBoard;
        else
            return *(possibleMoves.begin());
    }
    
    // keep cycling until they enter a valid move
    
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