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
#include "randomc.h"
#include "strategytdorigsym.h"

strategytdorigsym::strategytdorigsym()
{
    nMiddle = 40; // default # of middle nodes
    setup();
}

strategytdorigsym::strategytdorigsym( int nMiddle )
{
    this->nMiddle = nMiddle;
    setup();
}

strategytdorigsym::~strategytdorigsym()
{
}

void strategytdorigsym::setup()
{
    // generates random weights
    
    CRandomMersenne rng(1);
    
    outputWeights.resize(nMiddle);
    middleWeights.resize(nMiddle);
    
    int i, j;
    for( i=0; i<nMiddle; i++ )
    {
        outputWeights.at(i) = rng.IRandom(-100, 100)/1000.;
        middleWeights.at(i).resize(99);
        for( j=0; j<99; j++ )
            middleWeights.at(i).at(j) = rng.IRandom(-100, 100)/1000.;
    }
    
    // set alpha and beta sensibly. Lambda we always set to zero.
    
    alpha  = 0.1;
    beta   = 0.1;
    lambda = 0;
}

gameProbabilities strategytdorigsym::boardProbabilities( const board& brd, const hash_map<string,int>* context )
{
    // get the prob of win assuming the opponent holds the dice
    
    double probWin = getOutput( getMiddleValues( getInputValues( brd, false ) ) );
    gameProbabilities probs( probWin, 0, 0, 0, 0 );
    return probs;
}

vector<double> strategytdorigsym::getInputValues( const board& brd, bool holdDice ) const
{
    vector<double> inputs;
    inputs.resize(198,0);
    
    vector<int> checks( brd.checkers() );
    vector<int> otherChecks( brd.otherCheckers() );
    int hit          = brd.hit();
    int otherHit     = brd.otherHit();
    int borneIn      = brd.bornIn();
    int otherBorneIn = brd.otherBornIn();

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
    
    // one spot noting whose turn it is - always called from the player's perspective
    
    inputs.at(98)  = holdDice ? 1 : 0;
    inputs.at(197) = holdDice ? 0 : 1;
    
    return inputs;
}

vector<double> strategytdorigsym::getMiddleValues( const vector<double>& inputs ) const
{
    vector<double> mids(nMiddle);
    
    int i, j;
    double val;
    
    for( i=0; i<nMiddle; i++ )
    {
        const vector<double>& weights = middleWeights.at(i);
        
        val = 0;
        
        // we take advantage of symmetries of the layout when we flip a board's perspective to ensure
        // that the network's probability of player 1 winning == 1 - prob of player 2 winning on a 
        // perspective flip. We know that on a flip the checker layout is exactly reversed from what
        // it is before the flip, but the inputs for each spot come in groups of 4. So the weight
        // in the first half of the inputs = -weight at position (47-j/4)*4 + j%4 + 3, where j/4 does the
        // floor to the nearest integer. So 0->191, 3->194, 4->187, 7->190, and so on.
        
        for( j=0; j<96; j++ )
        {
            val += weights.at(j) * inputs.at(j);
            val -= weights.at(j) * inputs.at((47-j/4)*4+j%4+3); // weights for 2nd half equal negative of weights for 1st half
        }
        
        // the last three inputs in the 1st half are the # hit, the # born off, and whether it's the player's turn,
        // which map directly to three at the end of the 2nd half
        
        val += weights.at(96) * inputs.at(96);
        val -= weights.at(96) * inputs.at(195);
        val += weights.at(97) * inputs.at(97);
        val -= weights.at(97) * inputs.at(196);
        val += weights.at(98) * inputs.at(98);
        val -= weights.at(98) * inputs.at(197);
        
        mids.at(i) = 1. / ( 1 + exp( -val ) );
    }
    
    return mids;
}

double strategytdorigsym::getOutput( const vector<double>& middles ) const
{
    double sum=0;
    for( int i=0; i<nMiddle; i++ )
        sum += outputWeights.at(i) * ( middles.at(i) - 0.5 );
    return 1. / ( 1 + exp( -sum ) );
}

bool strategytdorigsym::needsUpdate() const
{
    return learning;
}

void strategytdorigsym::update( const board& oldBoard, const board& newBoard )
{
    // get the old board's probability of win and input/middle layer values
    
    vector<double> oldInputs( getInputValues( oldBoard, true ) );
    vector<double> oldMids( getMiddleValues( oldInputs ) );
    double oldProb = getOutput( oldMids );
    
    // calculate the partial deriv of the output node value wrt weights.
    // Two sets of weights: the ones from output->middle, and the ones
    // from each middle->input.
    
    vector<double> dProbdMiddle(nMiddle);
    vector< vector<double> > dProbdInput(nMiddle);
    
    int i, j;
    double probProd = oldProb * ( 1 - oldProb ); // cache it to avoid recalcing it over and over
    double midProd, midVal, outWeight, input, mirrorInput;
    
    for( i=0; i<nMiddle; i++ )
    {
        midVal = oldMids.at(i);
        dProbdMiddle.at(i) = probProd * midVal;
        dProbdInput.at(i).resize(99);
        midProd = midVal * ( 1 - midVal ); // to avoid 196 recalcs
        outWeight = outputWeights.at(i);
        for( j=0; j<99; j++ )
        {
            input = oldInputs.at(j);
            if( j<96 )
                mirrorInput = oldInputs.at((47-j/4)*4 + j%4 + 3);
            else
                mirrorInput = oldInputs.at(j+99); // for the # hit, the # born in, and whose turn it is

            dProbdInput.at(i).at(j) = probProd * outWeight * midProd * ( input - mirrorInput );
        }
    }
    
    // figure out the new network value
    
    double newProb;
    
    if( newBoard.bornIn() == 15 )
        newProb = 1;
    else
        newProb = getOutput( getMiddleValues( getInputValues( newBoard, false ) ) );
    
    // train the weights
    
    double probDiff = newProb - oldProb;
    
    for( i=0; i<nMiddle; i++ )
    {
        outputWeights.at(i) += alpha * probDiff * dProbdMiddle.at(i);
        vector<double>& mids = middleWeights.at(i);
        for( j=0; j<99; j++ )
            mids.at(j) += beta * probDiff * dProbdInput.at(i).at(j);
    }
}

