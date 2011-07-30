//
//  strategytd.cpp
//  bgtd
//
//  Created by Mark Higgins on 7/24/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <cmath>
#include <string>
#include <iostream>
#include "strategytd.h"
#include "randomc.h"

strategytd::strategytd()
{
    nMiddle = 40;
    setup();
}

strategytd::strategytd( int nMiddle )
{
    if( nMiddle % 2 != 0 ) throw string( "nMiddle must be even" );
    this->nMiddle = nMiddle;
    setup();
}

strategytd::strategytd( const vector<double>& outputWeights, const vector< vector<double> >& middleWeights, const vector<double>& outputTraces, const vector< vector<double> >& middleTraces, double alpha, double beta, double lambda )
{
    nMiddle = (int) middleWeights.size();
    if( nMiddle % 2 != 0 ) throw string( "nMiddle must be even" );
    if( middleWeights.size() != outputWeights.size() + 1 ) throw string( "outputWeights must have one less element than middleWeights" );
    for( int i=0; i<nMiddle; i++ )
        if( middleWeights.at(i).size() != 98 ) throw string( "middleWeights elements must all be 98 long" );
    
    this->outputWeights = outputWeights;
    this->middleWeights = middleWeights;
    this->outputTraces  = outputTraces;
    this->middleTraces  = middleTraces;
    this->alpha         = alpha;
    this->beta          = beta;
    this->lambda        = lambda;
}

strategytd::strategytd( const strategytd& otherStrat )
{
    nMiddle       = otherStrat.nMiddle;
    outputWeights = otherStrat.outputWeights;
    middleWeights = otherStrat.middleWeights;
    outputTraces  = otherStrat.outputTraces;
    middleTraces  = otherStrat.middleTraces;
    alpha         = otherStrat.alpha;
    beta          = otherStrat.beta;
    lambda        = otherStrat.lambda;
    learning      = otherStrat.learning;
}

strategytd::~strategytd()
{
}

void strategytd::setup()
{
    int i, j;
    // size the weights vectors appropriately
    
    outputWeights.resize( nMiddle-1 );
    middleWeights.resize( nMiddle );
    for( i=0; i<nMiddle; i++ ) middleWeights.at(i).resize(98);
    outputTraces.resize( nMiddle-1 );
    middleTraces.resize( nMiddle );
    for( i=0; i<nMiddle; i++ ) middleTraces.at(i).resize(98);
    
    // assign random weights
    
    CRandomMersenne rng(1);
    for( i=0; i<nMiddle; i++ )
    {
        if( i < nMiddle-1 ) outputWeights.at(i) = rng.IRandom(-100,100)/1000.;
        for( j=0; j<98; j++ )
            middleWeights.at(i).at(j) = rng.IRandom(-100,100)/1000.;
    }
    
    // put in default values for alpha and lambda (the learning process parameters)
    
    alpha  = 0.5;
    beta   = 0.5;
    lambda = 0.;
}

