//
//  strategytdexp2.cpp
//  bgtd
//
//  Created by Mark Higgins on 8/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <cmath>
#include <fstream>
#include "strategytdexp2.h"

strategytdexp2::strategytdexp2( const strategytdexp& baseStrat )
{
    setupExp( baseStrat );
}

strategytdexp2::strategytdexp2( const string& pathEnd, const string& fileSuffix, bool loadExp )
{
    if( loadExp )
    {
        strategytdexp s( pathEnd, fileSuffix );
        setupExp( s );
    }
    else
    {
        string path = "/Users/mghiggins/bgtdres";
        if( pathEnd != "" ) path += "/" + pathEnd;
        string fopName  = path + "/weightsOutProb_" + fileSuffix + ".txt";
        string fowName  = path + "/weightsOutGammonWin_" + fileSuffix + ".txt";
        string folName  = path + "/weightsOutGammonLoss_" + fileSuffix + ".txt";
        string fmName   = path + "/weightsMiddle_" + fileSuffix + ".txt";
        
        ifstream fop( fopName.c_str() );
        ifstream fow( fowName.c_str() );
        ifstream fol( folName.c_str() );
        ifstream fm( fmName.c_str() );
        
        // start by loading the number of middle weights from the prob output weights file
        
        string line;
        getline( fop, line );
        nMiddle = atoi( line.c_str() );
        
        // resize all the vectors appropriately
        
        outputProbWeights.resize( nMiddle + 1 );
        outputGammonWinWeights.resize( nMiddle + 1 );
        outputGammonLossWeights.resize( nMiddle + 1 );
        middleWeights.resize( nMiddle );
        
        int i, j;
        
        for( i=0; i<nMiddle; i++ )
            middleWeights[i].resize(197);
        
        // load the weights for the three output nodes
        
        for( i=0; i<nMiddle+1; i++ )
        {
            getline( fop, line );
            outputProbWeights[i] = atof( line.c_str() );
            getline( fow, line );
            outputGammonWinWeights[i] = atof( line.c_str() );
            getline( fol, line );
            outputGammonLossWeights[i] = atof( line.c_str() );
        }
        
        // load the middle weights
        
        for( i=0; i<nMiddle; i++ )
            for( j=0; j<197; j++ )
            {
                getline( fm, line );
                middleWeights[i][j] = atof( line.c_str() );
            }
    }
}

