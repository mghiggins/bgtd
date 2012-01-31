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


#include <cmath>
#include <fstream>
#include "strategytdexp4.h"
#include "randomc.h"

strategytdexp4::strategytdexp4( int nMiddle )
{
    this->nMiddle = nMiddle;
    
    int i, j;
    
    // size the weights and eligibility trace vectors appropriately
    
    outputProbWeights.resize( nMiddle+1 );
    outputGammonWinWeights.resize( nMiddle+1 );
    outputGammonLossWeights.resize( nMiddle+1 );
    outputBackgammonWinWeights.resize( nMiddle+1 );
    outputBackgammonLossWeights.resize( nMiddle+1 );
    middleWeights.resize( nMiddle );
    
    for( i=0; i<nMiddle; i++ )
        middleWeights.at(i).resize(199);
    
    // assign random weights
    
    CRandomMersenne rng(1);
    for( i=0; i<nMiddle+1; i++ )
    {
        outputProbWeights.at(i) = rng.IRandom(-100,100)/1000.;
        outputGammonWinWeights.at(i) = rng.IRandom(-100,100)/1000.;
        outputGammonLossWeights.at(i) = rng.IRandom(-100,100)/1000.;
        outputBackgammonWinWeights.at(i) = rng.IRandom(-100,100)/1000.;
        outputBackgammonLossWeights.at(i) = rng.IRandom(-100,100)/1000.;
    }
    for( i=0; i<nMiddle; i++ )
        for( j=0; j<199; j++ )
            middleWeights.at(i).at(j) = rng.IRandom(-100,100)/1000.;
    
    // allocate memory for the partial deriv vectors
    
    probDerivs.resize( nMiddle+1 );
    gamWinDerivs.resize( nMiddle+1 );
    gamLossDerivs.resize( nMiddle+1 );
    bgWinDerivs.resize( nMiddle+1 );
    bgLossDerivs.resize( nMiddle+1 );
    probInputDerivs.resize( nMiddle );
    gamWinInputDerivs.resize( nMiddle );
    gamLossInputDerivs.resize( nMiddle );
    bgWinInputDerivs.resize( nMiddle );
    bgLossInputDerivs.resize( nMiddle );
    
    for( i=0; i<nMiddle; i++ )
    {
        probInputDerivs.at(i).resize(199);
        gamWinInputDerivs.at(i).resize(199);
        gamLossInputDerivs.at(i).resize(199);
        bgWinInputDerivs.at(i).resize(199);
        bgLossInputDerivs.at(i).resize(199);
    }
    
    // put in default values for alpha and beta (the learning process parameters). We always
    // use lambda = 0 here.
    
    alpha  = 0.1;
    beta   = 0.1;
    lambda = 0.;
}

strategytdexp4::strategytdexp4( const strategytdexp2& baseStrat )
{
    setupExp( baseStrat );
}

strategytdexp4::strategytdexp4( const string& pathEnd, const string& fileSuffix, bool loadExp2 )
{
    if( loadExp2 )
    {
        strategytdexp2 s( pathEnd, fileSuffix, false );
        setupExp( s );
    }
    else
    {
        throw string( "Not implemented yet" );
    }
}

