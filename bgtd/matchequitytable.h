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


#ifndef bgtd_matchequitytable_h
#define bgtd_matchequitytable_h

#include <string>
#include <vector>

using namespace std;

class matchequitytable
{
    // holds the info about a match equity table and allows lookup
    
public:
    matchequitytable( int nGames ); // empty nGamesxnGames MET
    matchequitytable( const string& fileName ); // load from a file
    
    ~matchequitytable() {};
    
    double matchEquity( int n, int m ) const; // lookup for MET for pre-Crawford or Crawford games
    double matchEquityPostCrawford( int n ) const; // post-Crawford equity lookup
    
    double matchWinningChance( int n, int m ) const; // lookup for MWC instead of MET (MET=2*MWC-1)
    double matchWinningChancePostCrawford( int n ) const;
    
    int getNGames() const { return nGames; };
    
    void loadTable( const string& fileName ); // load from a file
    
private:
    int nGames;
    bool tableFilled;
    
    vector<double> cachedPostCrawfordEquities;
    vector< vector<double> > cachedPreCrawfordEquities;
};

#endif
