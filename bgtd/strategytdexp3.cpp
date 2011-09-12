//
//  strategytdexp3.cpp
//  bgtd
//
//  Created by Mark Higgins on 8/28/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <cmath>
#include <fstream>
#include <iostream>
#include "strategytdexp3.h"
#include "randomc.h"

strategytdexp3::strategytdexp3()
{
    nMiddle = 20;
    
    int i, j;
    
    // size the weights and eligibility trace vectors appropriately
    
    outputProbWeights.resize( nMiddle-1 );
    outputGammonWinWeights.resize( nMiddle+1 );
    middleWeights.resize( nMiddle );
    
    for( i=0; i<nMiddle; i++ )
        middleWeights.at(i).resize(99);
    
    // assign random weights
    
    CRandomMersenne rng(1);
    for( i=0; i<nMiddle+1; i++ )
    {
        if( i < nMiddle - 1 ) outputProbWeights.at(i) = rng.IRandom(-100,100)/1000.;
        outputGammonWinWeights.at(i) = rng.IRandom(-100,100)/1000.;
    }
    for( i=0; i<nMiddle; i++ )
        for( j=0; j<99; j++ )
            middleWeights.at(i).at(j) = rng.IRandom(-100,100)/1000.;
    
    // allocate memory for the partial deriv vectors
    
    probDerivs.resize( nMiddle-1 );
    gamWinDerivs.resize( nMiddle+1 );
    probInputDerivs.resize( nMiddle );
    gamWinInputDerivs.resize( nMiddle );
    
    for( i=0; i<nMiddle; i++ )
    {
        probInputDerivs.at(i).resize(99);
        gamWinInputDerivs.at(i).resize(99);
    }

    // put in default values for alpha and lambda (the learning process parameters)
    
    alpha  = 0.1;
    beta   = 0.1;
    lambda = 0.; // ignored in update and assumed to be zero
}

strategytdexp3::strategytdexp3( const strategytdexp& baseStrat )
{
    setupExp( baseStrat );
}

strategytdexp3::strategytdexp3( const string& pathEnd, const string& fileSuffix, bool loadExp )
{
    if( loadExp )
    {
        strategytdexp s( pathEnd, fileSuffix );
        setupExp( s );
    }
    else
    {
        throw string( "Not implemented yet" );
    }
}

void strategytdexp3::setupExp( const strategytdexp& baseStrat )
{
    // make the number of middle nodes the same
    
    nMiddle = baseStrat.nMiddle;
    
    // define lambda = 0 always, but grab alpha and beta from the base strategy
    
    alpha  = baseStrat.alpha;
    beta   = baseStrat.beta;
    lambda = 0;
    
    // grab the weights directly from the base strategy for the output nodes
    
    outputProbWeights       = baseStrat.getOutputProbWeights();
    outputGammonWinWeights  = baseStrat.getOutputGammonWeights();
    middleWeights           = baseStrat.getMiddleWeights();
    
    int i;
    
    for( i=0; i<nMiddle; i++ )
    {
        // add a weight to each set of middle weights representing the weight to
        // the new input for whether it's the player's turn. Initialize it to zero.
        
        middleWeights.at(i).insert( middleWeights.at(i).end(), 0. );
    }
    
    // allocate memory for the partial deriv vectors
    
    probDerivs.resize( nMiddle-1 );
    gamWinDerivs.resize( nMiddle+1 );
    probInputDerivs.resize( nMiddle );
    gamWinInputDerivs.resize( nMiddle );
    
    for( i=0; i<nMiddle; i++ )
    {
        probInputDerivs.at(i).resize(99);
        gamWinInputDerivs.at(i).resize(99);
    }
}

double strategytdexp3::boardValue( const board& brd ) const
{
    // get the inputs from the board
    
    vector<double> inputs = getInputValues( brd, true );
    
    // calculate the middle layer node values
    
    vector<double> middles = getMiddleValues( inputs );
    
    // calculate the output node values from the middles
    
    double probWin            = getOutputProbValue( middles );
    double probCondGammonWin  = getOutputGammonWinValue( middles, brd );
    double probCondGammonLoss = getOutputGammonLossValue( middles, brd );
    
    // calculate the expected number of points the white player will win. probWin
    // corresponds to the probability of a win (any win); probCondGammon
    // corresponds to the probability of a gammon conditional on a win;
    // probCondGammonLoss corresponds to the probability of a gammon loss
    // conditional on a loss.
    
    return probWin         * ( 1 * ( 1 - probCondGammonWin )  + 2 * probCondGammonWin  )
         - ( 1 - probWin ) * ( 1 * ( 1 - probCondGammonLoss ) + 2 * probCondGammonLoss );
}

vector<double> strategytdexp3::getInputValues( const board& brd, bool holdDice ) const
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