void strategytdexp2::setupExp( const strategytdexp& baseStrat )
{
    // make the number of middle nodes the same
    
    nMiddle = baseStrat.nMiddle;
    
    // define lambda = 0 always, but grab alpha and beta from the base strategy
    
    alpha  = baseStrat.alpha;
    beta   = baseStrat.beta;
    lambda = 0;
    
    // the tdexp strategy has a reduced set of weights, which it gets away with because
    // it assumes a symmetry on flipping the board perspective (prob of win->prob of loss,
    // prob of gammon win->prob of gammon loss, and so on). That symmetry is not exact,
    // because the network returns you the expected number of points one *given that the
    // player is the one with the dice*. If you flip the board perspective and pass it in
    // again, you're not calculating the expected points for the other player given the
    // first one still holds the dice; you're also assuming the dice change hands. So in
    // this strategy, we relax that assumption to get something more accurate.
    
    int i, j;
    
    // start with the prob-of-win weights. That has nMiddle-1 elements in the tdexp case
    // because the symmetry requires that sum of weights == 0. We put in -sum of weights
    // for the final weight here, so that we start basically the same as the original.
    // Also add a bias weight (initialized to zero).
    
    outputProbWeights = baseStrat.getOutputProbWeights();
    double sum=0;
    for( i=0; i<nMiddle-1; i++ ) sum += outputProbWeights.at(i);
    outputProbWeights.insert( outputProbWeights.end(), -sum );
    outputProbWeights.insert( outputProbWeights.end(), 0 );
    
    // next do the conditional-prob-of-gammon-win node. There is no constraint from symmetry
    // on its values, so just grab them. Note there are nMiddle+1 elements: nMiddle weights
    // for each middle node value, plus a final bias node.
    
    outputGammonWinWeights = baseStrat.getOutputGammonWeights();
    
    // next do the conditional-prob-of-gammon-loss node. This node's values are entirely
    // constrained in the tdexp case: the weights are -1*gammon win weights, and the bias
    // node weigh is the gammon win bias node value plus the sum of the other weights.
    
    outputGammonLossWeights.resize( nMiddle + 1 );
    sum = 0;
    double v;
    for( i=0; i<nMiddle; i++ )
    {
        v = outputGammonWinWeights.at(i);
        outputGammonLossWeights.at(i) = -v;
        sum += v;
    }
    outputGammonLossWeights.at(nMiddle) = outputGammonWinWeights.at(nMiddle) + sum;
    
    // then do the middle weights. In the tdexp case there are only 98, not 196 (the
    // number of inputs), for each middle node, because the symmetry constrains the
    // second half of weights to be -1*the first half weights. We also add a final
    // input in this version which is 1 if the player holds the dice, and 0 if not;
    // so we need to add an extra weight to this input for each middle node. We
    // initialize those weights to zero.
    
    middleWeights.resize( nMiddle );
    vector< vector<double> > expMids( baseStrat.getMiddleWeights() );
    double w;
    for( i=0; i<nMiddle; i++ )
    {
        middleWeights.at(i).resize(197);
        for( j=0; j<96; j++ )
        {
            w = expMids.at(i).at(j);
            middleWeights.at(i).at(j) = w;
            middleWeights.at(i).at((47-j/4)*4+j%4+2) = -w;
        }
        middleWeights.at(i).at(96)  =  expMids.at(i).at(96);
        middleWeights.at(i).at(194) = -expMids.at(i).at(96);
        middleWeights.at(i).at(97)  =  expMids.at(i).at(97);
        middleWeights.at(i).at(195) = -expMids.at(i).at(97);
        
        middleWeights.at(i).at(196) = 0; // initial weight for the "whose turn is it" weight = 0
    }
    
    // allocate memory for the partial deriv vectors
    
    probDerivs.resize( nMiddle + 1 );
    gamWinDerivs.resize( nMiddle + 1 );
    gamLossDerivs.resize( nMiddle + 1 );
    probInputDerivs.resize( nMiddle );
    gamWinInputDerivs.resize( nMiddle );
    gamLossInputDerivs.resize( nMiddle );
    
    for( i=0; i<nMiddle; i++ )
    {
        probInputDerivs.at(i).resize(197);
        gamWinInputDerivs.at(i).resize(197);
        gamLossInputDerivs.at(i).resize(197);
    }
}

double strategytdexp2::boardValue( const board& brd ) const
{
    // get the inputs from the board, assuming the player holds the dice
    
    vector<double> inputs = getInputValues( brd, true );
    
    // calculate the middle layer node values
    
    vector<double> middles = getMiddleValues( inputs );
    
    // calculate the output node values from the middles
    
    double probWin            = getOutputProbValue( middles );
    double probCondGammonWin  = getOutputGammonWinValue( middles, brd );
    double probCondGammonLoss = getOutputGammonLossValue( middles, brd );
    
    // calculate the expected number of points the player will win. probWin
    // corresponds to the probability of a win (any win); probCondGammon
    // corresponds to the probability of a gammon conditional on a win;
    // probCondGammonLoss corresponds to the probability of a gammon loss
    // conditional on a loss.
    
    return probWin         * ( 1 * ( 1 - probCondGammonWin )  + 2 * probCondGammonWin  )
         - ( 1 - probWin ) * ( 1 * ( 1 - probCondGammonLoss ) + 2 * probCondGammonLoss );
}

vector<double> strategytdexp2::getInputValues( const board& brd, bool holdDice ) const
{
    vector<double> inputs;
    inputs.resize(197,0);
    vector<int> checks = brd.checkers();
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
    
    // last input is 1 if we hold the dice, 0 if not
    
    if( holdDice )
        inputs.at(196) = 1;
    else
        inputs.at(196) = 0;
    
    return inputs;
}

vector<double> strategytdexp2::getMiddleValues( const vector<double>& inputs ) const
{
    vector<double> mids;
    mids.resize(nMiddle);
    
    double val;
    int    i, j;
    
    for( i=0; i<nMiddle; i++ )
    {
        val = 0;
        for( j=0; j<197; j++ )
            val += middleWeights.at(i).at(j) * inputs.at(j);
        mids.at(i) = 1. / ( 1 + exp( -val ) );
    }
    
    return mids;
}

