//
//  strategymult.cpp
//  bgtd
//
//  Created by Mark Higgins on 3/2/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#include "bearofffns.h"
#include "gamefns.h"
#include "strategymult.h"

strategymult::strategymult()
{
    nMiddle = 40;
    setup();
}

strategymult::strategymult( int nMiddle )
{
    this->nMiddle = nMiddle;
    setup();
}

strategymult::strategymult( const string& path, const string& filePrefix )
{
    setup();
    loadNets( path, filePrefix );
}

int strategymult::nInputs( const string& netName ) const
{
    if( netName == "race" )
        return 226;
    else
        return 204;
}

void strategymult::setup()
{
    // set up the networks
    
    annContact = fann_create_standard( 3, nInputs("contact"), nMiddle, 5 );
    annCrashed = fann_create_standard( 3, nInputs("crashed"), nMiddle, 5 );
    annRace    = fann_create_standard( 3, nInputs("race"),    nMiddle, 5 );
    fann_set_training_algorithm( annContact, FANN_TRAIN_INCREMENTAL );
    fann_set_training_algorithm( annCrashed, FANN_TRAIN_INCREMENTAL );
    fann_set_training_algorithm( annRace,    FANN_TRAIN_INCREMENTAL );
    
    // assign sensible values for alpha and beta
    
    alpha = 0.1;
    
    // load the bearoff dbs - hardcode the bearoff parameters and file location for now.
    // Two bearoff dbs: one for the distribution of steps to take off the last checker,
    // used to determine prob of win; and one for the distribution of steps to take off the
    // first checker, used to determine the prob of gammon win/loss.
    
    bearoffNPnts     = 6;
    bearoffNCheckers = 15;
    
    if( stepsProbs() == 0 )
    {
        loadBearoffDbOneSided( "/Users/mghiggins/bgtdres/benchmark/bearoffOS_6_15.csv" );
        loadGammonBearoffDbOneSided( "/Users/mghiggins/bgtdres/benchmark/bearoffOSGam_6_15.csv" );
        cout << "One-sided db size = " << stepsProbs()->size() << endl;
        cout << "Gammon db size    = " << gamStepsProbs()->size() << endl;
        setupHittingRolls();
    }
    
    // initialize the escape db. We don't bother serializing this one since it's quick to calculate
    
    if( blockadeEscapeRolls() == 0 ) 
    {
        constructBlockadeEscapeDb();
        cout << "Escape db size    = " << blockadeEscapeRolls()->size() << endl;
    }
}

gameProbabilities strategymult::boardProbabilities( const board& brd, const hash_map<string,int>* context )
{
    // Used to evaluate possible moves, and so needs to represent the value of the game assuming the player isn't
    // holding the dice. We do this by flipping the board perspective and returning probabilities flipped back
    // appropriately.
    
    board flippedBoard( brd );
    flippedBoard.setPerspective( 1 - brd.perspective() );
    
    string eval = evaluator( flippedBoard );
    if( eval == "done" )
    {
        double val = -doneValue( flippedBoard );
        gameProbabilities probs(0,0,0,0,0);
        if( val > 0 ) probs.probWin = 1;
        if( val > 1 ) probs.probGammonWin = 1;
        if( val > 2 ) probs.probBgWin  = 1;
        if( val < -1 ) probs.probGammonLoss = 1;
        if( val < -2 ) probs.probBgLoss = 1;
        return probs;
    }
    else if( eval == "bearoff" )
    {
        double probWin = 1 - bearoffProbabilityWin( flippedBoard );
        double probGammonWin  = bearoffProbabilityGammonLoss( flippedBoard );
        double probGammonLoss = bearoffProbabilityGammon( flippedBoard );
        gameProbabilities probs( probWin, probGammonWin, probGammonLoss, 0, 0 );
        return probs;
    }
    else
    {
        vector<double> inputs( getInputValues( flippedBoard, eval ) );
        return getOutputProbs( inputs, eval ).flippedProbs();
    }
}

