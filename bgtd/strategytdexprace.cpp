//
//  strategytdexprace.cpp
//  bgtd
//
//  Created by Mark Higgins on 8/21/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <cmath>
#include <iostream>
#include "strategytdexprace.h"
#include "gamefns.h"
#include "randomc.h"

strategytdexprace::strategytdexprace()
{
    nMiddle = 40;
    setupAll();
}

strategytdexprace::strategytdexprace( const strategytdexp& baseStrat )
{
    loadExpValues( baseStrat );
    setupRace();
}

void strategytdexprace::loadExpValues( const strategytdexp& baseStrat )
{
    nMiddle  = baseStrat.nMiddle;
    learning = baseStrat.learning;
    alpha    = baseStrat.alpha;
    beta     = baseStrat.beta;
    lambda   = baseStrat.lambda;
    
    // note that this loads only nMiddle-1 prob weights - setupRace
    // fixes this.
    
    outputProbWeights   = baseStrat.getOutputProbWeights();
    outputGammonWeights = baseStrat.getOutputGammonWeights();
    middleWeights       = baseStrat.getMiddleWeights();
    outputProbTraces    = baseStrat.getOutputProbTraces();
    outputGammonTraces  = baseStrat.getOutputGammonTraces();
    middleProbTraces    = baseStrat.getMiddleProbTraces();
    middleGammonTraces  = baseStrat.getMiddleGammonTraces();
}

strategytdexprace::strategytdexprace( const string& pathEnd, const string& fileSuffix, bool loadRace )
{
    // load the original expanded strategy values
    
    strategytdexp s( pathEnd, fileSuffix );
    loadExpValues( s );
    
    // if we're meant to load the race network values too, grab those from the files; otherwise initialize
    // them randomly
    
    if( loadRace )
    {
        throw string( "Not implemented yet" );
    }
    else
    {
        setupRace(); // initialize all the vectors - sets random values for the weights, sets traces to zero
        
        // initialize the weights in the race network to the generic one as a better starting
        // point than random weights
        
        outputGammonWeightsRace = s.getOutputGammonWeights();
        middleWeightsRace       = s.getMiddleWeights();
        
        // use the prob weights from the tdexp source; but that has one less element than this one does.
        
        double sum=0;
        outputProbWeightsRace.resize(nMiddle);
        vector<double> srcWeights( s.getOutputProbWeights() );
        for( int i=0; i<nMiddle-1; i++ )
        {
            outputProbWeightsRace.at(i) = srcWeights.at(i);
            sum += srcWeights.at(i);
        }
        outputProbWeightsRace.at(nMiddle-1) = -sum;
    }
}

void strategytdexprace::setupAll()
{
    setup();
    setupRace();
}

void strategytdexprace::setupRace()
{
    // extend the number of output prob weights to nMiddle. It was nMiddle-1 in
    // strategytdexp, because the prob node didn't have a bias node. We add one
    // here so that the probability of a win from the starting board isn't exactly
    // 50% for the player with the first roll.
    
    // set the prob last trace to zero, and set the weight to the sum of the other ones.
    
    int i, j;
    
    outputProbTraces.insert(outputProbTraces.end(), 0);
    double sum=0;
    for( i=0; i<nMiddle-1; i++ ) sum += outputProbWeights.at(i);
    outputProbWeights.insert(outputProbWeights.end(), -sum);
    
    // size the weights and eligibility trace vectors appropriately
    
    outputProbWeightsRace.resize( nMiddle );
    outputGammonWeightsRace.resize( nMiddle+1 );
    outputProbTracesRace.resize( nMiddle, 0 );
    outputGammonTracesRace.resize( nMiddle+1, 0 );
    middleWeightsRace.resize( nMiddle );
    middleProbTracesRace.resize( nMiddle );
    middleGammonTracesRace.resize( nMiddle );
    
    for( i=0; i<nMiddle; i++ )
    {
        middleWeightsRace.at(i).resize(98);
        middleProbTracesRace.at(i).resize(98,0);
        middleGammonTracesRace.at(i).resize(98,0);
    }
    
    // assign random weights
    
    CRandomMersenne rng(1);
    for( i=0; i<nMiddle; i++ )
    {
        outputProbWeightsRace.at(i) = rng.IRandom(-100,100)/1000.;
        outputGammonWeightsRace.at(i) = rng.IRandom(-100,100)/1000.;
    }
    outputGammonWeightsRace.at(nMiddle) = rng.IRandom(-100,100)/1000.;
    for( i=0; i<nMiddle; i++ )
        for( j=0; j<98; j++ )
            middleWeightsRace.at(i).at(j) = rng.IRandom(-100,100)/1000.;
}