vector<double> strategytd::getInputValues( const board& brd ) const
{
    vector<double> inputs;
    inputs.resize(196,0);
    vector<int> checks = brd.checkers();
    vector<int> otherChecks = brd.otherCheckers();
    
    int i, j;
    
    // we put values for the first player in the first half of the inputs and for the second player
    // in the second half. We do this because we want to ensure that the network knows that the odds
    // of player 2 winning is 1 - odds of player 1 winning; that basically means that the middle weights
    // for j=nMiddle/2->nMiddle-1 are the negative of the weights for j=0->nMiddle/2-1, with a bit of fiddling. 
    // It also means that the sum of the output weights must be zero.
    
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

vector<double> strategytd::getMiddleValues( const vector<double>& inputs ) const
{
    vector<double> weights;
    weights.resize( nMiddle );
    int i, j;
    double val;
    for( i=0; i<nMiddle; i++ )
    {
        val = 0;
        
        // we take advantage of symmetries of the layout when we flip a board's perspective to ensure
        // that the network's probability of player 1 winning == 1 - prob of player 2 winning on a 
        // perspective flip. We know that on a flip the checker layout is exactly reversed from what
        // it is before the flip, but the inputs for each spot come in groups of 4. So the weight
        // in the first half of the inputs = -weight at position (47-j/4)*4 + j%4 + 2, where j/4 does the
        // floor to the nearest integer. So 0->190, 3->193, 4->186, 7->189, and so on.
        
        for( j=0; j<96; j++ )
        {
            val += middleWeights.at(i).at(j) * inputs.at(j);
            val -= middleWeights.at(i).at(j) * inputs.at((47-j/4)*4+j%4+2); // weights for 2nd half equal negative of weights for 1st half
        }
        
        // the last two inputs in the 1st half are the # hit and the # born off, which map directly to two at the end of the 2nd half
        
        val += middleWeights.at(i).at(96) * inputs.at(96);
        val -= middleWeights.at(i).at(96) * inputs.at(194);
        val += middleWeights.at(i).at(97) * inputs.at(97);
        val -= middleWeights.at(i).at(97) * inputs.at(195);
        weights.at(i) = 1. / ( 1 + exp( -val ) );
    }
    
    return weights;
}

double strategytd::getOutput( const vector<double>& middles ) const
{
    double val, sum=0;
    for( int i=0; i<nMiddle-1; i++ )
    {
        val += outputWeights.at(i) * middles.at(i);
        sum += outputWeights.at(i);
    }
    val -= sum * middles.at(nMiddle-1); // the sum of the output weights for all nodes must be zero
    return 1. / ( 1 + exp( -val ) );
}

double strategytd::boardValue( const board& brd ) const
{
    // get the inputs from the board
    
    vector<double> inputs = getInputValues( brd );
    
    // calculate the middle layer node values
    
    vector<double> middles = getMiddleValues( inputs );
    
    // calculate the output node value from the middles
    
    double output = getOutput( middles );
    
    return output;
}

vector<double> strategytd::getOutputWeights() const { return outputWeights; }
vector< vector<double> > strategytd::getMiddleWeights() const { return middleWeights; }
vector<double> strategytd::getOutputTraces() const { return outputTraces; }
vector< vector<double> > strategytd::getMiddleTraces() const { return middleTraces; }

bool strategytd::needsUpdate() const
{
    return learning;
}

void strategytd::update( const board& oldBoard, const board& newBoard )
{
    // get the values from the old board
    
    vector<double> oldInputs = getInputValues( oldBoard );
    vector<double> oldMiddles = getMiddleValues( oldInputs );
    double oldOutput = getOutput( oldMiddles );
    
    // update the eligibility traces
    
    int i, j;
    double input, mirrorInput, finalOutWeight=0, outWeight;
    
    for( i=0; i<nMiddle; i++ )
    {
        if( i < nMiddle - 1 ) 
        {
            outputTraces.at(i) = lambda * outputTraces.at(i) + ( oldMiddles.at(i) - oldMiddles.at(nMiddle-1) ) * oldOutput * ( 1 - oldOutput );
            outWeight          = outputWeights.at(i);
            finalOutWeight    -= outWeight;
        }
        else
            outWeight = finalOutWeight;
        
        for( j=0; j<98; j++ )
        {
            input = oldInputs.at(j);
            if( j<96 )
                mirrorInput = oldInputs.at((47-j/4)*4 + j%4 + 2);
            else
                mirrorInput = oldInputs.at(j+98); // for the # hit and the # born in
            middleTraces.at(i).at(j) = lambda * middleTraces.at(i).at(j) + outWeight * ( input - mirrorInput ) * oldOutput * ( 1 - oldOutput ) * oldMiddles.at(i) * ( 1 - oldMiddles.at(i) );
        }
    }
    
    // update the parameter values based on the new output (or the result if the game is done)
    
    double newOutput;
    
    if( newBoard.bornIn() == 15 )
        newOutput = 1.; // this player definitely won
    else if( newBoard.otherBornIn() == 15 )
        newOutput = 0.; // this player definitely lost
    else
        newOutput = getOutput( getMiddleValues( getInputValues( newBoard ) ) ); // estimate from the next step
    
    for( i=0; i<nMiddle; i++ )
    {
        if( i < nMiddle - 1 ) outputWeights.at(i) += alpha * ( newOutput - oldOutput ) * outputTraces.at(i);
        for( j=0; j<98; j++ )
            middleWeights.at(i).at(j) += beta * ( newOutput - oldOutput ) * middleTraces.at(i).at(j);
    }
}