double strategytdexp2::getOutputProbValue( const vector<double>& middles ) const
{
    double val=0;
    for( int i=0; i<nMiddle; i++ )
        val += outputProbWeights.at(i) * middles.at(i);
    val += outputProbWeights.at(nMiddle); // bias node
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexp2::getOutputGammonWinValue( const vector<double>& middles, const board& brd ) const
{
    // special case - if the other player has taken any pieces in, the gammon win prob is zero
    
    if( brd.otherBornIn() > 0 ) return 0;
    
    // otherwise calculate the network value
    
    double val=0;
    for( int i=0; i<nMiddle; i++ )
        val += outputGammonWinWeights.at(i) * middles.at(i);
    val += outputGammonWinWeights.at(nMiddle); // bias node
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexp2::getOutputGammonLossValue( const vector<double>& middles, const board& brd ) const
{
    // special case - if the player has taken any pieces in, the gammon loss prob is zero
    
    if( brd.bornIn() > 0 ) return 0;
    
    // otherwise calculate the network value
    
    double val=0;
    for( int i=0; i<nMiddle; i++ )
        val += outputGammonLossWeights.at(i) * middles.at(i);
    val += outputGammonLossWeights.at(nMiddle); // bias node
    return  1. / ( 1 + exp( -val ) );
}

vector<double> strategytdexp2::getOutputProbWeights() const { return outputProbWeights; }
vector<double> strategytdexp2::getOutputGammonWinWeights() const { return outputGammonWinWeights; }
vector<double> strategytdexp2::getOutputGammonLossWeights() const { return outputGammonLossWeights; }
vector< vector<double> > strategytdexp2::getMiddleWeights() const { return middleWeights; }

bool strategytdexp2::needsUpdate() const
{
    return learning;
}

void strategytdexp2::update( const board& oldBoard, const board& newBoard )
{
    updateLocal( oldBoard, newBoard, true );
}

void strategytdexp2::updateLocal( const board& oldBoard, const board& newBoard, bool holdDice )
{
    // get the values from the old board
    
    vector<double> oldInputs   = getInputValues( oldBoard, holdDice );
    vector<double> oldMiddles  = getMiddleValues( oldInputs );
    double oldProbOutput       = getOutputProbValue( oldMiddles );
    double oldGammonWinOutput  = getOutputGammonWinValue( oldMiddles, oldBoard );
    double oldGammonLossOutput = getOutputGammonLossValue( oldMiddles, oldBoard );
    
    // calculate all the partial derivatives we'll need (of output node values
    // to the various weights)
    
    int i, j;
    
    // then do derivs of the prob nodes to each of the middle->input weights (that's a 2d array), and the derivs of each of
    // the middle nodes to its weights->inputs.
    
    double mid, input, v1, v2, v3;
    for( i=0; i<nMiddle; i++ )
    {
        mid = oldMiddles.at(i);
        v1  = outputProbWeights.at(i);
        v2  = outputGammonWinWeights.at(i);
        v3  = outputGammonLossWeights.at(i);
        
        probDerivs.at(i)    = mid * oldProbOutput       * ( 1 - oldProbOutput );
        gamWinDerivs.at(i)  = mid * oldGammonWinOutput  * ( 1 - oldGammonWinOutput );
        gamLossDerivs.at(i) = mid * oldGammonLossOutput * ( 1 - oldGammonLossOutput );
        
        for( j=0; j<197; j++ )
        {
            input = oldInputs.at(j);
            probInputDerivs.at(i).at(j)    = v1 * input * oldProbOutput       * ( 1 - oldProbOutput       ) * mid * ( 1 - mid );
            gamWinInputDerivs.at(i).at(j)  = v2 * input * oldGammonWinOutput  * ( 1 - oldGammonWinOutput  ) * mid * ( 1 - mid );
            gamLossInputDerivs.at(i).at(j) = v3 * input * oldGammonLossOutput * ( 1 - oldGammonLossOutput ) * mid * ( 1 - mid );
        }
    }
    probDerivs.at(nMiddle)    = oldProbOutput       * ( 1 - oldProbOutput );
    gamWinDerivs.at(nMiddle)  = oldGammonWinOutput  * ( 1 - oldGammonWinOutput );
    gamLossDerivs.at(nMiddle) = oldGammonLossOutput * ( 1 - oldGammonLossOutput );
    
    // now calculate the next estimate of the outputs. That's known if the game is over; otherwise we use the network's 
    // estimate on the new board as a proxy. Note that the update fn is only ever called by the game when the player wins, not when
    // the player loses, just because the winner is the last one to play. But we need to train on prob of losing a gammon too,
    // so we flip board perspective and train again based on that.
    
    bool gameOver        = false;
    bool trainGammonLoss = true;
    bool trainGammonWin  = true;
    double newProbOutput, newGammonWinOutput, newGammonLossOutput;
    
    if( newBoard.bornIn() == 15 )
    {
        gameOver = true;
        trainGammonLoss = false; // can't train the conditional prob of a gammon loss if there isn't a loss
        
        newProbOutput = 1.;
        if( newBoard.otherBornIn() == 0 ) // gammon or backgammon
            newGammonWinOutput = 1.;
        else
            newGammonWinOutput = 0.;
    }
    else if( newBoard.otherBornIn() == 15 )
    {
        gameOver = true;
        trainGammonWin = false;
        
        newProbOutput = 0.;
        
        if( newBoard.bornIn() == 0 ) // gammon loss or backgammon loss
            newGammonLossOutput = 1;
        else
            newGammonLossOutput = 0;
    }
    else
    {
        // estimate from the new board's outputs
        
        vector<double> midVals( getMiddleValues( getInputValues( newBoard, holdDice ) ) );
        newProbOutput       = getOutputProbValue( midVals );
        newGammonWinOutput  = getOutputGammonWinValue( midVals, newBoard );
        newGammonLossOutput = getOutputGammonLossValue( midVals, newBoard );
    }
    
    // train the nodes as appropriate
    
    for( i=0; i<nMiddle; i++ )
    {
        outputProbWeights.at(i) += alpha * ( newProbOutput - oldProbOutput ) * probDerivs.at(i);
        
        if( trainGammonWin )
            outputGammonWinWeights.at(i) += alpha * ( newGammonWinOutput - oldGammonWinOutput ) * gamWinDerivs.at(i);
        
        if( trainGammonLoss )
            outputGammonLossWeights.at(i) += alpha * ( newGammonLossOutput - oldGammonLossOutput ) * gamLossDerivs.at(i);
        
        for( j=0; j<196; j++ )
        {
            middleWeights.at(i).at(j) += beta * ( newProbOutput - oldProbOutput ) * probInputDerivs.at(i).at(j);
            if( trainGammonWin )
                middleWeights.at(i).at(j) += beta * ( newGammonWinOutput - oldGammonWinOutput ) * gamWinInputDerivs.at(i).at(j);
            if( trainGammonLoss )
                middleWeights.at(i).at(j) += beta * ( newGammonLossOutput - oldGammonLossOutput ) * gamLossInputDerivs.at(i).at(j);
        }
    }
    
    outputProbWeights.at(nMiddle) += alpha * ( newProbOutput - oldProbOutput ) * probDerivs.at(nMiddle);
    if( trainGammonWin )
        outputGammonWinWeights.at(nMiddle) += alpha * ( newGammonWinOutput - oldGammonWinOutput ) * gamWinDerivs.at(nMiddle);
    if( trainGammonLoss )
        outputGammonLossWeights.at(nMiddle) += alpha * ( newGammonLossOutput - oldGammonLossOutput ) * gamLossDerivs.at(nMiddle);
    
    // the first time we go through we train from the perspective of the player; also train from the perspective of
    // the other player, since we have to make sure that we don't always train on ending prob of win==1 (or the network
    // will converge to large +ve weights and always return 1 for prob of win).
    
    if( holdDice )
    {
        // construct copies of the old and new boards, but with perspective flipped.
        
        board oldBoardFlipped( oldBoard );
        oldBoardFlipped.setPerspective( ( oldBoardFlipped.perspective() + 1 ) % 2 );
        board newBoardFlipped( newBoard );
        newBoardFlipped.setPerspective( ( newBoardFlipped.perspective() + 1 ) % 2 );
        
        // update the weights again using this perspective, without holding the dice
        
        updateLocal( oldBoardFlipped, newBoardFlipped, false );
    }
}
