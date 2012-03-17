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

#include <boost/thread.hpp>
#include <cmath>
#include <fstream>
#include <vector>
#include "doublefns.h"
#include "gamefns.h"

vector<gameProbabilities> probsFDSP;

class workerFDSP
{
public:
    workerFDSP( int i, strategyprob& strat, board& startBoard, int die1, int die2, hash_map<string,int> * ctxPtr ) 
    : i(i), strat(strat), startBoard(startBoard), die1(die1), die2(die2), ctxPtr(ctxPtr) {};
    
    void operator()()
    {
        set<board> moves( possibleMoves( startBoard, die1, die2 ) );
        board bestBoard( strat.preferredBoard( startBoard, moves, ctxPtr ) );
        gameProbabilities gameProbs( strat.boardProbabilities(bestBoard, ctxPtr) );
        
        probsFDSP.at(i) = gameProbs;
    }
    
private:
    int i;
    strategyprob& strat;
    board& startBoard;
    int die1, die2;
    hash_map<string,int> * ctxPtr;
};

void writeCrawfordFirstDoubleStateProbDb( const string& dbFileName, strategyprob& strat, bool outrightWin )
{
    using namespace boost;
    
    // for each state calculate the probability of that state being the first opportunity
    // for the opponent to double (stateProbs); and the game win/loss probabilities in that
    // state (stateGameProbs).
    
    vector<gameProbabilities> stateGameProbs;
    vector<double> stateProbs;
    int i, j, k, l;
    
    hash_map<string,int> ctx;
    if( outrightWin ) ctx[ "singleGame" ] = 1;
    
    probsFDSP.resize(21);
    
    for( i=1; i<=6; i++ )
        for( j=1; j<i; j++ )
        {
            board b;
            set<board> moves( possibleMoves( b, i, j ) );
            board bestBoard( strat.preferredBoard( b, moves, &ctx ) );
            string move( moveDiff( b, bestBoard) );
            gameProbabilities gameProbs( strat.boardProbabilities(bestBoard,&ctx) );
            
            cout << "(" << i << "," << j << "): " << gameProbs.probWin << "   " << move << endl;
            
            stateGameProbs.push_back( gameProbs );
            stateProbs.push_back( 1./30 ); // there are 15 possible moves; and half the time the player has the 1st move and the opponent can double after the 1st move
            
            // now get the equities & probs for the case where the opponent goes first; then they have to wait two steps until they can double
            
            bestBoard.setPerspective(1);
            
            thread_group ts;
            int count = 0;
            vector<double> subProbs(21);
            
            for( k=1; k<=6; k++ )
                for( l=1; l<=k; l++ )
                {
                    ts.create_thread( workerFDSP( count, strat, bestBoard, k, l, &ctx ) );
                    if( k == l )
                        subProbs.at(count) = 1./30/36;
                    else
                        subProbs.at(count) = 1./30/18;
                    count++;
                }
            
            ts.join_all();
            for( k=0; k<21; k++ )
            {
                stateProbs.push_back(subProbs.at(k));
                stateGameProbs.push_back(probsFDSP.at(k));
            }
        }
    
    // now write out the results to the file
    
    ofstream f( dbFileName.c_str() );
    for( i=0; i<stateProbs.size(); i++ )
    {
        gameProbabilities ps( stateGameProbs.at(i) );
        f << stateProbs.at(i) << "," << ps.probWin << "," << ps.probGammonWin << "," << ps.probGammonLoss << "," << ps.probBgWin << "," << ps.probBgLoss << endl;
    }
    f.close();
}