double strategytdexprace::boardValue( const board& brd ) const
{
    // get the inputs from the board
    
    vector<double> inputs( getInputValues( brd ) );
    
    double probWin, probCondGammon, probCondGammonLoss;
    
    //if( isRace( brd ) )
    if( false )
    {
        // calculate the middle layer node values from the rate network
        
        vector<double> middles( getMiddleValuesRace( inputs ) );
        
        // calculate the output node values from the middles
        
        probWin            = getOutputProbValueRace( middles );
        probCondGammon     = getOutputGammonValueRace( middles, brd );
        probCondGammonLoss = getOutputGammonLossValueRace( middles,brd );
    }
    else
    {
        // calculate the middle layer node values from the contact network

        vector<double> middles( getMiddleValues( inputs ) );
        
        // calculate the output node values from the middles
        
        probWin            = getOutputProbValue( middles );
        probCondGammon     = getOutputGammonValue( middles, brd );
        probCondGammonLoss = getOutputGammonLossValue( middles, brd );
    }
        
    // calculate the expected number of points the player will win. probWin
    // corresponds to the probability of a win (any win); probCondGammon
    // corresponds to the probability of a gammon conditional on a win;
    // probCondGammonLoss corresponds to the probability of a gammon loss
    // conditional on a loss.
    
    return probWin         * ( ( 1 - probCondGammon )     + 2 * probCondGammon )
         - ( 1 - probWin ) * ( ( 1 - probCondGammonLoss ) + 2 * probCondGammonLoss );
}

// add new values for the network we use when there's a race

vector<double> strategytdexprace::getMiddleValuesRace( const vector<double>& inputs ) const
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
            val += middleWeightsRace.at(i).at(j) * inputs.at(j);
            val -= middleWeightsRace.at(i).at(j) * inputs.at((47-j/4)*4+j%4+2); // weights for 2nd half equal negative of weights for 1st half
        }
        
        // the last two inputs in the 1st half are the # hit and the # born off, which map directly to two at the end of the 2nd half
        
        val += middleWeightsRace.at(i).at(96) * inputs.at(96);
        val -= middleWeightsRace.at(i).at(96) * inputs.at(194);
        val += middleWeightsRace.at(i).at(97) * inputs.at(97);
        val -= middleWeightsRace.at(i).at(97) * inputs.at(195);
        weights.at(i) = 1. / ( 1 + exp( -val ) );
    }
    
    return weights;
}

