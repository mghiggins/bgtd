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
#include "gamefns.h"
#include "strategyhuman.h"

class humanBoardProbsPair
{
public:
    humanBoardProbsPair( const board& b, const gameProbabilities& probs ) : b(b), probs(probs) {};
    
    board b;
    gameProbabilities probs;
};

bool humanBoardProbsPairCompare( const humanBoardProbsPair& p1, const humanBoardProbsPair& p2 );
bool humanBoardProbsPairCompare( const humanBoardProbsPair& p1, const humanBoardProbsPair& p2 ) { return p1.probs.equity() > p2.probs.equity(); };

board strategyhuman::preferredBoard( const board& oldBoard, const set<board>& possibleMoves, const hash_map<string,int>* context )
{
    // if there are no moves or just one move, apply them, but put an input break in
    // so the user sees the move happening.
    
    string input;
    
    if( possibleMoves.size() < 2 )
    {
        cout << "No choice on move. Hit enter to continue.\n";
        getline( cin, input );
        nMoves ++;
        if( possibleMoves.size() == 0 )
            return oldBoard;
        else
            return *(possibleMoves.begin());
    }
    
    // keep cycling until they enter a valid move
    
    int count;
    vector<string> moveBits(2);
    
    // use the guide strategy to value the moves - we'll use this to calculate equity error
    // or to show hints.
    
    gameProbabilities probs;
    vector<humanBoardProbsPair> pairs;
    for( set<board>::const_iterator it=possibleMoves.begin(); it!=possibleMoves.end(); it++ )
        pairs.push_back( humanBoardProbsPair( (*it), guideStrat.boardProbabilities((*it)) ) );
    sort( pairs.begin(), pairs.end(), humanBoardProbsPairCompare );
    
    while( true )
    {
        cout << "Enter move: ";
        
        board newBoard( oldBoard ); // start with the old board
        
        // moves need to be entered as eg "8/3 3/1". The bar
        // is called b and off is called o. 
        
        getline( cin, input );
        if( input == "hint" )
        {
            //cout << oldBoard.repr() << endl;
            int nMax = 3;
            if( nMax > pairs.size() ) nMax = (int) pairs.size();
            cout << "Best moves:\n";
            for( int i=0; i<nMax; i++ )
            {
                //pairs.at(i).b.print();
                //cout << pairs.at(i).b.repr() << endl;
                cout << moveDiff(oldBoard, pairs.at(i).b) << ": ";
                cout << pairs.at(i).probs.equity();
                if( i > 0 ) cout << " (" << pairs.at(i).probs.equity() - pairs.at(0).probs.equity() << ")";
                cout << endl;
            }
            continue;
        }
        if( input == "rollout" )
        {
            vector<humanBoardProbsPair> rollPairs;
            int nMax = 3;
            if( nMax > pairs.size() ) nMax = (int) pairs.size();
            for( int i=0; i<nMax; i++ )
                rollPairs.push_back( humanBoardProbsPair( pairs.at(i).b, rolloutBoardProbabilitiesParallel( pairs.at(i).b, guideStrat, 3200, 1, 16, false ) ) );
            sort( rollPairs.begin(), rollPairs.end(), humanBoardProbsPairCompare );
            cout << "Best moves (rollout):\n";
            for( int i=0; i<nMax; i++ )
            {
                //rollPairs.at(i).b.print();
                cout << moveDiff(oldBoard, rollPairs.at(i).b) << ": ";
                cout << rollPairs.at(i).probs.equity();
                if( i > 0 ) cout << " (" << rollPairs.at(i).probs.equity() - rollPairs.at(0).probs.equity() << ")";
                cout << endl;
            }
            continue;
        }
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
        {
            // add the equity error to the total
            
            for( int i=0; i<possibleMoves.size(); i++ )
            {
                if( pairs.at(i).b == newBoard )
                {
                    equityError += pairs.at(0).probs.equity() - pairs.at(i).probs.equity();
                    nMoves ++;
                    break;
                }
            }
            return newBoard;
        }
        else
            cout << "Proposed move is not a valid move. Please try again.\n";
    }
}

bool strategyhuman::offerDouble( const board& brd, int cube )
{
    cout << "Would you like to double? (y for yes, anything else for no)\n";
    string input;
    getline( cin, input );
    if( input[0] == 'y' )
        return true;
    else
        return false;
}

bool strategyhuman::takeDouble( const board& brd, int cube )
{
    while( true )
    {
        cout << "You are being offered a double. Do you take? y for yes, n for no\n";
        string input;
        getline( cin, input );
        if( input[0] == 'y' ) return true;
        if( input[0] == 'n' ) return false;
    }
}