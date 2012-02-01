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
#include "strategytdorig.h"

strategytdorig::strategytdorig()
{
    nMiddle = 40; // default # of middle nodes
    setup();
}

strategytdorig::strategytdorig( int nMiddle )
{
    this->nMiddle = nMiddle;
    setup();
}

strategytdorig::~strategytdorig()
{
}

void strategytdorig::setup()
{
    // generates random weights
    
    CRandomMersenne rng(1);
    
    outputWeights.resize(nMiddle+1);
    middleWeights.resize(nMiddle);
    
    int i, j;
    for( i=0; i<nMiddle; i++ )
    {
        outputWeights.at(i) = rng.IRandom(-100, 100)/1000.;
        middleWeights.at(i).resize(196);
        for( j=0; j<196; j++ )
            middleWeights.at(i).at(j) = rng.IRandom(-100, 100)/1000.;
    }
    outputWeights.at(nMiddle) = rng.IRandom(-100, 100)/1000.; // bias weight
    
    // set alpha and beta sensibly. Lambda we always set to zero.
    
    alpha  = 0.1;
    beta   = 0.1;
    lambda = 0;
}

gameProbabilities strategytdorig::boardProbabilities( const board& brd, const hash_map<string,int>* context ) const
{
    // need to flip the board perspective to reflect the fact that the opponent holds the dice
    
    board flippedBoard( brd );
    flippedBoard.setPerspective( 1 - brd.perspective() );
    
    // get the prob of win from the perspective of the opponent
    
    double probWin = getOutput( getMiddleValues( getInputValues( flippedBoard ) ) );
    gameProbabilities probs( 1-probWin, 0, 0, 0, 0 );
    return probs;
}

vector<double> strategytdorig::getInputValues( const board& brd ) const
{
    vector<double> inputs;
    inputs.resize(196,0);
    
    vector<int> checks      = brd.checkers();
    vector<int> otherChecks = brd.otherCheckers();
    
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
            if( otherChecks.at(i) > j ) inputs.at(4*i+j+98) = 1;
        }
        if( checks.at(i) > 3 )      inputs.at(4*i+3)    = ( checks[i]-3 ) / 2.;
        if( otherChecks.at(i) > 3 ) inputs.at(4*i+3+98) = ( otherChecks[i]-3 ) / 2.;
    }
    
    // one spot for each player records the number on the bar
    
    inputs.at(96)  = brd.hit() / 2.;
    inputs.at(194) = brd.otherHit() / 2.;
    
    // one spot for each player records the number born in
    
    inputs.at(97)  = brd.bornIn() / 15.;
    inputs.at(195) = brd.otherBornIn() / 15.;
    
    return inputs;
}

vector<double> strategytdorig::getMiddleValues( const vector<double>& inputs ) const
{
    vector<double> mids(nMiddle);
    
    int i, j;
    for( i=0; i<nMiddle; i++ )
    {
        // each middle value is 1/(1+exp(-sum(weight(j)*input(j)))) - no bias weights for middle nodes
        double sum=0;
        for( j=0; j<196; j++ )
            sum += middleWeights.at(i).at(j) * inputs.at(j);
        mids.at(i) = 1 / ( 1 + exp( -sum ) );
    }
    
    return mids;
}

double strategytdorig::getOutput( const vector<double>& middles ) const
{
    double sum=0;
    for( int i=0; i<nMiddle; i++ )
        sum += outputWeights.at(i) * middles.at(i);
    sum += outputWeights.at(nMiddle); // output node does contain a bias weight
    return 1 / ( 1 + exp( -sum ) );
}

bool strategytdorig::needsUpdate() const
{
    return learning;
}

void strategytdorig::update( const board& oldBoard, const board& newBoard )
{
    // get the old board's probability of win and input/middle layer values
    
    vector<double> oldInputs( getInputValues( oldBoard ) );
    vector<double> oldMids( getMiddleValues( oldInputs ) );
    double oldProb = getOutput( oldMids );
    
    // calculate the partial deriv of the output node value wrt weights.
    // Two sets of weights: the ones from output->middle, and the ones
    // from each middle->input.
    
    vector<double> dProbdMiddle(nMiddle+1);
    vector< vector<double> > dProbdInput(nMiddle);
    
    int i, j;
    double probProd = oldProb * ( 1 - oldProb ); // cache it to avoid recalcing it over and over
    double midProd, midVal, outWeight;
    
    for( i=0; i<nMiddle; i++ )
    {
        midVal = oldMids.at(i);
        dProbdMiddle.at(i) = probProd * midVal;
        dProbdInput.at(i).resize(196);
        midProd = midVal * ( 1 - midVal ); // to avoid 196 recalcs
        outWeight = outputWeights.at(i);
        for( j=0; j<196; j++ )
            dProbdInput.at(i).at(j) = probProd * outWeight * midProd * oldInputs.at(j);
    }
    dProbdMiddle.at(nMiddle) = oldProb * ( 1 - oldProb ); // bias weight deriv
    
    // figure out the new network value
    
    double newProb;
    
    if( newBoard.bornIn() == 15 )
        newProb = 1;
    else
    {
        // need to get the prob from the perspective of the other player when they have
        // the dice, so flip perspective and get the prob of the other player winning,
        // then do 1 - that.
        
        board flippedBoard( newBoard );
        flippedBoard.setPerspective( ( newBoard.perspective() + 1 ) % 2 );
        newProb = 1 - getOutput( getMiddleValues( getInputValues( flippedBoard ) ) );
    }
    
    // train the weights
    
    double probDiff = newProb - oldProb;
    
    for( i=0; i<nMiddle; i++ )
    {
        outputWeights.at(i) += alpha * probDiff * dProbdMiddle.at(i);
        for( j=0; j<196; j++ )
            middleWeights.at(i).at(j) += beta * probDiff * dProbdInput.at(i).at(j);
    }
    outputWeights.at(nMiddle) += alpha * probDiff * dProbdMiddle.at(nMiddle); // bias weight
}
