//
//  strategytdorigbg.cpp
//  bgtd
//
//  Created by Mark Higgins on 1/16/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include "strategytdorigbg.h"
#include "randomc.h"

strategytdorigbg::strategytdorigbg()
{
    nMiddle = 40; // default # of middle nodes
    setup();
}

strategytdorigbg::strategytdorigbg( int nMiddle )
{
    this->nMiddle = nMiddle;
    setup();
}

strategytdorigbg::strategytdorigbg( const string& subPath, const string& filePrefix )
{
    loadWeights( subPath, filePrefix );
}

void strategytdorigbg::setupBgWeights()
{
    CRandomMersenne rng(1);
    
    outputBgWeights.resize(nMiddle+1);
    outputBgLossWeights.resize(nMiddle+1);
    
    for( int i=0; i<nMiddle+1; i++ )
    {
        outputBgWeights.at(i) = rng.IRandom(-100,100)/1000.;
        outputBgLossWeights.at(i) = rng.IRandom(-100,100)/1000.;
    }
}

void strategytdorigbg::setup()
{
    strategytdoriggam::setup();
    setupBgWeights();
}

double strategytdorigbg::boardValue( const board& brd, const hash_map<string,int>* context ) const
{
    // need the value in the state where the opponent holds the dice, so
    // flip the board perspective around
    
    board flippedBoard( brd );
    flippedBoard.setPerspective( 1 - brd.perspective() );
    
    // get the prob of win from the perspective of the opponent. Do usual
    // sanity checks around probs.
    
    double pw, pg, pb, pgl, pbl;
    
    vector<double> mids( getMiddleValues( getInputValues( flippedBoard ) ) );
    pw  = getOutputWin( mids );
    if( flippedBoard.otherBornIn() == 0 )
        pg  = getOutputGammon( mids );
    else
        pg = 0;
    if( flippedBoard.bornIn() == 0 )
        pgl = getOutputGammonLoss( mids );
    else
        pgl = 0;
    if( flippedBoard.otherNoBackgammon() )
        pb  = 0;
    else
        pb  = getOutputBackgammon( mids );
    if( flippedBoard.noBackgammon() )
        pbl = 0;
    else
        pbl = getOutputBackgammonLoss( mids );
    
    if( pg > pw ) pg = pw;
    if( pb > pg ) pb = pg;
    if( pgl > 1 - pw ) pgl = 1 - pw;
    if( pbl > pgl ) pbl = pgl;
    
    // get the equity from the perspective of the opponent
    
    double equity = 1 * ( pw - pg ) + 2 * ( pg - pb ) + 3 * pb - 1 * ( 1 - pw - pgl ) - 2 * ( pgl - pbl ) - 3 * pbl; 
    
    // return -1 * that to get the equity of the player
    
    return -equity;
}

double strategytdorigbg::getOutputBackgammon( const vector<double>& middles ) const
{
    double sum=0;
    for( int i=0; i<nMiddle; i++ )
        sum += outputBgWeights.at(i) * middles.at(i);
    sum += outputBgWeights.at(nMiddle); // output node does contain a bias weight
    return 1 / ( 1 + exp( -sum ) );
}

double strategytdorigbg::getOutputBackgammonLoss( const vector<double>& middles ) const
{
    double sum=0;
    for( int i=0; i<nMiddle; i++ )
        sum += outputBgLossWeights.at(i) * middles.at(i);
    sum += outputBgLossWeights.at(nMiddle); // output node does contain a bias weight
    return 1 / ( 1 + exp( -sum ) );
}

void strategytdorigbg::writeWeights( const string& filePrefix ) const
{
    strategytdoriggam::writeWeights( filePrefix );
    
    string path = "/Users/mghiggins/bgtdres";
    
    // define the file names for the weights files
    
    string fogName  = path + "/weightsOutBg_" + filePrefix + ".txt";
    string foglName = path + "/weightsOutBgLoss_" + filePrefix + ".txt";
    
    // open the files for writing
    
    ofstream fog( fogName.c_str() );
    ofstream fogl( foglName.c_str() );
    
    for( int i=0; i<nMiddle+1; i++ )
    {
        fog << outputBgWeights.at(i) << endl;
        fogl << outputBgLossWeights.at(i) << endl;
    }
    
    fog.close();
    fogl.close();
}