void strategytdexp4::setupExp( const strategytdexp2& baseStrat )
{
    // make the number of middle nodes the same
    
    nMiddle = baseStrat.nMiddle;
    
    // define lambda = 0 always, but grab alpha and beta from the base strategy
    
    alpha  = baseStrat.alpha;
    beta   = baseStrat.beta;
    lambda = 0;
    
    // the tdexp2 strategy has the full set of weights (no symmetry assumptions) but
    // has a slightly different setup for inputs, so we need to adjust for that.
    
    outputProbWeights = baseStrat.getOutputProbWeights();
    outputGammonWinWeights = baseStrat.getOutputGammonWinWeights();
    outputGammonLossWeights = baseStrat.getOutputGammonLossWeights();
    outputBackgammonWinWeights.resize(nMiddle+1,0);
    outputBackgammonLossWeights.resize(nMiddle+1,0);
    middleWeights.resize( nMiddle );
    vector< vector<double> > baseMiddles( baseStrat.getMiddleWeights() );
    
    int i, j;
    for( i=0; i<nMiddle; i++ )
    {
        // initialize all the weights to zero. We'll override all of them from the tdexp2 weights except
        // the bias weight and the "whose turn is it" weights, which we'll train.
        
        middleWeights.at(i).resize(199,0);
        
        for( j=0; j<98; j++ )
            middleWeights.at(i).at(j) = baseMiddles.at(i).at(j);
        for( j=98; j<196; j++ )
            middleWeights.at(i).at(j+1) = baseMiddles.at(i).at(j);
    }
    
    // allocate memory for the partial deriv vectors
    
    probDerivs.resize( nMiddle+1 );
    gamWinDerivs.resize( nMiddle+1 );
    gamLossDerivs.resize( nMiddle+1 );
    bgWinDerivs.resize( nMiddle+1 );
    bgLossDerivs.resize( nMiddle+1 );
    probInputDerivs.resize( nMiddle );
    gamWinInputDerivs.resize( nMiddle );
    gamLossInputDerivs.resize( nMiddle );
    
    for( i=0; i<nMiddle; i++ )
    {
        probInputDerivs.at(i).resize(199);
        gamWinInputDerivs.at(i).resize(199);
        gamLossInputDerivs.at(i).resize(199);
        bgWinInputDerivs.at(i).resize(199);
        bgLossInputDerivs.at(i).resize(199);
    }
}

double strategytdexp4::boardValue( const board& brd, const hash_map<string,int>* context ) const
{
    // get the inputs from the board, assuming the player holds the dice
    
    vector<double> inputs = getInputValues( brd, brd.perspective() );
    
    // calculate the middle layer node values
    
    vector<double> middles = getMiddleValues( inputs );
    
    // calculate the output node values from the middles
    
    double probWin            = getOutputProbValue( middles );
    double probCondGammonWin  = getOutputGammonWinValue( middles, brd );
    double probCondGammonLoss = getOutputGammonLossValue( middles, brd );
    double probCondBgWin      = getOutputBackgammonWinValue( middles, brd );
    double probCondBgLoss     = getOutputBackgammonLossValue( middles, brd );
    
    // calculate the expected number of points the player will win. probWin
    // corresponds to the probability of a win (any win); probCondGammon
    // corresponds to the probability of a gammon conditional on a win;
    // probCondGammonLoss corresponds to the probability of a gammon loss
    // conditional on a loss. Ditto for backgammon win/loss conditional probs.
    
    double ppg = probWin         * ( 1 * ( 1 - probCondGammonWin )  + 2 * probCondGammonWin  * ( 1 - probCondBgWin  ) + 3 * probCondGammonWin  * probCondBgWin  )
               - ( 1 - probWin ) * ( 1 * ( 1 - probCondGammonLoss ) + 2 * probCondGammonLoss * ( 1 - probCondBgLoss ) + 3 * probCondGammonLoss * probCondBgLoss );
    
    // the ppg is always from player 0's perspective. But we want it from the board
    // perspective.
    
    if( brd.perspective() == 0 )
        return ppg;
    else
        return -ppg;
}

