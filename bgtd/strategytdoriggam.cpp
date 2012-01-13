//
//  strategytdoriggam.cpp
//  bgtd
//
//  Created by Mark Higgins on 1/12/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#include <cmath>
#include <fstream>
#include <string>
#include "randomc.h"
#include "strategytdoriggam.h"

strategytdoriggam::strategytdoriggam()
{
    nMiddle = 40; // default # of middle nodes
    setup();
}

strategytdoriggam::strategytdoriggam( int nMiddle )
{
    this->nMiddle = nMiddle;
    setup();
}

strategytdoriggam::strategytdoriggam( const string& subPath, const string& filePrefix )
{
    loadWeights( subPath, filePrefix );
}

void strategytdoriggam::setup()
{
    // generates random weights
    
    CRandomMersenne rng(1);
    
    outputWinWeights.resize(nMiddle+1); // one more than nMiddle - the last is the bias weight
    outputGammonWeights.resize(nMiddle+1);
    outputGammonLossWeights.resize(nMiddle+1);
    middleWeights.resize(nMiddle);    
    
    int i, j;
    for( i=0; i<nMiddle; i++ )
    {
        outputWinWeights.at(i) = rng.IRandom(-100, 100)/1000.;
        outputGammonWeights.at(i) = rng.IRandom(-100, 100)/1000.;
        outputGammonLossWeights.at(i) = rng.IRandom(-100, 100)/1000.;
        middleWeights.at(i).resize(196);
        for( j=0; j<196; j++ )
            middleWeights.at(i).at(j) = rng.IRandom(-100, 100)/1000.;
    }
    outputWinWeights.at(nMiddle) = rng.IRandom(-100, 100)/1000.; // bias weight
    outputGammonWeights.at(nMiddle) = rng.IRandom(-100, 100)/1000.; // bias weight
    outputGammonLossWeights.at(nMiddle) = rng.IRandom(-100, 100)/1000.; // bias weight
    
    // set alpha and beta sensibly. Lambda we always set to zero.
    
    alpha  = 0.1;
    beta   = 0.1;
    lambda = 0;
}

double strategytdoriggam::boardValue( const board& brd ) const
{
    // need the value in the state where the opponent holds the dice, so
    // flip the board perspective around
    
    board flippedBoard( brd );
    flippedBoard.setPerspective( 1 - brd.perspective() );
    
    // get the prob of win from the perspective of the opponent
    
    vector<double> mids( getMiddleValues( getInputValues( flippedBoard ) ) );
    double pw  = getOutputWin( mids );
    double pg  = getOutputGammon( mids );
    double pgl = getOutputGammonLoss( mids );
    
    // get the equity from the perspective of the opponent
    
    double equity = 1 * ( pw - pg ) + 2 * pg - 1 * ( 1 - pw - pgl ) - 2 * pgl; 
    
    // return -1 * that to get the equity of the player
    
    return -equity;
}

vector<double> strategytdoriggam::getInputValues( const board& brd ) const
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

vector<double> strategytdoriggam::getMiddleValues( const vector<double>& inputs ) const
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

double strategytdoriggam::getOutputWin( const vector<double>& middles ) const
{
    double sum=0;
    for( int i=0; i<nMiddle; i++ )
        sum += outputWinWeights.at(i) * middles.at(i);
    sum += outputWinWeights.at(nMiddle); // output node does contain a bias weight
    return 1 / ( 1 + exp( -sum ) );
}

double strategytdoriggam::getOutputGammon( const vector<double>& middles ) const
{
    double sum=0;
    for( int i=0; i<nMiddle; i++ )
        sum += outputGammonWeights.at(i) * middles.at(i);
    sum += outputGammonWeights.at(nMiddle); // output node does contain a bias weight
    return 1 / ( 1 + exp( -sum ) );
}

double strategytdoriggam::getOutputGammonLoss( const vector<double>& middles ) const
{
    double sum=0;
    for( int i=0; i<nMiddle; i++ )
        sum += outputGammonLossWeights.at(i) * middles.at(i);
    sum += outputGammonLossWeights.at(nMiddle); // output node does contain a bias weight
    return 1 / ( 1 + exp( -sum ) );
}

bool strategytdoriggam::needsUpdate() const
{
    return learning;
}