double strategytdexprace::getOutputProbValue( const vector<double>& middles ) const
{
    double val=0;
    
    // the symmetry for prob->1-prob on flipping board perspective requires that the bias node
    // value is -1/2 sum(weights), so we can write the fn as 1/(1+exp( -sum( (middle[i]-1/2) * weight[i] ) )
    
    for( int i=0; i<nMiddle; i++ )
        val += outputProbWeights.at(i) * ( middles.at(i) - 0.5 );
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexprace::getOutputGammonValue( const vector<double>& middles, const board& brd ) const
{
    // special case: if there are of the other player's checkers borne in, we know the
    // probability of gammon is zero.
    
    if( brd.otherBornIn() > 0 ) return 0;
    
    // otherwise calculate the value from the network
    
    double val=0;
    
    for( int i=0; i<nMiddle; i++ )
        val += outputGammonWeights.at(i) * middles.at(i);
    val += outputGammonWeights.at(nMiddle); // no weighting by middle node value for the final weight
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexprace::getOutputGammonLossValue( const vector<double>& middles, const board& brd ) const
{
    // special case: if there are of the this player's checkers borne in, we know the
    // probability of gammon loss is zero.
    
    if( brd.bornIn() > 0 ) return 0;
    
    // otherwise calculate the value from the network
    
    double val=0, sum=0, v;
    
    // we require that flipping perspective means that the prob of a gammon win->prob
    // of a gammon loss. That means that the weights of the gammon loss cond on loss
    // output vs the middle node values are -1*the weights of the gammon win cond on
    // win node. Also the bias weight on the gammon loss node equals the bias weight
    // on the gammon win node, plus the sum of the gammon win weights.
    
    for( int i=0; i<nMiddle; i++ )
    {
        v    = outputGammonWeights.at(i);
        val -= v * middles.at(i);
        sum += v;
    }
    val += outputGammonWeights.at(nMiddle) + sum; // no weighting by middle node value for the final weight
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexprace::getOutputProbValueRace( const vector<double>& middles ) const
{
    double val=0;
    
    // the symmetry for prob->1-prob on flipping board perspective requires that the bias node
    // value is -1/2 sum(weights), so we can write the fn as 1/(1+exp( -sum( (middle[i]-1/2) * weight[i] ) )
    
    for( int i=0; i<nMiddle; i++ )
        val += outputProbWeightsRace.at(i) * ( middles.at(i) - 0.5 );
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexprace::getOutputGammonValueRace( const vector<double>& middles, const board& brd ) const
{
    // special case: if there are of the other player's checkers borne in, we know the
    // probability of gammon is zero.
    
    if( brd.otherBornIn() > 0 ) return 0;
    
    // otherwise calculate the value from the network
    
    double val=0;
    
    for( int i=0; i<nMiddle; i++ )
        val += outputGammonWeightsRace.at(i) * middles.at(i);
    val += outputGammonWeightsRace.at(nMiddle); // no weighting by middle node value for the final weight
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexprace::getOutputGammonLossValueRace( const vector<double>& middles, const board& brd ) const
{
    // special case: if there are of the this player's checkers borne in, we know the
    // probability of gammon loss is zero.
    
    if( brd.bornIn() > 0 ) return 0;
    
    // otherwise calculate the value from the network
    
    double val=0, sum=0, v;
    
    // we require that flipping perspective means that the prob of a gammon win->prob
    // of a gammon loss. That means that the weights of the gammon loss cond on loss
    // output vs the middle node values are -1*the weights of the gammon win cond on
    // win node. Also the bias weight on the gammon loss node equals the bias weight
    // on the gammon win node, plus the sum of the gammon win weights.
    
    for( int i=0; i<nMiddle; i++ )
    {
        v    = outputGammonWeightsRace.at(i);
        val -= v * middles.at(i);
        sum += v;
    }
    val += outputGammonWeightsRace.at(nMiddle) + sum; // no weighting by middle node value for the final weight
    return  1. / ( 1 + exp( -val ) );
}

vector<double> strategytdexprace::getOutputProbWeightsRace() const { return outputProbWeightsRace; }
vector<double> strategytdexprace::getOutputGammonWeightsRace() const { return outputGammonWeightsRace; }
vector< vector<double> > strategytdexprace::getMiddleWeightsRace() const { return middleWeightsRace; }
vector<double> strategytdexprace::getOutputProbTracesRace() const { return outputProbTracesRace; }
vector<double> strategytdexprace::getOutputGammonTracesRace() const { return outputGammonTracesRace; }
vector< vector<double> > strategytdexprace::getMiddleProbTracesRace() const { return middleProbTracesRace; }
vector< vector<double> > strategytdexprace::getMiddleGammonTracesRace() const { return middleGammonTracesRace; }

void strategytdexprace::update( const board& oldBoard, const board& newBoard )
{
    // get the inputs from the board. These are the same for both networks
    
    vector<double> oldInputs = getInputValues( oldBoard );
    
    // figure out which weights and traces we're dealing with, and get the corresponding values
    // from the old board.
    
    vector<double> * outputProbWeightsUsed;
    vector<double> * outputGammonWeightsUsed;
    vector<double> * outputProbTracesUsed;
    vector<double> * outputGammonTracesUsed;
    vector< vector<double> > * middleWeightsUsed;
    vector< vector<double> > * middleProbTracesUsed;
    vector< vector<double> > * middleGammonTracesUsed;
    
    vector<double> oldMiddles;
    double oldProbOutput, oldGammonOutput;
    bool oldGameIsRace;
    
    //if( isRace( oldBoard ) )
    if( false )
    {
        oldGameIsRace = true;
        
        // use the values from the race network
        
        oldMiddles      = getMiddleValuesRace( oldInputs );
        oldProbOutput   = getOutputProbValueRace( oldMiddles );
        oldGammonOutput = getOutputGammonValueRace( oldMiddles, oldBoard );
        
        outputProbWeightsUsed   = &outputProbWeightsRace;
        outputGammonWeightsUsed = &outputGammonWeightsRace;
        outputProbTracesUsed    = &outputProbTracesRace;
        outputGammonTracesUsed  = &outputGammonTracesRace;
        middleWeightsUsed       = &middleWeightsRace;
        middleProbTracesUsed    = &middleProbTracesRace;
        middleGammonTracesUsed  = &middleGammonTracesRace;
    }
    else
    {
        oldGameIsRace = false;
        
        // use the values from the contact network
        
        oldMiddles      = getMiddleValues( oldInputs );
        oldProbOutput   = getOutputProbValue( oldMiddles );
        oldGammonOutput = getOutputGammonValue( oldMiddles, oldBoard );
        
        outputProbWeightsUsed   = &outputProbWeights;
        outputGammonWeightsUsed = &outputGammonWeights;
        outputProbTracesUsed    = &outputProbTraces;
        outputGammonTracesUsed  = &outputGammonTraces;
        middleWeightsUsed       = &middleWeights;
        middleProbTracesUsed    = &middleProbTraces;
        middleGammonTracesUsed  = &middleGammonTraces;
    }
    
    // update the eligibility traces
    
    int i, j;
    double input, mirrorInput, outProbWeight, outGammonWeight, midVal;
    
    for( i=0; i<nMiddle; i++ )
    {
        midVal = oldMiddles.at(i);
        
        // the prob of win depends on a weight in two ways: directly in terms of the factor in front of the middle node value,
        // and also through the dependence of the bias weight, which is -1/2 the sum of the other weights to enforce the
        // perspective flip symmetry. So while this node has a bias weight, the value of that bias is constrained by the
        // other weights and is not trained explicitly (though it does enter the output prob trace since it changes the
        // partial derivative of prob of win wrt each weight).
        
        outputProbTracesUsed->at(i) = lambda * outputProbTracesUsed->at(i) + ( midVal - 0.5 ) * oldProbOutput * ( 1 - oldProbOutput );
        outProbWeight = outputProbWeightsUsed->at(i);
        
        // the prob of gammon win conditional on win output node has a bias node but its value is not constrained by the
        // perspective flip symmetry.
        
        outputGammonTracesUsed->at(i) = lambda * outputGammonTracesUsed->at(i) + midVal * oldGammonOutput * ( 1 - oldGammonOutput );
        outGammonWeight = outputGammonWeightsUsed->at(i);
        
        for( j=0; j<98; j++ )
        {
            input = oldInputs.at(j);
            if( j<96 )
                mirrorInput = oldInputs.at((47-j/4)*4 + j%4 + 2);
            else
                mirrorInput = oldInputs.at(j+98); // for the # hit and the # born in
            middleProbTracesUsed->at(i).at(j)   = lambda * middleProbTracesUsed->at(i).at(j)   + outProbWeight   * ( input - mirrorInput ) * oldProbOutput   * ( 1 - oldProbOutput   ) * midVal * ( 1 - midVal );
            middleGammonTracesUsed->at(i).at(j) = lambda * middleGammonTracesUsed->at(i).at(j) + outGammonWeight * ( input - mirrorInput ) * oldGammonOutput * ( 1 - oldGammonOutput ) * midVal * ( 1 - midVal );
        }
    }
    
    // update the trace for the final weight of the gammon node, which is its (unconstrained) bias weight
    
    outputGammonTracesUsed->at(nMiddle) = lambda * outputGammonTracesUsed->at(nMiddle) + oldGammonOutput * ( 1 - oldGammonOutput );
    
    // update the parameter values based on the new output (or the result if the game is done)
    
    double newProbOutput, newGammonOutput;

    if( newBoard.bornIn() == 15 )
    {
        newProbOutput = 1.; // this player definitely won
        if( newBoard.otherBornIn() == 0 )
            newGammonOutput = 1.; // gammon or backgammon
        else
            newGammonOutput = 0;
    }
    else if( newBoard.otherBornIn() == 15 )
    {
        // this should never happend - the winner always makes the final move so the update fn
        // only ever gets an ending board from the winner's perspective.
        
        throw string( "Should never see a losing final board in the update function" );
    }
    else
    {
        // get the calculated values from the new board. This is where the two networks potentially
        // cross: if the old board wasn't a race but the new one is, the estimated probs of win/gammon
        // come from the race network but the weights that are adjusted are the weights from the
        // contact network.
        
        //if( oldGameIsRace || isRace( newBoard ) )
        if( false )
        {
            vector<double> midVals( getMiddleValuesRace( getInputValues( newBoard ) ) );
            newProbOutput   = getOutputProbValueRace( midVals );
            newGammonOutput = getOutputGammonValueRace( midVals, newBoard );
        }
        else
        {
            vector<double> midVals( getMiddleValues( getInputValues( newBoard ) ) );
            newProbOutput   = getOutputProbValue(   midVals );
            newGammonOutput = getOutputGammonValue( midVals, newBoard );
        }
    }
    
    // update the weights
    
    for( i=0; i<nMiddle; i++ )
    {
        outputProbWeightsUsed->at(i)   += alpha * ( newProbOutput   - oldProbOutput   ) * outputProbTracesUsed->at(i);
        outputGammonWeightsUsed->at(i) += alpha * ( newGammonOutput - oldGammonOutput ) * outputGammonTracesUsed->at(i);
        
        for( j=0; j<98; j++ )
            middleWeightsUsed->at(i).at(j) += beta * ( ( newProbOutput   - oldProbOutput   ) * middleProbTracesUsed->at(i).at(j)
                                                     + ( newGammonOutput - oldGammonOutput ) * middleGammonTracesUsed->at(i).at(j) );
    }
    
    // update the final gammon node weight - this is the bias weight.
    
    outputGammonWeightsUsed->at(nMiddle) += alpha * ( newGammonOutput - oldGammonOutput ) * outputGammonTracesUsed->at(nMiddle);
}


