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

strategytdexp3::strategytdexp3( int nMiddle )
{
    this->nMiddle = nMiddle;
    
    int i, j;
    
    // size the weights and eligibility trace vectors appropriately
    
    outputProbWeights.resize( nMiddle-1 );
    outputGammonWinWeights.resize( nMiddle+1 );
    outputBackgammonWinWeights.resize( nMiddle+1 );
    middleWeights.resize( nMiddle );
    
    for( i=0; i<nMiddle; i++ )
        middleWeights.at(i).resize(99);
    
    // assign random weights
    
    CRandomMersenne rng(1);
    for( i=0; i<nMiddle+1; i++ )
    {
        if( i < nMiddle - 1 ) outputProbWeights.at(i) = rng.IRandom(-100,100)/1000.;
        outputGammonWinWeights.at(i) = rng.IRandom(-100,100)/1000.;
        outputBackgammonWinWeights.at(i) = rng.IRandom(-100,100)/1000.;
    }
    for( i=0; i<nMiddle; i++ )
        for( j=0; j<99; j++ )
            middleWeights.at(i).at(j) = rng.IRandom(-100,100)/1000.;
    
    // allocate memory for the partial deriv vectors
    
    probDerivs.resize( nMiddle-1 );
    gamWinDerivs.resize( nMiddle+1 );
    bgWinDerivs.resize( nMiddle+1 );
    probInputDerivs.resize( nMiddle );
    gamWinInputDerivs.resize( nMiddle );
    bgWinInputDerivs.resize( nMiddle );
    
    for( i=0; i<nMiddle; i++ )
    {
        probInputDerivs.at(i).resize(99);
        gamWinInputDerivs.at(i).resize(99);
        bgWinInputDerivs.at(i).resize(99);
    }

    // put in default values for alpha and lambda (the learning process parameters)
    
    alpha  = 0.1;
    beta   = 0.1;
    lambda = 0.; // ignored in update and assumed to be zero
    
    setFlags();
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
        string path     = "/Users/mghiggins/bgtdres";
        if( pathEnd != "" ) path += "/" + pathEnd;
        string fopName  = path + "/weightsOutProb_" + fileSuffix + ".txt";
        string fogName  = path + "/weightsOutGammonWin_" + fileSuffix + ".txt";
        string fobName  = path + "/weightsOutBgWin_" + fileSuffix + ".txt";
        string fmName   = path + "/weightsMiddle_" + fileSuffix + ".txt";
        
        ifstream fop( fopName.c_str() );
        ifstream fog( fogName.c_str() );
        ifstream fob( fobName.c_str() );
        ifstream fm( fmName.c_str() );
        
        // start by loading the number of middle weights from the prob output weights file
        
        string line;
        getline( fop, line );
        nMiddle = atoi( line.c_str() );
        
        // resize all the vectors appropriately
        
        outputProbWeights.resize( nMiddle - 1 );
        outputGammonWinWeights.resize( nMiddle + 1 );
        outputBackgammonWinWeights.resize( nMiddle + 1 );
        middleWeights.resize( nMiddle );
        
        int i, j;
        
        for( i=0; i<nMiddle; i++ )
            middleWeights[i].resize(99);
        
        // load the output weights for the prob and gammon nodes
        
        for( i=0; i<nMiddle-1; i++ )
        {
            getline( fop, line );
            outputProbWeights[i] = atof( line.c_str() );
        }
        
        for( i=0; i<nMiddle+1; i++ )
        {
            getline( fog, line );
            outputGammonWinWeights[i] = atof( line.c_str() );
        }
        
        for( i=0; i<nMiddle+1; i++ )
        {
            getline( fob, line );
            outputBackgammonWinWeights[i] = atof( line.c_str() );
        }
        
        // load the middle weights and traces
        
        for( i=0; i<nMiddle; i++ )
            for( j=0; j<99; j++ )
            {
                getline( fm, line );
                middleWeights[i][j] = atof( line.c_str() );
            }

        // allocate memory for the partial deriv vectors
        
        probDerivs.resize( nMiddle-1 );
        gamWinDerivs.resize( nMiddle+1 );
        bgWinDerivs.resize( nMiddle+1 );
        probInputDerivs.resize( nMiddle );
        gamWinInputDerivs.resize( nMiddle );
        bgWinInputDerivs.resize( nMiddle );
        
        for( i=0; i<nMiddle; i++ )
        {
            probInputDerivs.at(i).resize(99);
            gamWinInputDerivs.at(i).resize(99);
            bgWinInputDerivs.at(i).resize(99);
        }
        
        // set the learning parameters to some sensible defaults
        
        alpha  = 0.1;
        beta   = 0.1;
        lambda = 0;
        
        setFlags();
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
    
    // grab the weights directly from the base strategy for the output nodes. Use
    // random weights for the backgammon prob node
    
    outputProbWeights       = baseStrat.getOutputProbWeights();
    outputGammonWinWeights  = baseStrat.getOutputGammonWeights();
    outputBackgammonWinWeights.resize( nMiddle+1 );
    middleWeights           = baseStrat.getMiddleWeights();
    
    int i;
    
    for( i=0; i<nMiddle; i++ )
    {
        // add a weight to each set of middle weights representing the weight to
        // the new input for whether it's the player's turn. Initialize it to zero.
        
        middleWeights.at(i).insert( middleWeights.at(i).end(), 0. );
    }
    
    CRandomMersenne rng(1);
    for( i=0; i<nMiddle+1; i++ )
        outputBackgammonWinWeights.at(i) = rng.IRandom( -100, 100 ) / 1000.;
    
    // allocate memory for the partial deriv vectors
    
    probDerivs.resize( nMiddle-1 );
    gamWinDerivs.resize( nMiddle+1 );
    bgWinDerivs.resize( nMiddle+1 );
    probInputDerivs.resize( nMiddle );
    gamWinInputDerivs.resize( nMiddle );
    bgWinInputDerivs.resize( nMiddle );
    
    for( i=0; i<nMiddle; i++ )
    {
        probInputDerivs.at(i).resize(99);
        gamWinInputDerivs.at(i).resize(99);
        bgWinInputDerivs.at(i).resize(99);
    }
    
    setFlags();
}

