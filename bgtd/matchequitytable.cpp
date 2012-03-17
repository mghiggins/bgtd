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

#include "doublefns.h"
#include "matchequitytable.h"

matchequitytable::matchequitytable( int nGames ) : nGames(nGames), tableFilled(false) {}

matchequitytable::matchequitytable( const string& fileName )
{
    loadTable( fileName );
}

double matchequitytable::matchEquity( int n, int m ) const
{
    if( !tableFilled ) throw string( "MET not initialized" );
    if( n <= 0 ) return 1;
    if( m <= 0 ) return -1;
    if( n == m ) return 0;
    if( n > nGames or m > nGames ) throw string( "Outside cache size" );
    
    if( n > m )
        return cachedPreCrawfordEquities.at(n-1).at(m-1);
    else
        return -cachedPreCrawfordEquities.at(m-1).at(n-1);
}

double matchequitytable::matchEquityPostCrawford( int n ) const
{
    if( !tableFilled ) throw string( "MET not initialized" );
    if( n > nGames ) throw string( "Outside cache size" );
    if( n <= 0 ) return -1;
    return cachedPostCrawfordEquities.at(n-1);
}

double matchequitytable::matchWinningChance( int n, int m ) const
{
    return ( matchEquity(n,m) - 1 ) / 2.;
}

double matchequitytable::matchWinningChancePostCrawford( int n ) const
{
    return ( matchEquityPostCrawford(n) - 1 ) / 2.;
}

void matchequitytable::loadTable( const string& fileName )
{
    tableFilled = true;
    
    METData data = loadMatchEquityTableData( fileName );
    
    nGames                     = data.nGames;
    cachedPostCrawfordEquities = data.equitiesPostCrawford;
    cachedPreCrawfordEquities  = data.equitiesPreCrawford;
}