void strategytdoriggam::update( const board& oldBoard, const board& newBoard )
{
    // get the old board's probability of win and input/middle layer values
    
    vector<double> oldInputs( getInputValues( oldBoard ) );
    vector<double> oldMids( getMiddleValues( oldInputs ) );
    double oldProbWin = getOutputWin( oldMids );
    double oldProbGam = getOutputGammon( oldMids );
    double oldProbGamLoss = getOutputGammonLoss( oldMids );
    
    // calculate the partial deriv of the output node value wrt weights.
    // Two sets of weights: the ones from output->middle, and the ones
    // from each middle->input.
    
    vector<double> dPWdMiddle(nMiddle+1), dPGdMiddle(nMiddle+1), dPGLdMiddle(nMiddle+1);
    vector< vector<double> > dPWdInput(nMiddle), dPGdInput(nMiddle), dPGLdInput(nMiddle);
    
    int i, j;
    double probWinProd = oldProbWin * ( 1 - oldProbWin ); // cache it to avoid recalcing it over and over
    double probGamProd = oldProbGam * ( 1 - oldProbGam );
    double probGamLossProd = oldProbGamLoss * ( 1 - oldProbGamLoss );
    double midProd, midVal, winWeight, gamWeight, gamLossWeight;
    
    for( i=0; i<nMiddle; i++ )
    {
        midVal = oldMids.at(i);
        dPWdMiddle.at(i) = probWinProd * midVal;
        dPGdMiddle.at(i) = probGamProd * midVal;
        dPGLdMiddle.at(i) = probGamLossProd * midVal;
        dPWdInput.at(i).resize(196);
        dPGdInput.at(i).resize(196);
        dPGLdInput.at(i).resize(196);
        midProd = midVal * ( 1 - midVal ); // to avoid 196 recalcs
        winWeight = outputWinWeights.at(i);
        gamWeight = outputGammonWeights.at(i);
        gamLossWeight = outputGammonLossWeights.at(i);
        for( j=0; j<196; j++ )
        {
            dPWdInput.at(i).at(j) = probWinProd * winWeight * midProd * oldInputs.at(j);
            dPGdInput.at(i).at(j) = probGamProd * gamWeight * midProd * oldInputs.at(j);
            dPGLdInput.at(i).at(j) = probGamLossProd * gamLossWeight * midProd * oldInputs.at(j);
        }
    }
    dPWdMiddle.at(nMiddle) = probWinProd; // bias weight deriv
    dPGdMiddle.at(nMiddle) = probGamProd; // bias weight deriv
    dPGLdMiddle.at(nMiddle) = probGamLossProd; // bias weight deriv

    // figure out the new network value
    
    double newProbWin, newProbGam, newProbGamLoss;
    
    if( newBoard.bornIn() == 15 )
    {
        newProbWin = 1;
        if( newBoard.otherBornIn() == 0 )
            newProbGam = 1;
        else
            newProbGam = 0;
        newProbGamLoss = 0;
    }
    else
    {
        // need to get the prob from the perspective of the other player when they have
        // the dice, so flip perspective and get the prob of the other player winning,
        // then do 1 - that. Simiarly, flip gammon win/loss.
        
        board flippedBoard( newBoard );
        flippedBoard.setPerspective( ( newBoard.perspective() + 1 ) % 2 );
        vector<double> newMids( getMiddleValues( getInputValues( flippedBoard ) ) );
        newProbWin = 1 - getOutputWin( newMids );
        newProbGam = getOutputGammonLoss( newMids );
        newProbGamLoss = getOutputGammon( newMids );
    }
    
    // train the weights
    
    double probWinDiff = newProbWin - oldProbWin;
    double probGamDiff = newProbGam - oldProbGam;
    double probGamLossDiff = newProbGamLoss - oldProbGamLoss;
    
    for( i=0; i<nMiddle; i++ )
    {
        outputWinWeights.at(i) += alpha * probWinDiff * dPWdMiddle.at(i);
        outputGammonWeights.at(i) += alpha * probGamDiff * dPGdMiddle.at(i);
        outputGammonLossWeights.at(i) += alpha * probGamLossDiff * dPGLdMiddle.at(i);
        for( j=0; j<196; j++ )
        {
            middleWeights.at(i).at(j) += beta * probWinDiff * dPWdInput.at(i).at(j);
            middleWeights.at(i).at(j) += beta * probGamDiff * dPGdInput.at(i).at(j);
            middleWeights.at(i).at(j) += beta * probGamLossDiff * dPGLdInput.at(i).at(j);
        }
    }
    outputWinWeights.at(nMiddle) += alpha * probWinDiff * dPWdMiddle.at(nMiddle); // bias weight
    outputGammonWeights.at(nMiddle) += alpha * probGamDiff * dPGdMiddle.at(nMiddle); // bias weight
    outputGammonLossWeights.at(nMiddle) += alpha * probGamLossDiff * dPGLdMiddle.at(nMiddle); // bias weight
}