void strategytdexp3::setFlags()
{
    useBg              = false;
    weightLearning     = true;
    stopGammonTraining = true;
    exactGammon        = true;
    symmetric          = true;
    trainFlipped       = false;
}

double strategytdexp3::boardValue( const board& brd ) const
{
    // get the inputs from the board
    
    vector<double> inputs = getInputValues( brd, true );
    
    // calculate the middle layer node values
    
    vector<double> middles = getMiddleValues( inputs );
    
    // calculate the output node values from the middles
    
    double probWin            = getOutputProbValue( middles );
    double probCondGammonWin  = getOutputGammonWinValue( middles );
    double probCondGammonLoss = getOutputGammonLossValue( middles );
    double probCondBgWin, probCondBgLoss;
    if( useBg )
    {
        probCondBgWin  = getOutputBackgammonWinValue( middles );
        probCondBgLoss = getOutputBackgammonLossValue( middles );
    }
    else
    {
        probCondBgWin  = 0;
        probCondBgLoss = 0;
    }
    
    if( exactGammon )
    {
        // special case: we know when the prob of gammon win or loss == 0 based on
        // whether either side has taken at least 1 in.
        
        if( brd.bornIn() > 0 )      probCondGammonLoss = 0;
        if( brd.otherBornIn() > 0 ) probCondGammonWin  = 0;
    }
    
    // calculate the expected number of points the white player will win. probWin
    // corresponds to the probability of a win (any win); probCondGammon
    // corresponds to the probability of a gammon conditional on a win;
    // probCondGammonLoss corresponds to the probability of a gammon loss
    // conditional on a loss. probCondBgWin/Loss correspond to the probabilities of
    // a backgammon conditioned on a gammon.
    
    return probWin         * ( 1 * ( 1 - probCondGammonWin  ) + probCondGammonWin  * ( 2 * ( 1 - probCondBgWin  ) + 3 * probCondBgWin  ) )
         - ( 1 - probWin ) * ( 1 * ( 1 - probCondGammonLoss ) + probCondGammonLoss * ( 2 * ( 1 - probCondBgLoss ) + 3 * probCondBgLoss ) );
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
    vector<double> vals;
    vals.resize( nMiddle );
    int i, j;
    double val;
    for( i=0; i<nMiddle; i++ )
    {
        vector<double> const & weights( middleWeights.at(i) ); // cache the reference so we don't keep looking up the i'th element of middleWeights below
        
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
        
        vals.at(i) = 1. / ( 1 + exp( -val ) );
    }
    
    return vals;
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

double strategytdexp3::getOutputGammonWinValue( const vector<double>& middles ) const
{
    // this is the raw output for the conditional probability of a gammon win. It does not go to
    // zero automatically when the other player has borne in at least 1 checker; that is handled
    // external to the node calculation.
    
    double val=0;
    
    for( int i=0; i<nMiddle; i++ )
        val += outputGammonWinWeights.at(i) * middles.at(i);
    val += outputGammonWinWeights.at(nMiddle); // no weighting by middle node value for the final weight
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexp3::getOutputGammonLossValue( const vector<double>& middles ) const
{
    // this is the raw output for the conditional probability of a gammon loss. It does not go to
    // zero automatically when the player has borne in at least 1 checker; that is handled
    // external to the node calculation.
    
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

double strategytdexp3::getOutputBackgammonWinValue( const vector<double>& middles ) const
{
    // this is the raw output for the conditional probability of a backgammon win. 
    
    double val=0;
    
    for( int i=0; i<nMiddle; i++ )
        val += outputBackgammonWinWeights.at(i) * middles.at(i);
    val += outputBackgammonWinWeights.at(nMiddle); // no weighting by middle node value for the final weight
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexp3::getOutputBackgammonLossValue( const vector<double>& middles ) const
{
    // this is the raw output for the conditional probability of a backgammon loss. 
    
    double val=0, sum=0, v;
    
    // we require that flipping perspective means that the prob of a backgammon win->prob
    // of a backgammon loss. That means that the weights of the backgammon loss cond on loss
    // output vs the middle node values are -1*the weights of the backgammon win cond on
    // win node. Also the bias weight on the backgammon loss node equals the bias weight
    // on the backgammon win node, plus the sum of the backgammon win weights.
    
    for( int i=0; i<nMiddle; i++ )
    {
        v    = outputBackgammonWinWeights.at(i);
        val -= v * middles.at(i);
        sum += v;
    }
    val += outputBackgammonWinWeights.at(nMiddle) + sum; // no weighting by middle node value for the final weight
    return  1. / ( 1 + exp( -val ) );
}

vector<double> strategytdexp3::getOutputProbWeights() const { return outputProbWeights; }
vector<double> strategytdexp3::getOutputGammonWinWeights() const { return outputGammonWinWeights; }
vector<double> strategytdexp3::getOutputBackgammonWinWeights() const { return outputBackgammonWinWeights; }
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
    double oldGammonWinOutput = getOutputGammonWinValue( oldMiddles );
    double oldBgWinOutput=0;
    if( useBg )
        oldBgWinOutput = getOutputBackgammonWinValue( oldMiddles );
    
    // update the eligibility traces
    
    int i, j;
    double input, mirrorInput, finalOutProbWeight=0, outProbWeight, outGammonWeight, outBgWeight, midVal;
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
        if( useBg )
        {
            bgWinDerivs.at(i) = midVal * oldBgWinOutput * ( 1 - oldBgWinOutput );
            outBgWeight = outputBackgammonWinWeights.at(i);
        }
        
        for( j=0; j<99; j++ )
        {
            input = oldInputs.at(j);
            if( j<96 )
                mirrorInput = oldInputs.at((47-j/4)*4 + j%4 + 3);
            else
                mirrorInput = oldInputs.at(j+99); // for the # hit, the # born in, and whose turn it is
            probInputDerivs.at(i).at(j)   = outProbWeight   * ( input - mirrorInput ) * oldProbOutput      * ( 1 - oldProbOutput      ) * midVal * ( 1 - midVal );
            gamWinInputDerivs.at(i).at(j) = outGammonWeight * ( input - mirrorInput ) * oldGammonWinOutput * ( 1 - oldGammonWinOutput ) * midVal * ( 1 - midVal );
            if( useBg ) bgWinInputDerivs.at(i).at(j)  = outBgWeight * ( input - mirrorInput ) * oldBgWinOutput * ( 1 - oldBgWinOutput ) * midVal * ( 1 - midVal );
        }
    }
    
    // calculate the derivative to the final weight of the gammon node, which is the bias weight - ditto for backgammon node
    
    gamWinDerivs.at(nMiddle) = oldGammonWinOutput * ( 1 - oldGammonWinOutput );
    if( useBg ) bgWinDerivs.at(nMiddle)  = oldBgWinOutput * ( 1 - oldBgWinOutput );
    
    // update the parameter values based on the new output (or the result if the game is done)
    
    double newProbOutput, newGammonWinOutput, newBgWinOutput;
    bool trainGammonWin = true, trainBgWin = true;
    bool gameOver = true;
    
    if( !useBg ) trainBgWin = false;
    
    if( stopGammonTraining && oldBoard.otherBornIn() > 0 )
    {
        // we effectively stop using the gammon/bg prob nodes at this point, so stop training them
        
        trainGammonWin = false;
        trainBgWin     = false;
    }
    
    // if we are past the point where gammon prob is zero, don't keep training it or we'll push the gammon prob to too low a value
    
    if( newBoard.bornIn() == 15 )
    {
        newProbOutput = 1.; // this player definitely won
        if( newBoard.otherBornIn() == 0 )
        {
            newGammonWinOutput = 1.; // gammon or backgammon
            
            // check if it's a backgammon win
            
            if( useBg )
            {
                bool foundOne = false;
                for( i=0; i<6; i++ )
                {
                    if( newBoard.otherChecker(i) > 0 )
                    {
                        foundOne = true;
                        break;
                    }
                }
                
                if( foundOne || newBoard.otherHit() > 0 )
                    newBgWinOutput = 1;
                else
                    newBgWinOutput = 0;
            }
        }
        else
        {
            newGammonWinOutput = 0.;
            trainBgWin = false;
        }
    }
    else if( newBoard.otherBornIn() == 15 )
    {
        newProbOutput  = 0.;
        trainGammonWin = false;
        trainBgWin     = false;
    }
    else
    {
        gameOver = false;
        vector<double> midVals( getMiddleValues( getInputValues( newBoard, !holdDice ) ) ); // at the end of the turn, the holder of the dice flips
        newProbOutput = getOutputProbValue( midVals );
        if( trainGammonWin ) newGammonWinOutput = getOutputGammonWinValue( midVals );
        if( trainBgWin ) newBgWinOutput = getOutputBackgammonWinValue( midVals );
        
        // handle the known cases about when gammons don't happen. This is effectively like the training at the
        // end of the game for gammons (in the case when the gammon prob does go to zero before the end, anyways).
        // We stop training gammon weights after this point in the game.
        
        if( trainGammonWin && exactGammon && newBoard.otherBornIn() > 0 ) 
        {
            newGammonWinOutput = 0;
            trainBgWin = false;
        }
    }
    
    double gamAlpha, gamBeta, bgAlpha, bgBeta;
    
    if( weightLearning )
    {
        gamAlpha = alpha * newProbOutput;
        gamBeta  = beta  * newProbOutput;
        bgAlpha  = gamAlpha * newGammonWinOutput;
        bgBeta   = gamBeta  * newGammonWinOutput;
    }
    else
    {
        gamAlpha = alpha;
        bgAlpha  = alpha;
        gamBeta  = beta;
        bgBeta   = beta;
    }
    
    // update the weights
    
    for( i=0; i<nMiddle; i++ )
    {
        if( i < nMiddle - 1 )
            outputProbWeights.at(i) += alpha * ( newProbOutput - oldProbOutput ) * probDerivs.at(i);
        
        if( trainGammonWin )
            outputGammonWinWeights.at(i) += gamAlpha * ( newGammonWinOutput - oldGammonWinOutput ) * gamWinDerivs.at(i);
        
        if( trainBgWin )
            outputBackgammonWinWeights.at(i) += bgAlpha * ( newBgWinOutput - oldBgWinOutput ) * bgWinDerivs.at(i);
        
        for( j=0; j<99; j++ )
        {
            // the last input is the "whose move is it" input - weights to this default to zero. Leave them thus if
            // we're in symmetric mode.
            
            if( j == 98 && symmetric ) continue;
            
            middleWeights.at(i).at(j) += beta * ( newProbOutput - oldProbOutput ) * probInputDerivs.at(i).at(j);
            if( trainGammonWin )
                middleWeights.at(i).at(j) += gamBeta * ( newGammonWinOutput - oldGammonWinOutput ) * gamWinInputDerivs.at(i).at(j);
            if( trainBgWin )
                middleWeights.at(i).at(j) += bgBeta * ( newBgWinOutput - oldBgWinOutput ) * bgWinInputDerivs.at(i).at(j);
        }
    }
    
    // update the final gammon node weight - this is the bias weight.
    
    if( trainGammonWin )
        outputGammonWinWeights.at(nMiddle) += gamAlpha * ( newGammonWinOutput - oldGammonWinOutput ) * gamWinDerivs.at(nMiddle);
    if( trainBgWin )
        outputBackgammonWinWeights.at(nMiddle) += bgAlpha * ( newBgWinOutput - oldBgWinOutput ) * bgWinDerivs.at(nMiddle);
    
    // the first time we go through we train from the perspective of the player; also train from the perspective of
    // the other player to be symmetric. Though not really sure why this is required. If I don't do this, then the
    // prob of win is fine but the conditional prob of gammon goes to 100%.
    
    if( trainFlipped && holdDice )
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

