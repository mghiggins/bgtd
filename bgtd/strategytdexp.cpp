//
//  strategytdexp.cpp
//  bgtd
//
//  Created by Mark Higgins on 7/31/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <string>
#include <cmath>
#include <fstream>
#include "strategytdexp.h"
#include "randomc.h"

strategytdexp::strategytdexp()
{
    nMiddle = 40;
    setup();
}

strategytdexp::strategytdexp( int nMiddle )
{
    if( nMiddle % 2 != 0 ) throw string( "nMiddle must be even" );
    this->nMiddle = nMiddle;
    setup();
}

strategytdexp::strategytdexp( const vector<double>& outputProbWeights, vector<double>& outputGammonWeights, const vector< vector<double> >& middleWeights, 
                             const vector<double>& outputProbTraces, const vector<double>& outputGammonTraces, 
                             const vector< vector<double> >& middleProbTraces, const vector< vector<double> >& middleGammonTraces,
                             double alpha, double beta, double lambda )
{
    nMiddle = (int) middleWeights.size();
    if( nMiddle % 2 != 0 ) throw string( "nMiddle must be even" );
    if( outputProbWeights.size() != nMiddle-1 ) throw string( "outputProbWeights must have nMiddle-1 elements" );
    if( outputGammonWeights.size() != nMiddle+1 ) throw string( "outputGammonWeights must have nMiddle+1 elements" );
    if( outputProbTraces.size() != nMiddle-1 ) throw string( "outputProbTraces must have nMiddle-1 elements" );
    if( outputGammonTraces.size() != nMiddle+1 ) throw string( "outputGammonTraces must have nMiddle+1 elements" );
    if( middleWeights.size() != nMiddle ) throw string( "middleWeights must have nMiddle elements" );
    if( middleProbTraces.size() != nMiddle ) throw string( "middleProbTraces must have nMiddle elements" );
    if( middleGammonTraces.size() != nMiddle ) throw string( "middleGammonTraces must have nMiddle elements" );
    for( int i=0; i<nMiddle; i++ )
    {
        if( middleWeights.at(i).size() != 98 ) throw string( "middleWeights elements must all have 98 elements" );
        if( middleProbTraces.at(i).size() != 98 ) throw string( "middleProbTraces elements must all have 98 elements" );
        if( middleGammonTraces.at(i).size() != 98 ) throw string( "middleGammonTraces elements must all have 98 elements" );
    }
    
    this->outputProbWeights   = outputProbWeights;
    this->outputGammonWeights = outputGammonWeights;
    this->middleWeights       = middleWeights;
    this->outputProbTraces    = outputProbTraces;
    this->outputGammonTraces  = outputGammonTraces;
    this->middleProbTraces    = middleProbTraces;
    this->middleGammonTraces  = middleGammonTraces;
    this->alpha               = alpha;
    this->beta                = beta;
    this->lambda              = lambda;
}

strategytdexp::strategytdexp( const strategytdexp& otherStrat )
{
    nMiddle             = otherStrat.nMiddle;
    outputProbWeights   = otherStrat.outputProbWeights;
    outputGammonWeights = otherStrat.outputGammonWeights;
    middleWeights       = otherStrat.middleWeights;
    outputProbTraces    = otherStrat.outputProbTraces;
    outputGammonTraces  = otherStrat.outputGammonTraces;
    middleProbTraces    = otherStrat.middleProbTraces;
    middleGammonTraces  = otherStrat.middleGammonTraces;
    alpha               = otherStrat.alpha;
    beta                = otherStrat.beta;
    lambda              = otherStrat.lambda;
    learning            = otherStrat.learning;
}