vector<stateData> loadCrawfordFirstDoubleStateProbDb( const string& dbFileName )
{
    ifstream f( dbFileName.c_str(), ios::in );
    if( !f ) throw string( "No file named " + dbFileName );
    
    string line, bit;
    vector<stateData> data;
    
    double avgProb=0, avgGamWinProb=0, avgGamLossProb=0, avgBgWinProb=0, avgBgLossProb=0;
    
    while( !f.eof() )
    {
        getline( f, line );
        if( line != "" )
        {
            stringstream ss(line);
            getline( ss, bit, ',' );
            double stateProb = atof( bit.c_str() );
            getline( ss, bit, ',' );
            double probWin = atof( bit.c_str() );
            getline( ss, bit, ',' );
            double probGammonWin = atof( bit.c_str() );
            getline( ss, bit, ',' );
            double probGammonLoss = atof( bit.c_str() );
            getline( ss, bit, ',' );
            double probBgWin = atof( bit.c_str() );
            getline( ss, bit, ',' );
            double probBgLoss = atof( bit.c_str() );
            
            stateData datum( stateProb, gameProbabilities( probWin, probGammonWin, probGammonLoss, probBgWin, probBgLoss ) );
            data.push_back(datum);
            
            // also calculate the average prob of win. This should come out to exactly 0.5; if not, we normalize.
            
            avgProb        += stateProb * probWin;
            avgGamWinProb  += stateProb * probGammonWin;
            avgGamLossProb += stateProb * probGammonLoss;
            avgBgWinProb   += stateProb * probBgWin;
            avgBgLossProb  += stateProb * probBgLoss;
        }
    }
    
    if( abs( avgProb - 0.5 ) > 1e-10 )
    {
        // add a small correction to all probs to get the average prob to equal 0.5 exactly
        
        int i;
        for( i=0; i<data.size(); i++ )
            data.at(i).stateGameProbs.probWin += 0.5 - avgProb;
    }
    
    if( abs( avgGamWinProb - avgGamLossProb ) > 1e-10 )
    {
        // correct the gammon win and loss probs back to the average
        
        double avgGamProb = 0.5 * ( avgGamWinProb + avgGamLossProb );
        int i;
        for( i=0; i<data.size(); i++ )
        {
            data.at(i).stateGameProbs.probGammonWin  += avgGamProb - avgGamWinProb;
            data.at(i).stateGameProbs.probGammonLoss += avgGamProb - avgGamLossProb;
        }
    }
    
    if( abs( avgBgWinProb - avgBgLossProb ) > 1e-10 )
    {
        // correct the bg win and loss probs back to the average
        
        double avgBgProb = 0.5 * ( avgBgWinProb + avgBgLossProb );
        int i;
        for( i=0; i<data.size(); i++ )
        {
            data.at(i).stateGameProbs.probBgWin  += avgBgProb - avgBgWinProb;
            data.at(i).stateGameProbs.probBgLoss += avgBgProb - avgBgLossProb;
        }
    }
    
    return data;
}

int maxCacheSize=24;
vector<double> * cachedPostCrawfordEquities=0;
vector< vector<double> > * cachedPreCrawfordEquities=0;

void initializeCaches();
void initializeCaches()
{
    // -5 is a signal that the calc isn't done
    
    cachedPostCrawfordEquities = new vector<double>;
    cachedPostCrawfordEquities->resize(maxCacheSize,-5);
    cachedPreCrawfordEquities = new vector< vector<double> >;
    cachedPreCrawfordEquities->resize(maxCacheSize);
    for( int i=0; i<maxCacheSize; i++ ) cachedPreCrawfordEquities->at(i).resize(i+1,-5);
}