vector<double> strategytdexp4::getInputValues( const board& brd, int turn ) const
{
    vector<double> inputs;
    inputs.resize(198,0);
    vector<int> checks = brd.checkers0Raw();
    vector<int> otherChecks = brd.checkers1Raw();
    int hit = brd.hit0Raw();
    int otherHit = brd.hit1Raw();
    int borneIn = brd.bornIn0Raw();
    int otherBorneIn = brd.bornIn1Raw();
    
    int i, j;
    
    // we put values for the first player in the first half of the inputs and for the second player
    // in the second half. 
    
    for( i=0; i<24; i++ )
    {
        // each spot gets four units. The first is 1 if there is at least one checker on the point,
        // else 0; the 2nd is 1 if there are at least two checkers; the 3rd if there are at least
        // three; and the fourth = max(0,(n-3)/2), where n=# of checkers. That's done for both players.
        
        for( j=0; j<3; j++ )
        {
            if( checks.at(i) > j )      inputs.at(4*i+j)    = 1;
            if( otherChecks.at(i) > j ) inputs.at(4*i+j+99) = 1;
        }
        if( checks.at(i) > 3 )      inputs.at(4*i+3)    = ( checks[i]-3 ) / 2.;
        if( otherChecks.at(i) > 3 ) inputs.at(4*i+3+99) = ( otherChecks[i]-3 ) / 2.;
    }
    
    // one spot for each player records the number on the bar
    
    inputs.at(96)  = hit / 2.;
    inputs.at(195) = otherHit / 2.;
    
    // one spot for each player records the number born in
    
    inputs.at(97)  = borneIn / 15.;
    inputs.at(196) = otherBorneIn / 15.;
    
    // one spot for each player notes whose turn it is
    
    inputs.at(98)  = turn == 0 ? 1 : 0;
    inputs.at(197) = turn == 0 ? 0 : 1;
    
    return inputs;
}

vector<double> strategytdexp4::getMiddleValues( const vector<double>& inputs ) const
{
    vector<double> mids;
    mids.resize(nMiddle);
    
    double val;
    int    i, j;
    
    for( i=0; i<nMiddle; i++ )
    {
        val = 0;
        for( j=0; j<198; j++ )
            val += middleWeights.at(i).at(j) * inputs.at(j);
        val += middleWeights.at(i).at(198); // bias weight
        mids.at(i) = 1. / ( 1 + exp( -val ) );
    }
    
    return mids;
}