gameProbabilities strategymult::getOutputProbs( const vector<double>& inputs, const string& netName ) const
{
    // evaluate the network probabilities: prob of any kind of win; prob of gammon win; prob
    // of gammon loss; prob of bg win; prob of bg loss. Sanity check to make sure probs are
    // ordered sensibly.
    
    fann * ann;
    if( netName == "race" )
        ann = annRace;
    else if( netName == "contact" )
        ann = annContact;
    else if( netName == "crashed" )
        ann = annCrashed;
    else
        throw string( "Unknown network name - could not get probs" );
    
    double * outputs = fann_run( ann, (double*) &inputs[0] );
    
    double pWin     = outputs[0];
    double pGam     = outputs[1];
    double pBg      = outputs[2];
    double pGamLoss = outputs[3];
    double pBgLoss  = outputs[4];
    
    if( pGam > pWin ) pGam = pWin;
    if( pGamLoss > 1 - pWin ) pGamLoss = 1 - pWin;
    
    if( pBg > pGam ) pBg = pGam;
    if( pBgLoss > pGamLoss ) pBgLoss = pGamLoss;
    
    gameProbabilities probs( pWin, pGam, pGamLoss, pBg, pBgLoss );
    return probs;
}

vector<double> strategymult::getInputValues( const board& brd, const string& netName ) const
{
    vector<double> inputs( getBaseInputValues( brd ) );
    
    if( netName == "race" )
    {
        // 14 inputs for each player: the i'th input is 1 if the player has borne in at least i checkers
        
        for( int i=1; i<15; i++ )
        {
            if( brd.bornIn() < i )
                inputs.push_back(0);
            else
                inputs.push_back(1);
        }
        for( int i=1; i<15; i++ )
        {
            if( brd.otherBornIn() < i )
                inputs.push_back(0);
            else
                inputs.push_back(1);
        }
    }
    
    // for contact networks, add extended inputs
    
    if( netName != "race" )
    {
        inputs.push_back( hittingProb2( brd, true ) );
        inputs.push_back( hittingProb2( brd, false ) );
        
        // for primes we follow Berliner - for the player, one input that's the minimum number of escape rolls from 24->16, and
        // another that's from 16->2. The mirror image for the opponent.
        
        for( int persp=0; persp<2; persp++ )
        {
            board useBrd( brd );
            if( persp==1 ) useBrd.setPerspective( 1 - brd.perspective() ); // we're doing the opponent
            
            int i;
            int rolls, rollsMin=36; // we use this if there are no blockades
            for( i=23; i>=15; i-- )
            {
                if( useBrd.checker(i) == 0 ) continue;
                rolls = getBlockadeEscapeCount( useBrd, i );
                if( rolls < rollsMin ) rollsMin = rolls;
            }
            inputs.push_back( rollsMin / 36. );
            rollsMin = 36;
            for( i=15; i>=1; i-- )
            {
                if( useBrd.checker(i) == 0 ) continue;
                rolls = getBlockadeEscapeCount( useBrd, i );
                if( rolls < rollsMin ) rollsMin = rolls;
            }
            inputs.push_back( rollsMin / 36. );
        }
    }
    
    return inputs;
}

vector<double> strategymult::getBaseInputValues( const board& brd ) const
{
    // standard Tesauro inputs, except no inputs for whose turn it is, and new inputs for
    // whether it's impossible to get a backgammon. These are common to the inputs for all networks.
    
    vector<double> inputs;
    inputs.resize(198,0);
    vector<int> checks( brd.checkers() );
    vector<int> otherChecks( brd.otherCheckers() );
    int hit          = brd.hit();
    int otherHit     = brd.otherHit();
    int borneIn      = brd.bornIn();
    int otherBorneIn = brd.otherBornIn();
    bool noBg        = brd.noBackgammon();
    bool otherNoBg   = brd.otherNoBackgammon();
    
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
            if( checks[i] > j )      inputs[4*i+j]    = 1;
            if( otherChecks[i] > j ) inputs[4*i+j+99] = 1;
        }
        if( checks[i] > 3 )      inputs[4*i+3]    = ( checks[i]-3 ) / 2.;
        if( otherChecks[i] > 3 ) inputs[4*i+3+99] = ( otherChecks[i]-3 ) / 2.;
    }
    
    // one spot for each player records the number on the bar
    
    inputs.at(96)  = hit / 2.;
    inputs.at(195) = otherHit / 2.;
    
    // one spot for each player records the number born in
    
    inputs.at(97)  = borneIn / 15.;
    inputs.at(196) = otherBorneIn / 15.;
    
    // one spot for each player records whether a backgammon is possible
    
    inputs.at(98)  = noBg ? 1 : 0;
    inputs.at(197) = otherNoBg ? 1 : 0;
    
    return inputs;
}