double matchEquityPostCrawford( int nGames, double gamProb, const vector<stateData>& singleData, const vector<stateData>& data, bool useCache )
{
    if( nGames < 1 ) throw string( "nGames must be >= 1 - represents the number of games the opponent is away from winning the match" );
    
    if( nGames == 1 ) return 0; // symmetric - equal odds on either side
    if( nGames == 3 ) return 0.5 - gamProb; // player always accepts double; easy to calculate this equity from ending state probs
    
    if( useCache and cachedPostCrawfordEquities )
    {
        double cachedME = cachedPostCrawfordEquities->at(nGames-1);
        if( cachedME > -2 ) return cachedME;
    }
    
    const vector<stateData> * workingData;
    
    if( nGames == 2 )
        workingData = &singleData;
    else
        workingData = &data;
    
    // get the equity if the player passes when doubled - the same for every state
    
    double passEquity = matchEquityPostCrawford( nGames - 1, gamProb, singleData, data, useCache );
    
    // figure out the match equity in the different win/loss states after a double
    
    double winVal = 1; // no matter the type of win, a player win means a match win
    double singleLossVal;
    if( nGames <= 2 )
        singleLossVal = -1;
    else
        singleLossVal = matchEquityPostCrawford( nGames - 2, gamProb, singleData, data, useCache );
    double gamLossVal;
    if( nGames <= 4 )
        gamLossVal = -1;
    else
        gamLossVal = matchEquityPostCrawford( nGames - 4, gamProb, singleData, data, useCache );
    double bgLossVal;
    if( nGames <= 6 )
        bgLossVal = -1;
    else
        bgLossVal = matchEquityPostCrawford( nGames - 6, gamProb, singleData, data, useCache );
    
    // calculate the equity in each state and max it with the pass equity - means that in each state they either take or pass,
    // whichever gives a larger match equity
    
    double matchEquity=0;
    double equity;
    
    for( int i=0; i<workingData->size(); i++ )
    {
        const gameProbabilities& probs( workingData->at(i).stateGameProbs );
        equity = probs.probWin * winVal + ( 1 - probs.probWin - probs.probGammonLoss ) * singleLossVal + ( probs.probGammonLoss - probs.probBgLoss ) * gamLossVal + probs.probBgLoss * bgLossVal;
        if( equity < passEquity ) equity = passEquity; // we pass instead of take
        matchEquity += workingData->at(i).stateProb * equity;
    }
    
    return matchEquity;
}

double matchEquityCrawford( int nGames, double gamProb, const vector<stateData>& singleData, const vector<stateData>& data, bool useCache )
{
    if( nGames < 1 ) throw string( "nGames must be >= 1 - represents the number of games the opponent is away from winning the match" );
    
    if( nGames == 1 ) return 0; // symmetric
    
    if( useCache and cachedPreCrawfordEquities )
    {
        double cachedME = cachedPreCrawfordEquities->at(nGames-1).at(0);
        if( cachedME > -2 ) return -cachedME;
    }
    
    double winVal=1; // any win by the player means a match win
    double singleLossVal;
    if( nGames == 2 )
        singleLossVal = 0;
    else
        singleLossVal = matchEquityPostCrawford( nGames - 1, gamProb, singleData, data, useCache );
    double gamLossVal;
    if( nGames == 2 )
        gamLossVal = -1;
    else if( nGames == 3 )
        gamLossVal = 0;
    else
        gamLossVal = matchEquityPostCrawford( nGames - 2, gamProb, singleData, data, useCache );
    
    return 0.5 * winVal + ( 0.5 - gamProb ) * singleLossVal + gamProb * gamLossVal;
}

double matchEquity( int n, int m, double gamProb, const vector<stateData>& singleData, const vector<stateData>& data, bool useCache )
{
    // we explicitly do calcs for n<m; others we imply
    
    if( n == m ) return 0; // symmetric
    if( n > m ) return -matchEquity( m, n, gamProb, singleData, data, useCache );
    
    // if the match is over, return a match equity of +1
    
    if( n <= 0 ) return 1;
    
    if( useCache and cachedPreCrawfordEquities )
    {
        double cachedME = cachedPreCrawfordEquities->at(m-1).at(n-1);
        if( cachedME > -2 ) 
            return -cachedME; // -1 because we cached with first index >= second index
    }
    
    // if it's a pre-Crawford game, return that match equity
    
    if( n == 1 ) return matchEquityCrawford( m, gamProb, singleData, data, useCache );
    
    // otherwise do the calc - interpolate linearly btw take and cash points
    
    interpMEdata interp = matchEquityInterpData( n, m, 1, true, gamProb, singleData, data, useCache );
    return interp(0.5);
}

