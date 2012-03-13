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
    
    double avgProb=0;
    
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
            
            avgProb += stateProb * probWin;
        }
    }
    
    if( abs( avgProb - 0.5 ) > 1e-10 )
    {
        // add a small correction to all the probs that correspond to the two-moves-away set
        
        int i;
        double totWeight=0;
        for( i=0; i<data.size(); i++ )
            if( abs( data.at(i).stateProb - 1./30 ) > 1e-5 )
                totWeight += data.at(i).stateProb;
        
        // correct all the probs
        
        for( i=0; i<data.size(); i++ )
            if( abs( data.at(i).stateProb - 1./30 ) > 1e-5 )
                data.at(i).stateGameProbs.probWin += ( 0.5 - avgProb ) / totWeight;
    }
    
    return data;
}

double matchEquityPostCrawford( int nGames, double gamProb, const vector<stateData>& singleData, const vector<stateData>& data )
{
    if( nGames < 1 ) throw string( "nGames must be >= 1 - represents the number of games the opponent is away from winning the match" );
    
    if( nGames == 1 ) return 0; // symmetric - equal odds on either side
    if( nGames == 3 ) return 0.5 - gamProb; // player always accepts double; easy to calculate this equity from ending state probs
    
    const vector<stateData> * workingData;
    
    if( nGames == 2 )
        workingData = &singleData;
    else
        workingData = &data;
    
    // get the equity if the player passes when doubled - the same for every state
    
    double passEquity = matchEquityPostCrawford( nGames - 1, gamProb, singleData, data );
    
    // figure out the match equity in the different win/loss states after a double
    
    double winVal = 1; // no matter the type of win, a player win means a match win
    double singleLossVal;
    if( nGames <= 2 )
        singleLossVal = -1;
    else
        singleLossVal = matchEquityPostCrawford( nGames - 2, gamProb, singleData, data );
    double gamLossVal;
    if( nGames <= 4 )
        gamLossVal = -1;
    else
        gamLossVal = matchEquityPostCrawford( nGames - 4, gamProb, singleData, data );
    double bgLossVal;
    if( nGames <= 6 )
        bgLossVal = -1;
    else
        bgLossVal = matchEquityPostCrawford( nGames - 6, gamProb, singleData, data );
    
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

double matchEquityCrawford( int nGames, double gamProb, const vector<stateData>& singleData, const vector<stateData>& data )
{
    if( nGames < 1 ) throw string( "nGames must be >= 1 - represents the number of games the opponent is away from winning the match" );
    
    if( nGames == 1 ) return 0; // symmetric
    
    double winVal=1; // any win by the player means a match win
    double singleLossVal;
    if( nGames == 2 )
        singleLossVal = 0;
    else
        singleLossVal = matchEquityPostCrawford( nGames - 1, gamProb, singleData, data );
    double gamLossVal;
    if( nGames == 2 )
        gamLossVal = -1;
    else if( nGames == 3 )
        gamLossVal = 0;
    else
        gamLossVal = matchEquityPostCrawford( nGames - 2, gamProb, singleData, data );
    
    return 0.5 * winVal + ( 0.5 - gamProb ) * singleLossVal + gamProb * gamLossVal;
}