double strategymult::doneValue( const board& brd ) const
{
    // value of the game assuming it's done. 
    
    // NOTE: this does not return anything sensible if the board represents a game
    // that isn't actually over.
    
    if( brd.bornIn() == 15 )
    {
        if( brd.otherBornIn() == 0 )
        {
            if( not brd.otherNoBackgammon() )
                return 3; // backgammon
            else
                return 2; // gammon
        }
        else
            return 1;
    }
    else
    {
        if( brd.bornIn() == 0 )
        {
            if( not brd.noBackgammon() )
                return -3; // backgammon loss
            else
                return -2; // gammon loss
        }
        else
            return -1;
    }
}

double strategymult::bearoffValue( const board& brd ) const
{
    return getBoardPntOS( brd, bearoffNPnts );
}

double strategymult::bearoffProbabilityWin( const board& brd ) const
{
    return getProbabilityWin( brd, bearoffNPnts );
}

double strategymult::bearoffProbabilityGammon( const board& brd ) const
{
    return getProbabilityGammonWin( brd, bearoffNPnts );
}

double strategymult::bearoffProbabilityGammonLoss( const board& brd ) const
{
    return getProbabilityGammonLoss( brd, bearoffNPnts );
}

string strategymult::evaluator( const board& brd ) const
{
    // the evaluator returns the string name that defines how to calculate the board value from its
    // current setup. This is bespoke logic and can change if you want to set up many different nets
    // for different parts of the game (eg one for contact, one for race), and where you tell it to
    // use a bearoff database if appropriate.
    //
    // The three networks we use are:
    //    Race: all player checkers are past the furthest-back opponent checker
    //    Crashed: in contact, bearing off, most checkers on 1 or 2 point but racing checkers back still
    //    Contact: everything else
    //
    // Other evaluator states are "done", when the game is over, and "bearoff", which is a race state
    // where we can evaluate equity from a lookup table instead of a neural network.
    
    // did someone win? If so, return "done".
    
    if( brd.bornIn() == 15 or brd.otherBornIn() == 15 ) return "done";
    
    string contactName = "contact";
    string crashedName = "crashed";
    string raceName    = "race";
    
    // is this contact? Find the furthest player piece and check if any opponent piece is in front of it.
    
    int playerBack, opponentBack;
    
    if( brd.hit() > 0 )
        playerBack = 24;
    else
    {
        for( playerBack=23; playerBack>-1; playerBack-- )
            if( brd.checker(playerBack) > 0 )
                break;
    }
    
    if( brd.otherHit() > 0 )
        opponentBack = -1;
    else
    {
        for( opponentBack=0; opponentBack<24; opponentBack++ )
            if( brd.otherChecker(opponentBack) > 0 )
                break;
    }
    
    if( playerBack > opponentBack )
    {
        // check if it's crashed
        
        if( isCrashed( brd.bornIn(), brd.checker(0), brd.checker(1) ) ) return crashedName;
        if( isCrashed( brd.otherBornIn(), brd.otherChecker(23), brd.otherChecker(22) ) ) return crashedName;
        
        // not crashed, therefore contact
        
        return contactName;
    }
    
    // it's a race - just need to figure out whether it's time to use the bearoff db
    
    // if there are more checkers left on the board than the number we use for the bearoff db, need to use the race network
    
    if( brd.bornIn() < 15 - bearoffNCheckers or brd.otherBornIn() < 15 - bearoffNCheckers ) return raceName;
    
    // if there are any checkers on spots past the largest spot we use for the bearoff db, need to use the race network
    
    int i;
    
    for( i=bearoffNPnts; i<24; i++ )
        if( brd.checker(i) > 0 ) return raceName;
    for( i=23-bearoffNPnts; i>-1; i-- )
        if( brd.otherChecker(i) > 0 ) return raceName;
    
    // otherwise we can use the bearoff db
    
    return "bearoff";
}

bool strategymult::needsUpdate() const
{
    return learning;
}