void strategytdorigbg::loadWeights( const string& subPath, const string& filePrefix )
{
    strategytdoriggam::loadWeights( subPath, filePrefix );
    
    // try to load the bg weights. If they're not there, initialize with
    // random numbers. That lets us initialize the strategy with weights
    // from strategytdoriggam if we like, and keep the training for the
    // other outputs.
    
    string path = "/Users/mghiggins/bgtdres";
    if( subPath != "" ) path += "/" + subPath;
    
    // define the file names for the weights files
    
    string fogName  = path + "/weightsOutBg_" + filePrefix + ".txt";
    string foglName = path + "/weightsOutBgLoss_" + filePrefix + ".txt";
    
    // open the files for reading
    
    ifstream fog( fogName.c_str(), ios::in );
    ifstream fogl( foglName.c_str(), ios::in );
    
    if( !fog )
        setupBgWeights(); // no such file - set random weights
    else
    {
        string line;
        outputBgWeights.resize(nMiddle+1);
        outputBgLossWeights.resize(nMiddle+1);
        
        for( int i=0; i<nMiddle+1; i++ )
        {
            getline( fog, line );
            outputBgWeights.at(i) = atof( line.c_str() );
            getline( fogl, line );
            outputBgLossWeights.at(i) = atof( line.c_str() );
        }
    }
}