strategytdexp::strategytdexp( const string& pathEnd, const string& fileSuffix )
{
    string path     = "/Users/mghiggins/bgtdres";
    if( pathEnd != "" ) path += "/" + pathEnd;
    string fopName  = path + "/weightsOutProb_" + fileSuffix + ".txt";
    string fogName  = path + "/weightsOutGammon_" + fileSuffix + ".txt";
    string foptName = path + "/tracesOutProb_" + fileSuffix + ".txt";
    string fogtName = path + "/tracesOutGammon_" + fileSuffix + ".txt";
    string fmName   = path + "/weightsMiddle_" + fileSuffix + ".txt";
    string fmptName = path + "/tracesMiddleProb_" + fileSuffix + ".txt";
    string fmgtName = path + "/tracesMiddleGammon_" + fileSuffix + ".txt";
    
    ifstream fop( fopName.c_str() );
    ifstream fog( fogName.c_str() );
    ifstream fopt( foptName.c_str() );
    ifstream fogt( fogtName.c_str() );
    ifstream fm( fmName.c_str() );
    ifstream fmpt( fmptName.c_str() );
    ifstream fmgt( fmgtName.c_str() );
    
    // start by loading the number of middle weights from the prob output weights file
    
    string line;
    getline( fop, line );
    nMiddle = atoi( line.c_str() );
    
    // resize all the vectors appropriately
    
    outputProbWeights.resize( nMiddle - 1 );
    outputGammonWeights.resize( nMiddle + 1 );
    outputProbTraces.resize( nMiddle - 1 );
    outputGammonTraces.resize( nMiddle + 1 );
    middleWeights.resize( nMiddle );
    middleProbTraces.resize( nMiddle );
    middleGammonTraces.resize( nMiddle );
    
    int i, j;
    
    for( i=0; i<nMiddle; i++ )
    {
        middleWeights[i].resize(98);
        middleProbTraces[i].resize(98);
        middleGammonTraces[i].resize(98);
    }
    
    // load the output weights and traces for the prob and gammon nodes
    
    for( i=0; i<nMiddle-1; i++ )
    {
        getline( fop, line );
        outputProbWeights[i] = atof( line.c_str() );
        getline( fopt, line );
        outputProbTraces[i] = atof( line.c_str() );
    }
    
    for( i=0; i<nMiddle+1; i++ )
    {
        getline( fog, line );
        outputGammonWeights[i] = atof( line.c_str() );
        getline( fogt, line );
        outputGammonTraces[i] = atof( line.c_str() );
    }
    
    // load the middle weights and traces
    
    for( i=0; i<nMiddle; i++ )
        for( j=0; j<98; j++ )
        {
            getline( fm, line );
            middleWeights[i][j] = atof( line.c_str() );
            getline( fmpt, line );
            middleProbTraces[i][j] = atof( line.c_str() );
            getline( fmgt, line );
            middleGammonTraces[i][j] = atof( line.c_str() );
        }
}

strategytdexp::~strategytdexp()
{
}

void strategytdexp::setup()
{
    int i, j;
    
    // size the weights and eligibility trace vectors appropriately
    
    outputProbWeights.resize( nMiddle-1 );
    outputGammonWeights.resize( nMiddle+1 );
    outputProbTraces.resize( nMiddle-1, 0 );
    outputGammonTraces.resize( nMiddle+1, 0 );
    middleWeights.resize( nMiddle );
    middleProbTraces.resize( nMiddle );
    middleGammonTraces.resize( nMiddle );
    
    for( i=0; i<nMiddle; i++ )
    {
        middleWeights.at(i).resize(98);
        middleProbTraces.at(i).resize(98,0);
        middleGammonTraces.at(i).resize(98,0);
    }
    
    // assign random weights
    
    CRandomMersenne rng(1);
    for( i=0; i<nMiddle+1; i++ )
    {
        if( i<nMiddle-1 ) outputProbWeights.at(i) = rng.IRandom(-100,100)/1000.;
        outputGammonWeights.at(i) = rng.IRandom(-100,100)/1000.;
    }
    for( i=0; i<nMiddle; i++ )
        for( j=0; j<98; j++ )
            middleWeights.at(i).at(j) = rng.IRandom(-100,100)/1000.;
    
    // put in default values for alpha and lambda (the learning process parameters)
    
    alpha  = 0.1;
    beta   = 0.1;
    lambda = 0.;
}

vector<double> strategytdexp::getInputValues( const board& brd ) const
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

vector<double> strategytdexp::getMiddleValues( const vector<double>& inputs ) const
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

double strategytdexp::getOutputProbValue( const vector<double>& middles ) const
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
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexp::getOutputGammonValue( const vector<double>& middles ) const
{
    double val=0;
    
    for( int i=0; i<nMiddle; i++ )
        val += outputGammonWeights.at(i) * middles.at(i);
    val += outputGammonWeights.at(nMiddle); // no weighting by middle node value for the final weight
    return  1. / ( 1 + exp( -val ) );
}