void strategymult::update( const board& oldBoard, const board& newBoard )
{
    // calculate the target probabilities from the new board, or from the bearoff
    // database if we're in bearoff (which we use to do supervised learning for
    // the race weights, instead of doing TD learning).
    
    double newPWin, newPGam, newPGamLoss, newPBg, newPBgLoss;
    string eval( evaluator( oldBoard ) );
    if( eval == "done" ) return;
    
    if( eval == "bearoff" )
    {
        newPWin     = bearoffProbabilityWin( oldBoard );
        newPGam     = bearoffProbabilityGammon( oldBoard );
        newPGamLoss = bearoffProbabilityGammonLoss( oldBoard );
        newPBg      = 0;
        newPBgLoss  = 0;
    }
    else
    {
        board flippedBoard( newBoard );
        flippedBoard.setPerspective( ( newBoard.perspective() + 1 ) % 2 );
        
        string newEval( evaluator( flippedBoard ) ); // generally want the network corresponding to the flipped board
        if( newEval == "done" )
        {
            // this fn is always called from the perspective of the player on
            // move, so a game ending can only come from that player winning.
            
            newPWin = 1;
            if( newBoard.otherBornIn() == 0 )
                newPGam = 1;
            else
                newPGam = 0;
            if( not newBoard.otherNoBackgammon() )
                newPBg = 1;
            else
                newPBg = 0;
            newPGamLoss = 0;
            newPBgLoss  = 0;
        }
        else if( newEval == "bearoff" )
        {
            // get the estimates from the bearoff db using the flipped board - need to change probabilities
            // to reflect view from the player, not the opponent.
            
            newPWin     = 1 - bearoffProbabilityWin( flippedBoard );    // prob of win == 1 - prob of opponent loss
            newPGam     = bearoffProbabilityGammonLoss( flippedBoard ); // player prob of gammon win == opponent prob of gammon loss
            newPGamLoss = bearoffProbabilityGammon( flippedBoard );     // player prob of gammon loss == opponent prob of gammon win
            newPBg      = 0;
            newPBgLoss  = 0;
        }
        else
        {
            // calculate the probabilities from the new board's network. Need to reflect that
            // the other player has the dice, so flip the board around.
            
            vector<double> inputs( getInputValues( flippedBoard, newEval ) );
            gameProbabilities probs( getOutputProbs( inputs, newEval ) );
            
            newPWin     = 1-probs.probWin;
            newPGam     = probs.probGammonLoss;
            newPGamLoss = probs.probGammonWin;
            newPBg      = probs.probBgLoss;
            newPBgLoss  = probs.probBgWin;
        }
    }
    
    updateFromProbs( oldBoard, newPWin, newPGam, newPGamLoss, newPBg, newPBgLoss );
}

void strategymult::updateFromProbs( const board& brd, double probWin, double probGammonWin, double probGammonLoss, double probBgWin, double probBgLoss )
{
    // we train the weights of the evaluator for the network corresponding to the board, if
    // that's actually using a network. If we're in bearoff, still use the results to train the 
    // race network which connects to bearoff.
    
    string eval( evaluator( brd ) );
    if( eval == "done" ) return; // no network to train
    
    string netName;
    
    if( eval == "bearoff" )
        netName = "race";
    else
        netName = eval;
    
    double outputs[5];
    outputs[0] = probWin;
    outputs[1] = probGammonWin;
    outputs[2] = probBgWin;
    outputs[3] = probGammonLoss;
    outputs[4] = probBgLoss;
    
    fann * ann;
    if( netName == "race" )
        ann = annRace;
    else if( netName == "contact" )
        ann = annContact;
    else if( netName == "crashed" )
        ann = annCrashed;
    else
        throw string( "Unknown network name " + netName + " in update" );
    
    fann_set_learning_rate( annRace, alpha );
    vector<double> inputs( getInputValues( brd, netName ) );
    fann_train( ann, (double *) &inputs[0], outputs );
}

void strategymult::writeNets( const string& filePrefix, const string& singleNetName ) const
{
    string path = "/Users/mghiggins/bgtdres";
    
    if( singleNetName == "" or singleNetName == "contact" )
    {
        string fileName = path + "/netContact.net";
        fann_save( annContact, fileName.c_str() );
    }
    if( singleNetName == "" or singleNetName == "crashed" )
    {
        string fileName = path + "/netCrashed.net";
        fann_save( annCrashed, fileName.c_str() );
    }
    if( singleNetName == "" or singleNetName == "race" )
    {
        string fileName = path + "/netRace.net";
        fann_save( annRace, fileName.c_str() );
    }
}

void strategymult::loadNets( const string& subPath, const string& filePrefix )
{
    // load the # of middle nodes and the network names from the names file
    
    string path = "/Users/mghiggins/bgtdres";
    if( subPath != "" ) path += "/" + subPath;
    
    string fileName = path + "/netContact.net";
    annContact = fann_create_from_file( fileName.c_str() );
    fileName = path + "/netCrashed.net";
    annCrashed = fann_create_from_file( fileName.c_str() );
    fileName = path + "/netRace.net";
    annRace = fann_create_from_file( fileName.c_str() );
}