vector<double> strategytdexp3::getMiddleValues( const vector<double>& inputs ) const
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
        // in the first half of the inputs = -weight at position (47-j/4)*4 + j%4 + 3, where j/4 does the
        // floor to the nearest integer. So 0->191, 3->194, 4->187, 7->190, and so on.
        
        for( j=0; j<96; j++ )
        {
            val += middleWeights.at(i).at(j) * inputs.at(j);
            val -= middleWeights.at(i).at(j) * inputs.at((47-j/4)*4+j%4+3); // weights for 2nd half equal negative of weights for 1st half
        }
        
        // the last three inputs in the 1st half are the # hit, the # born off, and whether it's the player's turn,
        // which map directly to three at the end of the 2nd half
        
        val += middleWeights.at(i).at(96) * inputs.at(96);
        val -= middleWeights.at(i).at(96) * inputs.at(195);
        val += middleWeights.at(i).at(97) * inputs.at(97);
        val -= middleWeights.at(i).at(97) * inputs.at(196);
        val += middleWeights.at(i).at(98) * inputs.at(98);
        val -= middleWeights.at(i).at(98) * inputs.at(197);
        weights.at(i) = 1. / ( 1 + exp( -val ) );
    }
    
    return weights;
}

double strategytdexp3::getOutputProbValue( const vector<double>& middles ) const
{
    double val=0, sum=0;
    
    // the symmetry for prob->1-prob on flipping board perspective requires that the sum of the
    // output->middle weights equal zero. So the output weights has only nMiddle-1 elements, and
    // we calculate the last element as -1*sum of the rest.
    
    for( int i=0; i<nMiddle-1; i++ )
    {
        val += outputProbWeights.at(i) * middles.at(i);
        sum += outputProbWeights.at(i);
    }
    val -= sum * middles.at(nMiddle-1); // the sum of the output weights for all nodes must be zero
    return 1. / ( 1 + exp( -val ) );
}