interpMEdata matchEquityInterpData( int n, int m, int cube, bool playersCube, double gamProb, const vector<stateData>& singleData, const vector<stateData>& data, bool useCache )
{
    if( n >= m or m < 3 ) throw string( "Invalid args: must have n < m and m > 2" );
    
    // if the cube's at its maximum value, any win is a match win and any loss is a match loss
    
    int cmax=4; // because we don't call this for m<3
    while( cmax < m ) cmax *= 2;
    if( cube == cmax ) return interpMEdata( 0, 1, -1, 1 );
    
    double takePoint, cashPoint, takeME, cashME;
    
    // if it's the player's cube, take point is zero and equity depends on the single vs gammon loss;
    // otherwise calculate the take point.
    
    if( playersCube and cube != 1 )
    {
        takePoint = 0;
        takeME    = 2 * gamProb * matchEquity( n, m-2*cube, gamProb, singleData, data, useCache ) + ( 1 - 2*gamProb ) * matchEquity( n, m-cube, gamProb, singleData, data, useCache );
    }
    else
    {
        interpMEdata interp2 = matchEquityInterpData( n, m, cube*2, true, gamProb, singleData, data, useCache );
        
        takeME    = matchEquity( n, m-cube, gamProb, singleData, data, useCache );
        takePoint = interp2.solve(takeME);
    }
    
    // if it's the opponent's cube, cash point is 100% and equity depends on the single vs gammon win;
    // otherwise calculate the cash point.
    
    if( !playersCube and cube != 1 )
    {
        cashPoint = 1;
        cashME    = 2 * gamProb * matchEquity( n-2*cube, m, gamProb, singleData, data, useCache ) + ( 1 - 2*gamProb ) * matchEquity( n-cube, m, gamProb, singleData, data, useCache );
    }
    else
    {
        interpMEdata interp2 = matchEquityInterpData( n, m, cube*2, false, gamProb, singleData, data, useCache );
        
        cashME    = matchEquity( n-cube, m, gamProb, singleData, data, useCache );
        cashPoint = interp2.solve(cashME);
    }
    
    return interpMEdata(takePoint,cashPoint,takeME,cashME);
}