void strategytdorigbg::update( const board& oldBoard, const board& newBoard )
{
    // get the old board's probability of win and input/middle layer values
    
    vector<double> oldInputs( getInputValues( oldBoard ) );
    vector<double> oldMids( getMiddleValues( oldInputs ) );
    double oldProbWin = getOutputWin( oldMids );
    double oldProbGam = getOutputGammon( oldMids );
    double oldProbGamLoss = getOutputGammonLoss( oldMids );
    double oldProbBg = getOutputBackgammon( oldMids );
    double oldProbBgLoss = getOutputBackgammonLoss( oldMids );
    
    // calculate the partial deriv of the output node value wrt weights.
    // Two sets of weights: the ones from output->middle, and the ones
    // from each middle->input.
    
    vector<double> dPWdMiddle(nMiddle+1), dPGdMiddle(nMiddle+1), dPGLdMiddle(nMiddle+1), dPBdMiddle(nMiddle+1), dPBLdMiddle(nMiddle+1);
    vector< vector<double> > dPWdInput(nMiddle), dPGdInput(nMiddle), dPGLdInput(nMiddle), dPBdInput(nMiddle), dPBLdInput(nMiddle);
    
    int i, j;
    double probWinProd = oldProbWin * ( 1 - oldProbWin ); // cache it to avoid recalcing it over and over
    double probGamProd = oldProbGam * ( 1 - oldProbGam );
    double probGamLossProd = oldProbGamLoss * ( 1 - oldProbGamLoss );
    double probBgProd = oldProbBg * ( 1 - oldProbBg );
    double probBgLossProd = oldProbBgLoss * ( 1 - oldProbBgLoss );
    double midProd, midVal, winWeight, gamWeight, gamLossWeight, bgWeight, bgLossWeight;
    
    for( i=0; i<nMiddle; i++ )
    {
        midVal = oldMids.at(i);
        dPWdMiddle.at(i) = probWinProd * midVal;
        dPGdMiddle.at(i) = probGamProd * midVal;
        dPGLdMiddle.at(i) = probGamLossProd * midVal;
        dPBdMiddle.at(i) = probBgProd * midVal;
        dPBLdMiddle.at(i) = probBgLossProd * midVal;
        dPWdInput.at(i).resize(196);
        dPGdInput.at(i).resize(196);
        dPGLdInput.at(i).resize(196);
        dPBdInput.at(i).resize(196);
        dPBLdInput.at(i).resize(196);
        midProd = midVal * ( 1 - midVal ); // to avoid 196 recalcs
        winWeight = outputWinWeights.at(i);
        gamWeight = outputGammonWeights.at(i);
        gamLossWeight = outputGammonLossWeights.at(i);
        bgWeight = outputBgWeights.at(i);
        bgLossWeight = outputBgLossWeights.at(i);
        
        for( j=0; j<196; j++ )
        {
            dPWdInput.at(i).at(j) = probWinProd * winWeight * midProd * oldInputs.at(j);
            dPGdInput.at(i).at(j) = probGamProd * gamWeight * midProd * oldInputs.at(j);
            dPGLdInput.at(i).at(j) = probGamLossProd * gamLossWeight * midProd * oldInputs.at(j);
            dPBdInput.at(i).at(j) = probBgProd * bgWeight * midProd * oldInputs.at(j);
            dPBLdInput.at(i).at(j) = probBgLossProd * bgLossWeight * midProd * oldInputs.at(j);
        }
    }
    dPWdMiddle.at(nMiddle) = probWinProd; // bias weight deriv
    dPGdMiddle.at(nMiddle) = probGamProd; // bias weight deriv
    dPGLdMiddle.at(nMiddle) = probGamLossProd; // bias weight deriv
    dPBdMiddle.at(nMiddle) = probBgProd; // bias weight deriv
    dPBLdMiddle.at(nMiddle) = probBgLossProd; // bias weight deriv
    
    // figure out the new network value
    
    double newProbWin, newProbGam, newProbGamLoss, newProbBg, newProbBgLoss;
    
    if( newBoard.bornIn() == 15 )
    {
        newProbWin = 1;
        if( newBoard.otherBornIn() == 0 )
        {
            newProbGam = 1;
            if( newBoard.otherNoBackgammon() )
                newProbBg = 0;
            else
                newProbBg = 1;
        }
        else
        {
            newProbGam = 0;
            newProbBg  = 0;
        }
        newProbGamLoss = 0;
        newProbBgLoss  = 0;
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
        if( newBoard.otherBornIn() > 0 )
            newProbGam = 0;
        else
            newProbGam = getOutputGammonLoss( newMids );
        if( newBoard.bornIn() > 0 )
            newProbBgLoss = 0;
        else
            newProbGamLoss = getOutputGammon( newMids );
        if( newBoard.otherNoBackgammon() )
            newProbBg = 0;
        else
            newProbBg = getOutputBackgammonLoss( newMids );
        if( newBoard.noBackgammon() )
            newProbBgLoss = 0;
        else
            newProbBgLoss = getOutputBackgammon( newMids );
        if( newProbGam > newProbWin ) newProbGam = newProbWin;
        if( newProbGamLoss > 1 - newProbWin ) newProbGamLoss = 1 - newProbWin;
        if( newProbBg > newProbGam ) newProbBg = newProbGam;
        if( newProbBgLoss > newProbGamLoss ) newProbBgLoss = newProbGamLoss;
    }
    
    // train the weights
    
    double probWinDiff = newProbWin - oldProbWin;
    double probGamDiff = newProbGam - oldProbGam;
    double probGamLossDiff = newProbGamLoss - oldProbGamLoss;
    double probBgDiff = newProbBg - oldProbBg;
    double probBgLossDiff = newProbBgLoss - oldProbBgLoss;
    
    for( i=0; i<nMiddle; i++ )
    {
        outputWinWeights.at(i) += alpha * probWinDiff * dPWdMiddle.at(i);
        outputGammonWeights.at(i) += alpha * probGamDiff * dPGdMiddle.at(i);
        outputGammonLossWeights.at(i) += alpha * probGamLossDiff * dPGLdMiddle.at(i);
        outputBgWeights.at(i) += alpha * probBgDiff * dPBdMiddle.at(i);
        outputBgLossWeights.at(i) += alpha * probBgLossDiff * dPBLdMiddle.at(i);
        for( j=0; j<196; j++ )
        {
            middleWeights.at(i).at(j) += beta * probWinDiff * dPWdInput.at(i).at(j);
            middleWeights.at(i).at(j) += beta * probGamDiff * dPGdInput.at(i).at(j);
            middleWeights.at(i).at(j) += beta * probGamLossDiff * dPGLdInput.at(i).at(j);
            middleWeights.at(i).at(j) += beta * probBgDiff * dPBdInput.at(i).at(j);
            middleWeights.at(i).at(j) += beta * probBgLossDiff * dPBLdInput.at(i).at(j);
        }
    }
    outputWinWeights.at(nMiddle) += alpha * probWinDiff * dPWdMiddle.at(nMiddle); // bias weight
    outputGammonWeights.at(nMiddle) += alpha * probGamDiff * dPGdMiddle.at(nMiddle); // bias weight
    outputGammonLossWeights.at(nMiddle) += alpha * probGamLossDiff * dPGLdMiddle.at(nMiddle); // bias weight
    outputBgWeights.at(nMiddle) += alpha * probBgDiff * dPBdMiddle.at(nMiddle); // bias weight
    outputBgLossWeights.at(nMiddle) += alpha * probBgLossDiff * dPBLdMiddle.at(nMiddle); // bias weight
}