double strategytdexp3::getOutputGammonWinValue( const vector<double>& middles, const board& brd ) const
{
    // special case - if the other player has taken any pieces in, the gammon win prob is zero
    
    if( brd.otherBornIn() > 0 ) return 0;
    
    // otherwise calculate the network value
    
    double val=0;
    
    for( int i=0; i<nMiddle; i++ )
        val += outputGammonWinWeights.at(i) * middles.at(i);
    val += outputGammonWinWeights.at(nMiddle); // no weighting by middle node value for the final weight
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexp3::getOutputGammonLossValue( const vector<double>& middles, const board& brd ) const
{
    // special case - if the player has taken any pieces in, the gammon loss prob is zero
    
    if( brd.bornIn() > 0 ) return 0;
    
    // otherwise calculate the network value
    
    double val=0, sum=0, v;
    
    // we require that flipping perspective means that the prob of a gammon win->prob
    // of a gammon loss. That means that the weights of the gammon loss cond on loss
    // output vs the middle node values are -1*the weights of the gammon win cond on
    // win node. Also the bias weight on the gammon loss node equals the bias weight
    // on the gammon win node, plus the sum of the gammon win weights.
    
    for( int i=0; i<nMiddle; i++ )
    {
        v    = outputGammonWinWeights.at(i);
        val -= v * middles.at(i);
        sum += v;
    }
    val += outputGammonWinWeights.at(nMiddle) + sum; // no weighting by middle node value for the final weight
    return  1. / ( 1 + exp( -val ) );
}

vector<double> strategytdexp3::getOutputProbWeights() const { return outputProbWeights; }
vector<double> strategytdexp3::getOutputGammonWinWeights() const { return outputGammonWinWeights; }
vector< vector<double> > strategytdexp3::getMiddleWeights() const { return middleWeights; }

bool strategytdexp3::needsUpdate() const
{
    return learning;
}

void strategytdexp3::update( const board& oldBoard, const board& newBoard )
{
    updateLocal( oldBoard, newBoard, true );
}

void strategytdexp3::updateLocal( const board& oldBoard, const board& newBoard, bool holdDice )
{
    // get the values from the old board
    
    vector<double> oldInputs  = getInputValues( oldBoard, holdDice );
    vector<double> oldMiddles = getMiddleValues( oldInputs );
    double oldProbOutput      = getOutputProbValue( oldMiddles );
    double oldGammonWinOutput = getOutputGammonWinValue( oldMiddles, oldBoard );
    
    // update the eligibility traces
    
    int i, j;
    double input, mirrorInput, finalOutProbWeight=0, outProbWeight, outGammonWeight, midVal;
    double midValLast = oldMiddles.at(nMiddle-1);
    
    for( i=0; i<nMiddle; i++ )
    {
        midVal = oldMiddles.at(i);
        
        if( i < nMiddle - 1 )
        {
            probDerivs.at(i)    = ( midVal - midValLast ) * oldProbOutput * ( 1 - oldProbOutput );
            outProbWeight       = outputProbWeights.at(i);
            finalOutProbWeight -= outProbWeight;
        }
        else
            // the prob of win output weights have nMiddle-1 values because the final nMiddle'th weight is constrained to equal the sum of all the
            // previous weights. This is so that we ensure symmetry of the probabilities on flipping the board perspective.
            
            outProbWeight = finalOutProbWeight;
        
        // the prob of gammon win conditional on win output node does not require the same constraint on sum of weights==0; in fact, it
        // has an extra weight called a "bias weight" which is multiplied by 1 instead of a middle node value. But there are nMiddle
        // weights that *are* weighted by middle node values and we need to calculate the partials for them.
        
        gamWinDerivs.at(i) = midVal * oldGammonWinOutput * ( 1 - oldGammonWinOutput );
        outGammonWeight = outputGammonWinWeights.at(i);
        
        for( j=0; j<99; j++ )
        {
            input = oldInputs.at(j);
            if( j<96 )
                mirrorInput = oldInputs.at((47-j/4)*4 + j%4 + 3);
            else
                mirrorInput = oldInputs.at(j+99); // for the # hit, the # born in, and whose turn it is
            probInputDerivs.at(i).at(j)   = outProbWeight   * ( input - mirrorInput ) * oldProbOutput      * ( 1 - oldProbOutput      ) * midVal * ( 1 - midVal );
            gamWinInputDerivs.at(i).at(j) = outGammonWeight * ( input - mirrorInput ) * oldGammonWinOutput * ( 1 - oldGammonWinOutput ) * midVal * ( 1 - midVal );
        }
    }
    
    // calculate the derivative to the final weight of the gammon node, which is the bias weight
    
    gamWinDerivs.at(nMiddle) = oldGammonWinOutput * ( 1 - oldGammonWinOutput );
    
    // update the parameter values based on the new output (or the result if the game is done)
    
    double newProbOutput, newGammonWinOutput;
    bool trainGammonWin = true;
    bool gameOver = true;
    
    if( newBoard.bornIn() == 15 )
    {
        newProbOutput = 1.; // this player definitely won
        if( newBoard.otherBornIn() == 0 )
            newGammonWinOutput = 1.; // gammon or backgammon
        else
            newGammonWinOutput = 0.;
    }
    else if( newBoard.otherBornIn() == 15 )
    {
        newProbOutput = 0.;
        trainGammonWin = false;
    }
    else
    {
        gameOver = false;
        vector<double> midVals( getMiddleValues( getInputValues( newBoard, !holdDice ) ) );
        newProbOutput      = getOutputProbValue( midVals );
        newGammonWinOutput = getOutputGammonWinValue( midVals, newBoard );
    }
    
    // update the weights
    
    for( i=0; i<nMiddle; i++ )
    {
        if( i < nMiddle - 1 )
            outputProbWeights.at(i) += alpha * ( newProbOutput - oldProbOutput ) * probDerivs.at(i);
        
        if( trainGammonWin )
            outputGammonWinWeights.at(i) += alpha * ( newGammonWinOutput - oldGammonWinOutput ) * gamWinDerivs.at(i);
        
        for( j=0; j<99; j++ )
        {
            middleWeights.at(i).at(j) += beta * ( newProbOutput - oldProbOutput ) * probInputDerivs.at(i).at(j);
            if( trainGammonWin )
                middleWeights.at(i).at(j) += beta * ( newGammonWinOutput - oldGammonWinOutput ) * gamWinInputDerivs.at(i).at(j);
        }
    }
    
    // update the final gammon node weight - this is the bias weight.
    
    if( trainGammonWin )
        outputGammonWinWeights.at(nMiddle) += alpha * ( newGammonWinOutput - oldGammonWinOutput ) * gamWinDerivs.at(nMiddle);
    
    // the first time we go through we train from the perspective of the player; also train from the perspective of
    // the other player to be symmetric. Though not really sure why this is required. If I don't do this, then the
    // prob of win is fine but the conditional prob of gammon goes to 100%.
    
    if( false && holdDice )
    {
        // construct copies of the old and new boards, but with perspective flipped.
        
        board oldBoardFlipped( oldBoard );
        oldBoardFlipped.setPerspective( 1 - oldBoardFlipped.perspective() );
        board newBoardFlipped( newBoard );
        newBoardFlipped.setPerspective( 1 - newBoardFlipped.perspective() );
        
        // update the weights again using this perspective, without holding the dice
        
        updateLocal( oldBoardFlipped, newBoardFlipped, false );
    }
}