double matchEquityBI( int n, int m, double gamProb, const vector<stateData>& singleData, const vector<stateData>& data, double sigma, int nP, int nT, double timeMultiple )
{
    // we explicitly do calcs for n<m; others we imply
    
    if( n == m ) return 0; // symmetric
    if( n > m ) return -matchEquityBI( m, n, gamProb, singleData, data, sigma, nP, nT, timeMultiple );
    if( n <= 0 ) return 1;
    if( n == 1 ) return matchEquityCrawford( m, gamProb, singleData, data, true );
    
    // nP must be odd so that there's a point at P=1/2
    
    if( nP % 2 != 1 ) throw string( "nP must be odd" );
    
    // figure out the max cube value and number of layers
    
    vector<double> cubes(1);
    int cmax=1;
    while( cmax < m )
    {
        cmax *= 2;
        cubes.push_back(cmax);
    }
    int nl=(int)cubes.size()*2 - 2; // number of layers. 1 for cube=1 and cube=cmax; 2 for the rest, one for player and one for opponent owning the cube
    
    // get the boundary conditions if we hit P=0 or P=1, for each layer; also pass and cash values
    
    vector<double> zeroBCs;
    vector<double> oneBCs;
    vector<double> cashVals;
    vector<double> passVals;
    
    int i, j;
    int cube;
    
    for( i=0; i<nl; i++ )
    {
        if( i == 0 )
            cube = 1;
        else if( i < nl-1 )
            cube = cubes.at( (i-1)/2 + 1 );
        else
            cube = cmax;
        
        zeroBCs.push_back( 2*gamProb*matchEquityBI( n, m-2*cube, gamProb, singleData, data, sigma, nP, nT, timeMultiple ) 
                           + ( 1 - 2*gamProb )*matchEquityBI( n, m-cube, gamProb, singleData, data, sigma, nP, nT, timeMultiple ) );
        oneBCs.push_back( 2*gamProb*matchEquityBI( n-2*cube, m, gamProb, singleData, data, sigma, nP, nT, timeMultiple ) 
                          + ( 1 - 2*gamProb )*matchEquityBI( n-cube, m, gamProb, singleData, data, sigma, nP, nT, timeMultiple ) );
        cashVals.push_back( matchEquityBI( n-cube, m, gamProb, singleData, data, sigma, nP, nT, timeMultiple ) );
        passVals.push_back( matchEquityBI( n, m-cube, gamProb, singleData, data, sigma, nP, nT, timeMultiple ) );
    }
    
    // the end time of the grid we'll set to be very far away - measured in multiples of 1/sigma^2. timeMultiple is that multiplier
    // which should be >> 1.
    
    double tMax = timeMultiple / sigma / sigma;
    
    // define the grid dimensions
    
    double dP = 1. / ( nP - 1 );
    double dt = tMax / ( nT - 1 );
    
    // setup the grid and set the ending match equity if we don't touch P=0 or P=1. Make match equity linear between
    // P=0 and P=1 for each grid. Shouldn't matter much since the odds of making it to the end are low.
    
    vector< vector<double> > MEs(nl);
    for( i=0; i<nl; i++ ) 
    {
        MEs.at(i).resize(nP,0.);
        for( j=0; j<nP; j++ )
            MEs.at(i).at(j) = oneBCs.at(i) * j*dP + zeroBCs.at(i) * ( 1 - j*dP );
    }
    
    // backward induct all layers independently, then mix based on doubling decisions. Uses an explicit scheme.
    
    double gamma, doubleEquity;
    int it, doubleLayer;
    
    for( it=0; it<nT-1; it++ )
    {
        // backward-induct all the layers
        
        for( i=0; i<nl; i++ )
        {
            vector<double> newMEs(nP);
            newMEs.at(0) = zeroBCs.at(i);
            newMEs.at(nP-1) = oneBCs.at(i);
            
            for( j=1; j<nP-1; j++ )
            {
                gamma = ( MEs.at(i).at(j+1) + MEs.at(i).at(j-1) - 2 * MEs.at(i).at(j) ) / dP / dP;
                newMEs.at(j) = MEs.at(i).at(j) + sigma * sigma * dt / 2. * gamma;
            }
            MEs.at(i) = newMEs;
        }
        
        // mix based on doubling decisions
        
        vector< vector<double> > newMEs(MEs);
        
        for( i=0; i<nl; i++ )
        {
            // if the player owns the cube or it's centered, check whether it's worth doubling. Equity
            // goes to max of current equity, equity if opponent has the doubled cube, and the cash value.
            
            if( ( i==0 or (i-1)%2==0 ) and i!=nl-1 )
            {
                if( i==0 )
                    doubleLayer = 2; // opponent holds cube at 2
                else if( i==nl-3 )
                    doubleLayer = nl-1; // player or opponent holds cube at cmax
                else
                    doubleLayer = i+3; // opponent holds cube at double its current value
                
                for( j=1; j<nP-1; j++ )
                {
                    doubleEquity = MEs.at(doubleLayer).at(j);
                    if( cashVals.at(i) < doubleEquity ) doubleEquity = cashVals.at(i);
                    if( doubleEquity > newMEs.at(i).at(j) )
                        newMEs.at(i).at(j) = doubleEquity;
                }
            }
            
            // if the opponent holds the cube or it's centered, check whether the opponent doubles.
            
            if( ( i==0 or (i-1)%2==1 ) and i!=nl-1 )
            {
                if( i==0 )
                    doubleLayer = 1; // player holds cube at 1
                else if( i==nl-2 )
                    doubleLayer = nl-1; // player or opponent holds cube at cmax
                else
                    doubleLayer = i+1; // player holds cube at double its current value
                
                for( j=1; j<nP-1; j++ )
                {
                    doubleEquity = MEs.at(doubleLayer).at(j);
                    if( passVals.at(i) > doubleEquity ) doubleEquity = passVals.at(i);
                    
                    if( doubleEquity < newMEs.at(i).at(j) )
                        newMEs.at(i).at(j) = doubleEquity;
                }
            }
        }
        
        MEs = newMEs;
    }
    
    return MEs.at(0).at(nP/2); // grab the value for centered cube and P=1/2
}