double strategytdexp4::getOutputProbValue( const vector<double>& middles ) const
{
    double val=0;
    for( int i=0; i<nMiddle; i++ )
        val += outputProbWeights.at(i) * middles.at(i);
    val += outputProbWeights.at(nMiddle); // bias node
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexp4::getOutputGammonWinValue( const vector<double>& middles, const board& brd ) const
{
    // special case - if the other player has taken any pieces in, the gammon win prob is zero
    
    if( brd.bornIn1Raw() > 0 ) return 0;
    
    // otherwise calculate the network value
    
    double val=0;
    for( int i=0; i<nMiddle; i++ )
        val += outputGammonWinWeights.at(i) * middles.at(i);
    val += outputGammonWinWeights.at(nMiddle); // bias node
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexp4::getOutputGammonLossValue( const vector<double>& middles, const board& brd ) const
{
    // special case - if the player has taken any pieces in, the gammon loss prob is zero
    
    if( brd.bornIn0Raw() > 0 ) return 0;
    
    // otherwise calculate the network value
    
    double val=0;
    for( int i=0; i<nMiddle; i++ )
        val += outputGammonLossWeights.at(i) * middles.at(i);
    val += outputGammonLossWeights.at(nMiddle); // bias node
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexp4::getOutputBackgammonWinValue( const vector<double>& middles, const board& brd ) const
{
    // special case - if the other player has taken any pieces in, the gammon win prob is zero
    
    if( brd.bornIn1Raw() > 0 ) return 0;
    
    // also if there are no other player pieces in the player's box, the backgammon win prob is zero
    
    vector<int> checks( brd.checkers1Raw() );
    bool foundOne = brd.hit1Raw() > 0;
    if( !foundOne )
        for( int i=0; i<6; i++ )
            if( checks.at(i) > 0 )
            {
                foundOne = true;
                break;
            }
    if( !foundOne ) return 0;
    
    // otherwise calculate the network value
    
    double val=0;
    for( int i=0; i<nMiddle; i++ )
        val += outputBackgammonWinWeights.at(i) * middles.at(i);
    val += outputBackgammonWinWeights.at(nMiddle); // bias node
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexp4::getOutputBackgammonLossValue( const vector<double>& middles, const board& brd ) const
{
    // special case - if the player 0 has taken any pieces in, the gammon loss prob is zero
    
    if( brd.bornIn0Raw() > 0 ) return 0;
    
    // also if there are no player 0 pieces in player 1's box, the backgammon win prob is zero
    
    vector<int> checks( brd.checkers0Raw() );
    bool foundOne = brd.hit0Raw() > 0;
    if( !foundOne )
        for( int i=18; i<24; i++ )
            if( checks.at(i) > 0 )
            {
                foundOne = true;
                break;
            }
    if( !foundOne ) return 0;
    
    // otherwise calculate the network value
    
    double val=0;
    for( int i=0; i<nMiddle; i++ )
        val += outputBackgammonLossWeights.at(i) * middles.at(i);
    val += outputBackgammonLossWeights.at(nMiddle); // bias node
    return  1. / ( 1 + exp( -val ) );
}

vector<double> strategytdexp4::getOutputProbWeights() const { return outputProbWeights; }
vector<double> strategytdexp4::getOutputGammonWinWeights() const { return outputGammonWinWeights; }
vector<double> strategytdexp4::getOutputGammonLossWeights() const { return outputGammonLossWeights; }
vector<double> strategytdexp4::getOutputBackgammonWinWeights() const { return outputBackgammonWinWeights; }
vector<double> strategytdexp4::getOutputBackgammonLossWeights() const { return outputBackgammonLossWeights; }
vector< vector<double> > strategytdexp4::getMiddleWeights() const { return middleWeights; }

bool strategytdexp4::needsUpdate() const
{
    return learning;
}

void strategytdexp4::update( const board& oldBoard, const board& newBoard )
{
    // get the values from the old board
    
    vector<double> oldInputs   = getInputValues( oldBoard, oldBoard.perspective() );
    vector<double> oldMiddles  = getMiddleValues( oldInputs );
    double oldProbOutput       = getOutputProbValue( oldMiddles );
    double oldGammonWinOutput  = getOutputGammonWinValue( oldMiddles, oldBoard );
    double oldGammonLossOutput = getOutputGammonLossValue( oldMiddles, oldBoard );
    double oldBgWinOutput      = getOutputBackgammonWinValue( oldMiddles, oldBoard );
    double oldBgLossOutput     = getOutputBackgammonLossValue( oldMiddles, oldBoard );
    
    // calculate all the partial derivatives we'll need (of output node values
    // to the various weights)
    
    int i, j;
    
    // then do derivs of the prob nodes to each of the middle->input weights (that's a 2d array), and the derivs of each of
    // the middle nodes to its weights->inputs.
    
    double mid, input, v1, v2, v3, v4, v5;
    for( i=0; i<nMiddle; i++ )
    {
        mid = oldMiddles.at(i);
        v1  = outputProbWeights.at(i);
        v2  = outputGammonWinWeights.at(i);
        v3  = outputGammonLossWeights.at(i);
        v4  = outputBackgammonWinWeights.at(i);
        v5  = outputBackgammonLossWeights.at(i);
        
        probDerivs.at(i)    = mid * oldProbOutput       * ( 1 - oldProbOutput );
        gamWinDerivs.at(i)  = mid * oldGammonWinOutput  * ( 1 - oldGammonWinOutput );
        gamLossDerivs.at(i) = mid * oldGammonLossOutput * ( 1 - oldGammonLossOutput );
        bgWinDerivs.at(i)   = mid * oldBgWinOutput      * ( 1 - oldBgWinOutput );
        bgLossDerivs.at(i)  = mid * oldBgLossOutput     * ( 1 - oldBgLossOutput );
        
        for( j=0; j<198; j++ )
        {
            input = oldInputs.at(j);
            probInputDerivs.at(i).at(j)    = v1 * input * oldProbOutput       * ( 1 - oldProbOutput       ) * mid * ( 1 - mid );
            gamWinInputDerivs.at(i).at(j)  = v2 * input * oldGammonWinOutput  * ( 1 - oldGammonWinOutput  ) * mid * ( 1 - mid );
            gamLossInputDerivs.at(i).at(j) = v3 * input * oldGammonLossOutput * ( 1 - oldGammonLossOutput ) * mid * ( 1 - mid );
            bgWinInputDerivs.at(i).at(j)   = v4 * input * oldBgWinOutput      * ( 1 - oldBgWinOutput      ) * mid * ( 1 - mid );
            bgLossInputDerivs.at(i).at(j)  = v5 * input * oldBgLossOutput     * ( 1 - oldBgLossOutput     ) * mid * ( 1 - mid );
        }
        probInputDerivs.at(i).at(198)    = v1 * oldProbOutput       * ( 1 - oldProbOutput       ) * mid * ( 1 - mid );
        gamWinInputDerivs.at(i).at(198)  = v2 * oldGammonWinOutput  * ( 1 - oldGammonWinOutput  ) * mid * ( 1 - mid );
        gamLossInputDerivs.at(i).at(198) = v3 * oldGammonLossOutput * ( 1 - oldGammonLossOutput ) * mid * ( 1 - mid );
        bgWinInputDerivs.at(i).at(198)   = v4 * oldBgWinOutput      * ( 1 - oldBgWinOutput  )     * mid * ( 1 - mid );
        bgLossInputDerivs.at(i).at(198)  = v5 * oldBgLossOutput     * ( 1 - oldBgLossOutput )     * mid * ( 1 - mid );
    }
    probDerivs.at(nMiddle)    = oldProbOutput       * ( 1 - oldProbOutput );
    gamWinDerivs.at(nMiddle)  = oldGammonWinOutput  * ( 1 - oldGammonWinOutput );
    gamLossDerivs.at(nMiddle) = oldGammonLossOutput * ( 1 - oldGammonLossOutput );
    bgWinDerivs.at(nMiddle)   = oldBgWinOutput      * ( 1 - oldBgWinOutput );
    bgLossDerivs.at(nMiddle)  = oldBgLossOutput     * ( 1 - oldBgLossOutput );
    
    // now calculate the next estimate of the outputs. That's known if the game is over; otherwise we use the network's 
    // estimate on the new board as a proxy. Note that the update fn is only ever called by the game when the player wins, not when
    // the player loses, just because the winner is the last one to play. But we need to train on prob of losing a gammon too,
    // so we flip board perspective and train again based on that.
    
    bool trainGammonLoss = true;
    bool trainGammonWin  = true;
    bool trainBgLoss     = true;
    bool trainBgWin      = true;
    double newProbOutput, newGammonWinOutput, newGammonLossOutput, newBgWinOutput, newBgLossOutput;
    
    if( newBoard.bornIn0Raw() == 15 )
    {
        trainGammonLoss = false; // can't train the conditional prob of a gammon loss if there isn't a loss
        trainBgLoss     = false; // ditto for backgammon loss
        
        newProbOutput = 1.;
        if( newBoard.bornIn1Raw() == 0 ) // gammon or backgammon
        {
            newGammonWinOutput = 1.;
            vector<int> checks( newBoard.checkers1Raw() );
            bool foundOne = newBoard.hit1Raw() > 0;
            if( !foundOne )
            {
                for( int i=0; i<6; i++ )
                    if( checks.at(i) > 0 )
                    {
                        foundOne = true;
                        break;
                    }
            }
            newBgWinOutput = foundOne ? 1 : 0;
        }
        else
        {
            newGammonWinOutput = 0.;
            trainBgWin = false; // no gammon win so can't train conditional bg win prob
        }
    }
    else if( newBoard.bornIn1Raw() == 15 )
    {
        trainGammonWin = false;
        trainBgWin     = false;
        
        newProbOutput = 0.;
        
        if( newBoard.bornIn0Raw() == 0 ) // gammon loss or backgammon loss
        {
            newGammonLossOutput = 1;
            vector<int> checks( newBoard.checkers0Raw() );
            bool foundOne = newBoard.hit0Raw() > 0;
            if( !foundOne )
            {
                for( int i=18; i<24; i++ )
                    if( checks.at(i) > 0 )
                    {
                        foundOne = true;
                        break;
                    }
            }
            newBgLossOutput = foundOne ? 1 : 0;
        }
        else
        {
            newGammonLossOutput = 0;
            trainBgLoss = false;
        }
    }
    else
    {
        // estimate from the new board's outputs, remembering that after the move is done,
        // the other player gets the dice.
        
        vector<double> midVals( getMiddleValues( getInputValues( newBoard, !newBoard.perspective() ) ) );
        newProbOutput       = getOutputProbValue( midVals );
        newGammonWinOutput  = getOutputGammonWinValue( midVals, newBoard );
        newGammonLossOutput = getOutputGammonLossValue( midVals, newBoard );
        newBgWinOutput      = getOutputBackgammonWinValue( midVals, newBoard );
        newBgLossOutput     = getOutputBackgammonLossValue( midVals, newBoard );
    }
    
    // train the nodes as appropriate
    
    for( i=0; i<nMiddle; i++ )
    {
        outputProbWeights.at(i) += alpha * ( newProbOutput - oldProbOutput ) * probDerivs.at(i);
        
        if( trainGammonWin )
            outputGammonWinWeights.at(i) += alpha * ( newGammonWinOutput - oldGammonWinOutput ) * gamWinDerivs.at(i);
        
        if( trainGammonLoss )
            outputGammonLossWeights.at(i) += alpha * ( newGammonLossOutput - oldGammonLossOutput ) * gamLossDerivs.at(i);
        
        if( trainBgWin )
            outputBackgammonWinWeights.at(i) += alpha * ( newBgWinOutput - oldBgWinOutput ) * bgWinDerivs.at(i);
        
        if( trainBgLoss )
            outputBackgammonLossWeights.at(i) += alpha * ( newBgLossOutput - oldBgLossOutput ) * bgLossDerivs.at(i);
        
        for( j=0; j<199; j++ )
        {
            middleWeights.at(i).at(j) += beta * ( newProbOutput - oldProbOutput ) * probInputDerivs.at(i).at(j);
            if( trainGammonWin )
                middleWeights.at(i).at(j) += beta * ( newGammonWinOutput - oldGammonWinOutput ) * gamWinInputDerivs.at(i).at(j);
            if( trainGammonLoss )
                middleWeights.at(i).at(j) += beta * ( newGammonLossOutput - oldGammonLossOutput ) * gamLossInputDerivs.at(i).at(j);
            if( trainBgWin )
                middleWeights.at(i).at(j) += beta * ( newBgWinOutput - oldBgWinOutput ) * bgWinInputDerivs.at(i).at(j);
            if( trainBgLoss )
                middleWeights.at(i).at(j) += beta * ( newBgLossOutput - oldBgLossOutput ) * bgLossInputDerivs.at(i).at(j);
        }
    }
    
    outputProbWeights.at(nMiddle) += alpha * ( newProbOutput - oldProbOutput ) * probDerivs.at(nMiddle);
    if( trainGammonWin )
        outputGammonWinWeights.at(nMiddle) += alpha * ( newGammonWinOutput - oldGammonWinOutput ) * gamWinDerivs.at(nMiddle);
    if( trainGammonLoss )
        outputGammonLossWeights.at(nMiddle) += alpha * ( newGammonLossOutput - oldGammonLossOutput ) * gamLossDerivs.at(nMiddle);
    if( trainBgWin )
        outputBackgammonWinWeights.at(nMiddle) += alpha * ( newBgWinOutput - oldBgWinOutput ) * bgWinDerivs.at(nMiddle);
    if( trainBgLoss )
        outputBackgammonLossWeights.at(nMiddle) += alpha * ( newBgLossOutput - oldBgLossOutput ) * bgLossDerivs.at(nMiddle);
}