double strategytdexp::getOutputGammonLossValue( const vector<double>& middles ) const
{
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

double strategytdexp::boardValue( const board& brd ) const
{
    // get the inputs from the board
    
    vector<double> inputs = getInputValues( brd );
    
    // calculate the middle layer node values
    
    vector<double> middles = getMiddleValues( inputs );
    
    // calculate the output node values from the middles
    
    double probWin            = getOutputProbValue( middles );
    double probCondGammon     = getOutputGammonValue( middles );
    double probCondGammonLoss = getOutputGammonLossValue( middles );
    
    // calculate the expected number of points the player will win. probWin
    // corresponds to the probability of a win (any win); probCondGammon
    // corresponds to the probability of a gammon conditional on a win;
    // probCondGammonLoss corresponds to the probability of a gammon loss
    // conditional on a loss.
    
    return probWin         * ( 1 * ( 1 - probCondGammon )     + 2 * probCondGammon )
         - ( 1 - probWin ) * ( 1 * ( 1 - probCondGammonLoss ) + 2 * probCondGammonLoss );
}

vector<double> strategytdexp::getOutputProbWeights() const { return outputProbWeights; }
vector<double> strategytdexp::getOutputGammonWeights() const { return outputGammonWeights; }
vector< vector<double> > strategytdexp::getMiddleWeights() const { return middleWeights; }
vector<double> strategytdexp::getOutputProbTraces() const { return outputProbTraces; }
vector<double> strategytdexp::getOutputGammonTraces() const { return outputGammonTraces; }
vector< vector<double> > strategytdexp::getMiddleProbTraces() const { return middleProbTraces; }
vector< vector<double> > strategytdexp::getMiddleGammonTraces() const { return middleGammonTraces; }

bool strategytdexp::needsUpdate() const
{
    return learning;
}

void strategytdexp::update( const board& oldBoard, const board& newBoard )
{
    // get the values from the old board
    
    vector<double> oldInputs  = getInputValues( oldBoard );
    vector<double> oldMiddles = getMiddleValues( oldInputs );
    double oldProbOutput      = getOutputProbValue( oldMiddles );
    double oldGammonOutput    = getOutputGammonValue( oldMiddles );
    
    // update the eligibility traces
    
    int i, j;
    double input, mirrorInput, finalOutProbWeight=0, outProbWeight, outGammonWeight, midVal;
    
    for( i=0; i<nMiddle; i++ )
    {
        midVal = oldMiddles.at(i);
        
        if( i < nMiddle - 1 )
        {
            outputProbTraces.at(i) = lambda * outputProbTraces.at(i) + ( midVal - oldMiddles.at(nMiddle-1) ) * oldProbOutput * ( 1 - oldProbOutput );
            outProbWeight       = outputProbWeights.at(i);
            finalOutProbWeight -= outProbWeight;
        }
        else
            // the prob of win output weights have nMiddle-1 values because the final nMiddle'th weight is constrained to equal the sum of all the
            // previous weights. This is so that we ensure symmetry of the probabilities on flipping the board perspective.
            
            outProbWeight = finalOutProbWeight;
        
        // the prob of gammon win conditional on win output node does not require the same constraint on sum of weights==0; in fact, it
        // has an extra weight called a "bias weight" which is multiplied by 1 instead of a middle node value. But there are nMiddle
        // weights that *are* weighted by middle node values and we need to track the traces for them.
        
        outputGammonTraces.at(i) = lambda * outputGammonTraces.at(i) + midVal * oldGammonOutput * ( 1 - oldGammonOutput );
        outGammonWeight = outputGammonWeights.at(i);
        
        for( j=0; j<98; j++ )
        {
            input = oldInputs.at(j);
            if( j<96 )
                mirrorInput = oldInputs.at((47-j/4)*4 + j%4 + 2);
            else
                mirrorInput = oldInputs.at(j+98); // for the # hit and the # born in
            middleProbTraces.at(i).at(j)   = lambda * middleProbTraces.at(i).at(j)   + outProbWeight   * ( input - mirrorInput ) * oldProbOutput   * ( 1 - oldProbOutput   ) * midVal * ( 1 - midVal );
            middleGammonTraces.at(i).at(j) = lambda * middleGammonTraces.at(i).at(j) + outGammonWeight * ( input - mirrorInput ) * oldGammonOutput * ( 1 - oldGammonOutput ) * midVal * ( 1 - midVal );
        }
    }
    
    // update the trace for the final weight of the gammon node, which is the bias weight
    
    outputGammonTraces.at(nMiddle) = lambda * outputGammonTraces.at(nMiddle) + oldGammonOutput * ( 1 - oldGammonOutput );
    
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
        // the nodes we track are the probability of winning and that of getting a gammon conditional on a win.
        // We don't track the probabilities of losing explicitly, because those can be derived from the
        // winning node parameters. This is thanks to the requirement that the probabilities reverse properly
        // when the board perspective flips.
        // Anyways, it means that on a win, both of the output nodes we track should have value zero.
        
        newProbOutput   = 0;
        newGammonOutput = 0;
    }
    else
    {
        vector<double> midVals( getMiddleValues( getInputValues( newBoard ) ) );
        newProbOutput   = getOutputProbValue(   midVals );
        newGammonOutput = getOutputGammonValue( midVals );
    }
    
    // update the weights
    
    for( i=0; i<nMiddle; i++ )
    {
        if( i < nMiddle - 1 )
            outputProbWeights.at(i) += alpha * ( newProbOutput - oldProbOutput   ) * outputProbTraces.at(i);
        
        outputGammonWeights.at(i) += alpha * ( newGammonOutput - oldGammonOutput ) * outputGammonTraces.at(i);
        
        for( j=0; j<98; j++ )
            middleWeights.at(i).at(j) += beta * ( ( newProbOutput   - oldProbOutput   ) * middleProbTraces.at(i).at(j)
                                                + ( newGammonOutput - oldGammonOutput ) * middleGammonTraces.at(i).at(j) );
    }
    
    // update the final gammon node weight - this is the bias weight.
    
    outputGammonWeights.at(nMiddle) += alpha * ( newGammonOutput - oldGammonOutput ) * outputGammonTraces.at(nMiddle);
}