void writeMatchEquityTable( double gamProb, const vector<stateData>& singleData, const vector<stateData>& data, const string& fileName )
{
    // everything gets stored in the cache
    
    if( !cachedPostCrawfordEquities ) initializeCaches();
    
    // start by calculating all the post-Crawford equities. 
    
    int i, j;
    for( i=0; i<maxCacheSize; i++ )
        cachedPostCrawfordEquities->at(i) = matchEquityPostCrawford( i+1, gamProb, singleData, data, false );
    
    // then the Crawford and post-Crawford equities
    
    for( i=0; i<maxCacheSize; i++ )
        for( j=0; j<=i; j++ )
            cachedPreCrawfordEquities->at(i).at(j) = matchEquity( i+1, j+1, gamProb, singleData, data, true );
    
    // now write everything to the file
    
    ofstream f( fileName.c_str() );
    for( i=0; i<maxCacheSize; i++ )
        f << cachedPostCrawfordEquities->at(i) << endl;
    for( i=0; i<maxCacheSize; i++ )
        for( j=0; j<=i; j++ )
            f << cachedPreCrawfordEquities->at(i).at(j) << endl;
    f.close();
}

void loadMatchEquityTable( const string& fileName )
{
    if( !cachedPreCrawfordEquities ) initializeCaches();
    
    ifstream f( fileName.c_str(), ios::in );
    if( !f ) throw string( "Could not find file with name " + fileName );
    
    string line;
    int i, j;
    for( i=0; i<maxCacheSize; i++ )
    {
        getline( f, line );
        cachedPostCrawfordEquities->at(i) = atof( line.c_str() );
    }
    for( i=0; i<maxCacheSize; i++ )
        for( j=0; j<=i; j++ )
        {
            getline( f, line );
            cachedPreCrawfordEquities->at(i).at(j) = atof( line.c_str() );
        }
}

METData loadMatchEquityTableData( const string& fileName )
{
    ifstream f( fileName.c_str(), ios::in );
    if( !f ) throw string( "Could not find file with name " + fileName );
    
    string line;
    
    // first element is size of table
    
    getline( f, line );
    int nGames = atoi( line.c_str() );
    
    vector<double> equitiesPostCrawford(nGames);
    vector< vector<double> > equitiesPreCrawford(nGames);
    
    int i, j;
    for( i=0; i<nGames; i++ )
    {
        getline( f, line );
        equitiesPostCrawford.at(i) = atof( line.c_str() );
    }
    for( i=0; i<nGames; i++ )
    {
        equitiesPreCrawford.at(i).resize(i+1);
        for( j=0; j<=i; j++ )
        {
            getline( f, line );
            equitiesPreCrawford.at(i).at(j) = atof( line.c_str() );
        }
    }
    
    METData data;
    data.nGames = nGames;
    data.equitiesPostCrawford = equitiesPostCrawford;
    data.equitiesPreCrawford  = equitiesPreCrawford;
    
    return data;
}

double matchEquityPostCrawfordCached( int n )
{
    if( !cachedPreCrawfordEquities ) throw string( "MET not initialized" );
    if( n > maxCacheSize ) throw string( "Outside cache size" );
    if( n <= 0 ) return -1;
    return cachedPostCrawfordEquities->at(n-1);
}

double matchEquityCached( int n, int m )
{
    if( n <= 0 ) return 1;
    if( m <= 0 ) return -1;
    if( n == m ) return 0;
    if( n > maxCacheSize or m > maxCacheSize ) throw string( "Outside cache size" );
    
    if( n > m )
        return cachedPreCrawfordEquities->at(n-1).at(m-1);
    else
        return -cachedPreCrawfordEquities->at(m-1).at(n-1);
}

double matchEquityTableSize()
{
    return maxCacheSize;
}