void strategytdoriggam::writeWeights( const string& filePrefix ) const
{
    string path = "/Users/mghiggins/bgtdres";
    
    // define the file names for the weights files
    
    string fopName  = path + "/weightsOutProb_" + filePrefix + ".txt";
    string fogName  = path + "/weightsOutGammon_" + filePrefix + ".txt";
    string foglName = path + "/weightsOutGammonLoss_" + filePrefix + ".txt";
    string fmName   = path + "/weightsMiddle_" + filePrefix + ".txt";
    
    ofstream fop( fopName.c_str() );
    ofstream fog( fogName.c_str() );
    ofstream fogl( foglName.c_str() );
    ofstream fm( fmName.c_str() );
    
    fop << nMiddle << endl; // the file for the output of prob win weights contains the # of middle nodes
    
    for( int j=0; j<nMiddle; j++ )
    {
        fop << outputWinWeights.at(j) << endl;
        fog << outputGammonWeights.at(j) << endl;
        fogl << outputGammonLossWeights.at(j) << endl;
        for( int k=0; k<196; k++ )
            fm << middleWeights.at(j).at(k) << endl;
    }
    fop << outputWinWeights.at(nMiddle) << endl;
    fog << outputGammonWeights.at(nMiddle) << endl;
    fogl << outputGammonLossWeights.at(nMiddle) << endl;
    
    fop.close();
    fog.close();
    fogl.close();
    fm.close();
}

void strategytdoriggam::loadWeights( const string& subPath, const string& filePrefix )
{
    // load the # of middle nodes and the network names from the names file
    
    string path = "/Users/mghiggins/bgtdres";
    if( subPath != "" ) path += "/" + subPath;
    
    string fopName  = path + "/weightsOutProb_" + filePrefix + ".txt";
    string fogName  = path + "/weightsOutGammon_" + filePrefix + ".txt";
    string foglName = path + "/weightsOutGammonLoss_" + filePrefix + ".txt";
    string fmName   = path + "/weightsMiddle_" + filePrefix + ".txt";
    
    ifstream fop( fopName.c_str() );
    ifstream fog( fogName.c_str() );
    ifstream fogl( foglName.c_str() );
    ifstream fm( fmName.c_str() );
    
    // grab the # of middle nodes from the win output file
    
    string line;
    
    getline( fop, line );
    nMiddle = atoi( line.c_str() );
    
    // load the weights themselves; resize all the vectors properly
    
    outputWinWeights.resize(nMiddle+1);
    outputGammonWeights.resize(nMiddle+1);
    outputGammonLossWeights.resize(nMiddle+1);
    middleWeights.resize(nMiddle);
    
    for( int i=0; i<nMiddle; i++ )
    {
        getline( fop, line );
        outputWinWeights.at(i) = atof( line.c_str() );
        getline( fog, line );
        outputGammonWeights.at(i) = atof( line.c_str() );
        getline( fogl, line );
        outputGammonLossWeights.at(i) = atof( line.c_str() );
        middleWeights.at(i).resize(196);
        for( int j=0; j<196; j++ )
        {
            getline( fm, line );
            middleWeights.at(i).at(j) = atof( line.c_str() );
        }
    }
    getline( fop, line );
    outputWinWeights.at(nMiddle) = atof( line.c_str() );
    getline( fog, line );
    outputGammonWeights.at(nMiddle) = atof( line.c_str() );
    getline( fogl, line );
    outputGammonLossWeights.at(nMiddle) = atof( line.c_str() );
    
    fop.close();
    fog.close();
    fogl.close();
    fm.close();
}
