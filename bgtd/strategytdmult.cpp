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
#include <fstream>
#include "bearofffns.h"
#include "gamefns.h"
#include "randomc.h"
#include "strategytdmult.h"

vector<double> randomWeights( int nWeights, CRandomMersenne& rng );

vector<double> randomWeights( int nWeights, CRandomMersenne& rng )
{
    // generates a random vector of double weights, each btw -0.1 and +0.1
    
    vector<double> weights(nWeights);
    for( int i=0; i<nWeights; i++ )
        weights.at(i) = rng.IRandom(-100,100)/10000.;
    return weights;
}

strategytdmult::strategytdmult()
{
    useShotProbInput = true;
    usePrimesInput   = true;
    setupRandomWeights( 40 ); // default # of middle nodes is 40
}

strategytdmult::strategytdmult( int nMiddle, bool useShotProbInput, bool usePrimesInput, bool useExtendedBearoffInputs ) 
  : useShotProbInput(useShotProbInput), usePrimesInput(usePrimesInput), useExtendedBearoffInputs(useExtendedBearoffInputs)
{
    setupRandomWeights( nMiddle );
}

strategytdmult::strategytdmult( const string& path, const string& filePrefix, bool randomExtendedBearoff )
{
    loadWeights( path, filePrefix, randomExtendedBearoff );
    setup();
}

int strategytdmult::nInputs( const string& netName ) const
{
    if( netName == "race" )
    {
        if( useExtendedBearoffInputs )
            return 226;
        else
            return 198;
    }
    else
    {
        if( usePrimesInput and not useShotProbInput ) throw "Must include shot prob input if include primes input";
        if( usePrimesInput )
            return 204;
        else if( useShotProbInput )
            return 200;
        else
            return 198;
    }
}

void strategytdmult::setupRandomWeights( int nMiddle )
{
    this->nMiddle = nMiddle;
    
    // define the random # generator we'll use to get the random weights
    
    CRandomMersenne rng(1);
    
    // set up the network vectors for all the different networks
    
    vector<string> nets(3);
    nets[0] = "contact";
    nets[1] = "crashed";
    nets[2] = "race";
    
    int i, nInput;
    
    for( vector<string>::iterator it=nets.begin(); it!=nets.end(); it++ )
    {
        outputProbWeights[ (*it) ] = randomWeights( nMiddle + 1, rng );
        outputGammonWeights[ (*it) ] = randomWeights( nMiddle + 1, rng );
        outputGammonLossWeights[ (*it) ] = randomWeights( nMiddle + 1, rng );
        outputBgWeights[ (*it) ] = randomWeights( nMiddle + 1, rng );
        outputBgLossWeights[ (*it) ] = randomWeights( nMiddle + 1, rng );
        
        nInput = nInputs( (*it) );
        
        vector< vector<double> > middles(nMiddle);
        for( i=0; i<nMiddle; i++ ) middles.at(i) = randomWeights( nInput+1, rng );
        middleWeights[ (*it) ] = middles;
    }
    
    setup();
}

void strategytdmult::setup()
{
    // sets up non-weights stuff
    
    // assign sensible values for alpha and beta
    
    alpha = 0.1;
    beta  = 0.1;
    
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

gameProbabilities strategytdmult::boardProbabilities( const board& brd, const hash_map<string,int>* context )
{
    // Used to evaluate possible moves, and so needs to represent the value of the game assuming the player isn't
    // holding the dice. We do this by flipping the board perspective and returning probabilities flipped back
    // appropriately.
    
    board flippedBoard( brd );
    flippedBoard.setPerspective( 1 - brd.perspective() );
    
    // figure out what we're optimizing: equity or any kind of win. We optimize for the latter in
    // 1-game matches or eg in a 6-6 tie in a 7-game match. We determine this state using the
    // supplied context. The context must have an element called "singleGame" and the value
    // associated with it must be 1 for the machine to optimize on any win rather
    // than full equity.
    // NOTE that boardValue still returns an equity in this case, but one where all games are
    // worth 1 (as opposed to normal equity where gammons are worth 2 and backgammons are worth 3).
    // The game probabilities for backgammon and gammon return zero in that case.
    
    bool valIsAnyWin=false;
    if( context != 0 )
    {
        hash_map<string,int>::const_iterator it=context->find( "singleGame" );
        if( it != context->end() and it->second == 1 )
            valIsAnyWin = true;
    }
    
    // from the board position figure out how we'll evaluate it. There are two hardcoded
    // possibilities: "done", where the game is over; and "bearoff", where we're in a bearoff
    // position. Otherwise the string refers to the name of a particular neural network, since
    // we have multiple networks that focus on different stages of the game.
    
    string eval = evaluator( flippedBoard );
    if( eval == "done" )
    {
        double val = -doneValue( flippedBoard, valIsAnyWin );
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
        double probGammonWin=0, probGammonLoss=0;
        if( !valIsAnyWin )
        {
            probGammonWin  = bearoffProbabilityGammonLoss( flippedBoard );
            probGammonLoss = bearoffProbabilityGammon( flippedBoard );
        }
        gameProbabilities probs( probWin, probGammonWin, probGammonLoss, 0, 0 );
        return probs;
    }
    else
    {
        // evaluate the network probabilities: prob of any kind of win; prob of gammon win; prob
        // of gammon loss; prob of bg win; prob of bg loss. Sanity check to make sure probs are
        // ordered sensibly.
        
        vector<double> middles( getMiddleValues( getInputValues(flippedBoard,eval), eval ) );
        
        // get the prob of any kind of win - we always need this
        
        double pWin = getOutputProbValue( middles, eval );
        
        // if we're optimizing for any win (rather than full equity) this is all we need. 
        
        if( valIsAnyWin )
        {
            gameProbabilities probs( 1-pWin, 0, 0, 0, 0 );
            return probs;
        }
        else
        {
            // Otherwise we're optimizing for full equity. Get the probability of gammon win and loss.
            
            double pGam, pGamLoss;
            if( flippedBoard.otherBornIn() == 0 )
                pGam = getOutputGammonValue( middles, eval );
            else
                pGam = 0;
            if( flippedBoard.bornIn() == 0 )
                pGamLoss = getOutputGammonLossValue( middles, eval );
            else
                pGamLoss = 0;
            
            // check that prob of gammon win <= prob of any win, and ditto for loss
            
            if( pGam > pWin ) pGam = pWin;
            if( pGamLoss > 1 - pWin ) pGamLoss = 1 - pWin;
            
            // get the probability of backgammon win and loss
            
            double pBg, pBgLoss;
            if( flippedBoard.otherNoBackgammon() )
                pBg = 0;
            else
                pBg = getOutputBackgammonValue( middles, eval );
            if( flippedBoard.noBackgammon() )
                pBgLoss = 0;
            else
                pBgLoss = getOutputBackgammonLossValue( middles, eval );
            
            // check that prob of bg win <= prob of gammon win, same for loss
            
            if( pBg > pGam ) pBg = pGam;
            if( pBgLoss > pGamLoss ) pBgLoss = pGamLoss;
            
            gameProbabilities probs( 1-pWin, pGamLoss, pGam, pBgLoss, pBg );
            return probs;
        }
    }
}

vector<double> strategytdmult::getInputValues( const board& brd, const string& netName ) const
{
    vector<double> inputs( getBaseInputValues( brd ) );
    
    // if we're doing extended bearoff inputs for the race network, add them now
    
    if( netName == "race" and useExtendedBearoffInputs )
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
        if( useShotProbInput )
        {
            inputs.push_back( hittingProb2( brd, true ) );
            inputs.push_back( hittingProb2( brd, false ) );
        }
        if( usePrimesInput )
        {
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
    }
    
    return inputs;
}

vector<double> strategytdmult::getBaseInputValues( const board& brd ) const
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

vector<double> strategytdmult::getMiddleValues( const vector<double>& inputs, const string& netName ) const
{
    vector<double> vals;
    vals.resize( nMiddle );
    int i, j;
    int nInput( nInputs( netName ) );
    double val;
    
    hash_map< string, vector< vector<double> > >::const_iterator it = middleWeights.find( netName );
    if( it==middleWeights.end() ) throw "No element found";
    
    const double * inputsRaw = &inputs[0];
    
    for( i=0; i<nMiddle; i++ )
    {
        //const vector<double>& weights = it->second.at(i);
        const double * weightsRaw = &(it->second.at(i)[0]);
        
        val = 0;
        
        for( j=0; j<nInput; j++ )
            val += weightsRaw[j] * inputsRaw[j];
        val += weightsRaw[nInput]; // bias weight
        vals[i] = 1./(1+exp(-val));
    }
    
    return vals;
}

double strategytdmult::getOutputProbValue( const vector<double>& middles, const string& netName ) const
{
    hash_map< string, vector<double> >::const_iterator weights = outputProbWeights.find( netName );
    double arg=0;
    const double * weightsRaw = &(weights->second[0]);
    const double * middlesRaw = &middles[0];
    for( int i=0; i<nMiddle; i++ )
        arg += weightsRaw[i] * middlesRaw[i];
    arg += weightsRaw[nMiddle]; // bias weight
    return 1./(1+exp(-arg));
}

double strategytdmult::getOutputGammonValue( const vector<double>& middles, const string& netName ) const
{
    hash_map< string, vector<double> >::const_iterator weights = outputGammonWeights.find( netName );
    double arg=0;
    const double * weightsRaw = &(weights->second[0]);
    const double * middlesRaw = &middles[0];
    for( int i=0; i<nMiddle; i++ )
        arg += weightsRaw[i] * middlesRaw[i];
    arg += weightsRaw[nMiddle]; // bias weight
    return 1./(1+exp(-arg));
}

double strategytdmult::getOutputGammonLossValue( const vector<double>& middles, const string& netName ) const
{
    hash_map< string, vector<double> >::const_iterator weights = outputGammonLossWeights.find( netName );
    double arg=0;
    const double * weightsRaw = &(weights->second[0]);
    const double * middlesRaw = &middles[0];
    for( int i=0; i<nMiddle; i++ )
        arg += weightsRaw[i] * middlesRaw[i];
    arg += weightsRaw[nMiddle]; // bias weight
    return 1./(1+exp(-arg));
}

double strategytdmult::getOutputBackgammonValue( const vector<double>& middles, const string& netName ) const
{
    hash_map< string, vector<double> >::const_iterator weights = outputBgWeights.find( netName );
    double arg=0;
    const double * weightsRaw = &(weights->second[0]);
    const double * middlesRaw = &middles[0];
    for( int i=0; i<nMiddle; i++ )
        arg += weightsRaw[i] * middlesRaw[i];
    arg += weightsRaw[nMiddle]; // bias weight
    return 1./(1+exp(-arg));
}

double strategytdmult::getOutputBackgammonLossValue( const vector<double>& middles, const string& netName ) const
{
    hash_map< string, vector<double> >::const_iterator weights = outputBgLossWeights.find( netName );
    double arg=0;
    const double * weightsRaw = &(weights->second[0]);
    const double * middlesRaw = &middles[0];
    for( int i=0; i<nMiddle; i++ )
        arg += weightsRaw[i] * middlesRaw[i];
    arg += weightsRaw[nMiddle]; // bias weight
    return 1./(1+exp(-arg));
}

double strategytdmult::doneValue( const board& brd, bool valIsAnyWin ) const
{
    // value of the game assuming it's done. If valIsAnyWin==true it values
    // all wins as 1 and all losses as -1; otherwise it includes proper gammon and
    // backgammon values.
    
    // NOTE: this does not return anything sensible if the board represents a game
    // that isn't actually over.
    
    if( brd.bornIn() == 15 )
    {
        if( not valIsAnyWin and brd.otherBornIn() == 0 )
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
        if( not valIsAnyWin and brd.bornIn() == 0 )
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

double strategytdmult::bearoffValue( const board& brd, bool valIsAnyWin ) const
{
    if( valIsAnyWin )
        return 2 * getProbabilityWin( brd, bearoffNPnts ) - 1;
    else
        return getBoardPntOS( brd, bearoffNPnts );
}

double strategytdmult::bearoffProbabilityWin( const board& brd ) const
{
    return getProbabilityWin( brd, bearoffNPnts );
}

double strategytdmult::bearoffProbabilityGammon( const board& brd ) const
{
    return getProbabilityGammonWin( brd, bearoffNPnts );
}

double strategytdmult::bearoffProbabilityGammonLoss( const board& brd ) const
{
    return getProbabilityGammonLoss( brd, bearoffNPnts );
}

string strategytdmult::evaluator( const board& brd ) const
{
    // the evaluator returns the string name that defines how to calculate the board value from its
    // current setup. This is bespoke logic and can change if you want to set up many different nets
    // for different parts of the game (eg one for contact, one for race), and where you tell it to
    // use a bearoff database if appropriate.
    //
    // The two networks we use are:
    //    Race: all player checkers are past the furthest-back opponent checker
    //    Contact: everything else
    //
    // Other evaluator states are "done", when the game is over, and "bearoff", which is a race state
    // where we can evaluate equity from a lookup table instead of a neural network.
    
    // did someone win? If so, return "done".
    
    if( brd.bornIn() == 15 or brd.otherBornIn() == 15 ) return "done";
    
    string contactName = "contact";
    string crashedName = "contact";
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
        return contactName;
    
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

bool strategytdmult::needsUpdate() const
{
    return learning;
}

void strategytdmult::update( const board& oldBoard, const board& newBoard )
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
            
            vector<double> newMiddles( getMiddleValues( getInputValues( flippedBoard, newEval ), newEval ) );
            newPWin = 1 - getOutputProbValue( newMiddles, newEval ); // player win prob = 1 - other player win prob
            if( newBoard.otherBornIn() == 0 )
                newPGam = getOutputGammonLossValue( newMiddles, newEval ); // player gammon win prob = other's gammon loss prob
            else
                newPGam = 0;
            if( newBoard.bornIn() == 0 )
                newPGamLoss = getOutputGammonValue( newMiddles, newEval ); // odds of player losing a gammon = odds of other player winning a gammon
            else
                newPGamLoss = 0;
            if( newBoard.otherNoBackgammon() )
                newPBg = 0;
            else
                newPBg = getOutputBackgammonLossValue( newMiddles, newEval ); // odds of player winning a backgammon = odds of other player losing a backgammon
            if( newBoard.noBackgammon() )
                newPBgLoss = 0;
            else
                newPBgLoss = getOutputBackgammonValue( newMiddles, newEval ); // odds of player losing a backgammon = odds of other player winning a backgammon
        }
    }
    
    updateFromProbs( oldBoard, newPWin, newPGam, newPGamLoss, newPBg, newPBgLoss );
}

void strategytdmult::updateFromProbs( const board& brd, double probWin, double probGammonWin, double probGammonLoss, double probBgWin, double probBgLoss )
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
    
    // calculate the partial derivatives of the network outputs to the network weights. 
    
    vector<double> oldInputs( getInputValues( brd, netName ) );
    vector<double> oldMiddles( getMiddleValues( oldInputs, netName ) );
    double oldPWin     = getOutputProbValue( oldMiddles, netName );
    double oldPGam     = getOutputGammonValue( oldMiddles, netName );
    double oldPGamLoss = getOutputGammonLossValue( oldMiddles, netName );
    double oldPBg      = getOutputBackgammonValue( oldMiddles, netName );
    double oldPBgLoss  = getOutputBackgammonLossValue( oldMiddles, netName );
    int nInput( nInputs( netName ) );
    
    vector<double> dProbdMiddle(nMiddle+1);         // derivs of prob win output to its weights vs the middle nodes, and one extra for its bias weight
    vector<double> dGamdMiddle(nMiddle+1);          // derivs of gam win output to its weights vs the middle nodes, and one extra for its bias weight
    vector<double> dGamLossdMiddle(nMiddle+1);      // sim for gam loss
    vector<double> dBgdMiddle(nMiddle+1);           // sim for backgammon win
    vector<double> dBgLossdMiddle(nMiddle+1);       // sim for backgammon loss
    
    vector< vector<double> > dProbdInputs(nMiddle); // derivs of prob win output to middle->input weights
    vector< vector<double> > dGamdInputs(nMiddle);  // derivs of gam win output to middle->input weights
    vector< vector<double> > dGamLossdInputs(nMiddle); // sim for gam loss
    vector< vector<double> > dBgdInputs(nMiddle);   // sim for backgammon win
    vector< vector<double> > dBgLossdInputs(nMiddle); // sim for backgammon loss
    
    double pWinProd = oldPWin * ( 1 - oldPWin ); // precalc since it's called a lot
    double pGamProd = oldPGam * ( 1 - oldPGam ); // ditto
    double pGamLossProd = oldPGamLoss * ( 1 - oldPGamLoss ); // ditto
    double pBgProd = oldPBg * ( 1 - oldPBg );
    double pBgLossProd = oldPBgLoss * ( 1 - oldPBgLoss );
    
    int i, j;
    double input, middleProd, midProbWeight, midGamWeight, midGamLossWeight, midBgWeight, midBgLossWeight, mid;
    
    hash_map< string, vector<double> >::iterator itProbWeights          = outputProbWeights.find(netName);
    hash_map< string, vector<double> >::iterator itGamWeights           = outputGammonWeights.find(netName);
    hash_map< string, vector<double> >::iterator itGamLossWeights       = outputGammonLossWeights.find(netName);
    hash_map< string, vector<double> >::iterator itBgWeights            = outputBgWeights.find(netName);
    hash_map< string, vector<double> >::iterator itBgLossWeights        = outputBgLossWeights.find(netName);
    hash_map< string, vector< vector<double> > >::iterator itMidWeights = middleWeights.find(netName);
    
    for( i=0; i<nMiddle; i++ )
    {
        mid = oldMiddles.at(i);
        dProbdMiddle.at(i)    = pWinProd * mid;
        dGamdMiddle.at(i)     = pGamProd * mid;
        dGamLossdMiddle.at(i) = pGamLossProd * mid;
        dBgdMiddle.at(i)      = pBgProd * mid;
        dBgLossdMiddle.at(i)  = pBgLossProd * mid;
        
        dProbdInputs.at(i).resize(nInput+1);
        dGamdInputs.at(i).resize(nInput+1);
        dGamLossdInputs.at(i).resize(nInput+1);
        dBgdInputs.at(i).resize(nInput+1);
        dBgLossdInputs.at(i).resize(nInput+1);
        
        middleProd = mid * ( 1 - mid );
        
        midProbWeight    = (*itProbWeights).second.at(i);
        midGamWeight     = (*itGamWeights).second.at(i);
        midGamLossWeight = (*itGamLossWeights).second.at(i);
        midBgWeight      = (*itBgWeights).second.at(i);
        midBgLossWeight  = (*itBgLossWeights).second.at(i);
        
        vector<double>& probRow    = dProbdInputs.at(i);
        vector<double>& gamRow     = dGamdInputs.at(i);
        vector<double>& gamLossRow = dGamLossdInputs.at(i);
        vector<double>& bgRow      = dBgdInputs.at(i);
        vector<double>& bgLossRow  = dBgLossdInputs.at(i);
        
        for( j=0; j<nInput; j++ )
        {
            input = oldInputs.at(j);
            probRow.at(j)    = pWinProd * midProbWeight * middleProd * input;
            gamRow.at(j)     = pGamProd * midGamWeight * middleProd * input;
            gamLossRow.at(j) = pGamLossProd * midGamLossWeight * middleProd * input;
            bgRow.at(j)      = pBgProd * midBgWeight * middleProd * input;
            bgLossRow.at(j)  = pBgLossProd * midBgLossWeight * middleProd * input;
        }
        probRow.at(nInput)    = pWinProd * midProbWeight * middleProd; // bias weight
        gamRow.at(nInput)     = pGamProd * midGamWeight * middleProd;
        gamLossRow.at(nInput) = pGamLossProd * midGamLossWeight * middleProd;
        bgRow.at(nInput)      = pBgProd * midBgWeight * middleProd;
        bgLossRow.at(nInput)  = pBgLossProd * midBgLossWeight * middleProd;
    }
    
    // don't forget the partial deriv of outputs to their bias weight
    
    dProbdMiddle.at(nMiddle)    = pWinProd;
    dGamdMiddle.at(nMiddle)     = pGamProd;
    dGamLossdMiddle.at(nMiddle) = pGamLossProd;
    dBgdMiddle.at(nMiddle)      = pBgProd;
    dBgLossdMiddle.at(nMiddle)  = pBgLossProd;
    
    // now that we've got the partial derivatives we can use the target probabilities to train
    
    double probDiff    = probWin - oldPWin;
    double gamDiff     = probGammonWin - oldPGam;
    double gamLossDiff = probGammonLoss - oldPGamLoss;
    double bgDiff      = probBgWin - oldPBg;
    double bgLossDiff  = probBgLoss - oldPBgLoss;
    
    for( i=0; i<nMiddle; i++ )
    {
        (*itProbWeights).second.at(i) += alpha * probDiff * dProbdMiddle.at(i);
        (*itGamWeights).second.at(i) += alpha * gamDiff * dGamdMiddle.at(i);
        (*itGamLossWeights).second.at(i) += alpha * gamLossDiff * dGamLossdMiddle.at(i);
        (*itBgWeights).second.at(i) += alpha * bgDiff * dBgdMiddle.at(i);
        (*itBgLossWeights).second.at(i) += alpha * bgLossDiff * dBgLossdMiddle.at(i);
        
        vector<double>& midRow = (*itMidWeights).second.at(i);
        vector<double>& prodRow = dProbdInputs.at(i);
        vector<double>& gamRow = dGamdInputs.at(i);
        vector<double>& gamLossRow = dGamLossdInputs.at(i);
        vector<double>& bgRow = dBgdInputs.at(i);
        vector<double>& bgLossRow = dBgLossdInputs.at(i);
        
        for( j=0; j<nInput+1; j++ )
            midRow.at(j) += beta * ( probDiff * prodRow.at(j)
                                   + gamDiff * gamRow.at(j)
                                   + gamLossDiff * gamLossRow.at(j)
                                   + bgDiff * bgRow.at(j) 
                                   + bgLossDiff * bgLossRow.at(j) );
    }
    
    (*itProbWeights).second.at(nMiddle)    += alpha * probDiff * dProbdMiddle.at(nMiddle);
    (*itGamWeights).second.at(nMiddle)     += alpha * gamDiff * dGamdMiddle.at(nMiddle);
    (*itGamLossWeights).second.at(nMiddle) += alpha * gamLossDiff * dGamLossdMiddle.at(nMiddle);
    (*itBgWeights).second.at(nMiddle)      += alpha * bgDiff * dBgdMiddle.at(nMiddle);
    (*itBgLossWeights).second.at(nMiddle)  += alpha * bgLossDiff * dBgLossdMiddle.at(nMiddle);
}

void strategytdmult::writeWeights( const string& filePrefix ) const
{
    // write the weights and traces to files. Six files per network name: one for prob of win output->middle weights;
    // one for of prob gammon output->middle weights; one for gammon loss; one for bg win; one for bg loss and 
    // one for middle->input weights. And one for the list of network names.
    
    string path = "/Users/mghiggins/bgtdres";
    
    // define the file name for the list of network names and open that file. This file will also contain the number
    // of middle nodes. Also entries for the optional parameters.
    
    string netName = path + "/netNames_" + filePrefix + ".txt";
    ofstream fn( netName.c_str() );
    fn << nMiddle << endl;
    if( useShotProbInput )
        fn << 1 << endl;
    else
        fn << 0 << endl;
    if( usePrimesInput )
        fn << 1 << endl;
    else
        fn << 0 << endl;
    if( useExtendedBearoffInputs )
        fn << 1 << endl;
    else
        fn << 0 << endl;
    
    for( hash_map< string, vector<double> >::const_iterator itProb = outputProbWeights.begin(); itProb != outputProbWeights.end(); itProb ++ )
    {
        // write the network name to the file
        
        fn << itProb->first << endl;
        
        // get the # of inputs for this network
        
        int nInput( nInputs( itProb->first ) );
        
        // get references to the other weights vectors
        
        hash_map< string, vector<double> >::const_iterator itGam = outputGammonWeights.find( itProb->first );
        hash_map< string, vector<double> >::const_iterator itGamLoss = outputGammonLossWeights.find( itProb->first );
        hash_map< string, vector<double> >::const_iterator itBg = outputBgWeights.find( itProb->first );
        hash_map< string, vector<double> >::const_iterator itBgLoss = outputBgLossWeights.find( itProb->first );
        hash_map< string, vector< vector<double> > >::const_iterator itMid = middleWeights.find( itProb->first );
        
        // define the file names for the weights files
        
        string fopName  = path + "/weightsOutProb_" + filePrefix + "_" + itProb->first + ".txt";
        string fogName  = path + "/weightsOutGammon_" + filePrefix + "_" + itProb->first + ".txt";
        string foglName = path + "/weightsOutGammonLoss_" + filePrefix + "_" + itProb->first + ".txt";
        string fobName  = path + "/weightsOutBg_" + filePrefix + "_" + itProb->first + ".txt";
        string foblName = path + "/weightsOutBgLoss_" + filePrefix + "_" + itProb->first + ".txt";
        string fmName   = path + "/weightsMiddle_" + filePrefix + "_" + itProb->first + ".txt";
        
        ofstream fop( fopName.c_str() );
        ofstream fog( fogName.c_str() );
        ofstream fogl( foglName.c_str() );
        ofstream fob( fobName.c_str() );
        ofstream fobl( foblName.c_str() );
        ofstream fm( fmName.c_str() );
        
        for( int j=0; j<nMiddle; j++ )
        {
            fop << itProb->second.at(j) << endl;
            fog << itGam->second.at(j) << endl;
            fogl << itGamLoss->second.at(j) << endl;
            fob << itBg->second.at(j) << endl;
            fobl << itBgLoss->second.at(j) << endl;
            for( int k=0; k<nInput+1; k++ )
                fm  << itMid->second.at(j).at(k) << endl;
        }
        fop << itProb->second.at(nMiddle) << endl;
        fog << itGam->second.at(nMiddle) << endl;
        fogl << itGamLoss->second.at(nMiddle) << endl;
        fob << itBg->second.at(nMiddle) << endl;
        fobl << itBgLoss->second.at(nMiddle) << endl;
        
        fop.close();
        fog.close();
        fogl.close();
        fob.close();
        fobl.close();
        fm.close();
    }
    
    fn.close();
}

void strategytdmult::loadWeights( const string& subPath, const string& filePrefix, bool randomExtendedBearoff )
{
    // load the # of middle nodes and the network names from the names file
    
    string path = "/Users/mghiggins/bgtdres";
    if( subPath != "" ) path += "/" + subPath;
    
    // define the file name for the list of network names and open that file. This file will also contain the number
    // of middle nodes.
    
    string netFileName = path + "/netNames_" + filePrefix + ".txt";
    
    ifstream fn( netFileName.c_str(), ios::in );
    if( !fn ) throw "No netNames file with the supplied prefix";
    
    string line;
    getline( fn, line );
    nMiddle = atoi( line.c_str() );
    
    // the file will also contain values for the optional parameters
    
    getline( fn, line );
    useShotProbInput = ( atoi( line.c_str() ) == 1 );
    getline( fn, line );
    usePrimesInput = ( atoi( line.c_str() ) == 1 );
    getline( fn, line );
    useExtendedBearoffInputs = ( atoi( line.c_str() ) == 1 );
    if( randomExtendedBearoff )
    {
        if( useExtendedBearoffInputs ) throw "Cannot assign random weights to extended bearoff inputs - the file already contains these weights";
        useExtendedBearoffInputs = true; // we now assume the file doesn't contain them
    }
    if( usePrimesInput and not useShotProbInput ) throw "Invalid file: if usePrimesInput, cannot be not useShotProbInput";
    
    CRandomMersenne rng(1); // used for random weights if we're loading weights from a file with no shot prob weight but the network needs one
    int i, j, nInput;
    
    // clear any existing elements from the weights maps
    
    if( outputProbWeights.size() )
    {
        outputProbWeights.clear();
        outputGammonWeights.clear();
        outputGammonLossWeights.clear();
        outputBgWeights.clear();
        outputBgLossWeights.clear();
        middleWeights.clear();
    }
    
    // load the appropriate weights
    
    while( not fn.eof() )
    {
        string line;
        getline( fn, line );
        string netName( line );
        if( netName != "" )
        {
            // load the weights and assign them appropriately
            
            vector<double> probWeights(nMiddle+1); // one extra for bias weight
            vector<double> gamWeights(nMiddle+1);
            vector<double> gamLossWeights(nMiddle+1);
            vector<double> bgWeights(nMiddle+1);
            vector<double> bgLossWeights(nMiddle+1);
            vector< vector<double> > midWeights(nMiddle);
            
            string fopName  = path + "/weightsOutProb_" + filePrefix + "_" + netName + ".txt";
            string fogName  = path + "/weightsOutGammon_" + filePrefix + "_" + netName + ".txt";
            string foglName = path + "/weightsOutGammonLoss_" + filePrefix + "_" + netName + ".txt";
            string fobName  = path + "/weightsOutBg_" + filePrefix + "_" + netName + ".txt";
            string foblName = path + "/weightsOutBgLoss_" + filePrefix + "_" + netName + ".txt";
            string fmName   = path + "/weightsMiddle_" + filePrefix + "_" + netName + ".txt";
            
            ifstream fop( fopName.c_str() );
            ifstream fog( fogName.c_str() );
            ifstream fogl( foglName.c_str() );
            ifstream fob( fobName.c_str() );
            ifstream fobl( foblName.c_str() );
            ifstream fm( fmName.c_str() );
            
            nInput = nInputs( netName );
            
            for( i=0; i<nMiddle; i++ )
            {
                getline( fop, line );
                probWeights.at(i) = atof( line.c_str() );
                getline( fog, line );
                gamWeights.at(i) = atof( line.c_str() );
                getline( fogl, line );
                gamLossWeights.at(i) = atof( line.c_str() );
                getline( fob, line );
                bgWeights.at(i) = atof( line.c_str() );
                getline( fobl, line );
                bgLossWeights.at(i) = atof( line.c_str() );
                midWeights.at(i).resize(nInput+1);
                for( j=0; j<nInput+1; j++ )
                {
                    // base inputs - always there for any network; also last element is always the bias weight and
                    // needs to go in the last slot. Also contact we always load all weights. 
                    // For the race network, if we're using random extended bearoff weights, add those in between
                    // the last base weight and the bias weight, since they're not in the file we're loading from.
                    
                    if( netName != "race" or !randomExtendedBearoff or j < 198 or j == nInput )
                    {
                        getline( fm, line );
                        midWeights.at(i).at(j) = atof( line.c_str() );
                    }
                    else if( randomExtendedBearoff )
                        midWeights.at(i).at(j) = rng.IRandom(-100,100)/1000.;
                }
            }
            getline( fop, line );
            probWeights.at(nMiddle) = atof( line.c_str() );
            getline( fog, line );
            gamWeights.at(nMiddle) = atof( line.c_str() );
            getline( fogl, line );
            gamLossWeights.at(nMiddle) = atof( line.c_str() );
            getline( fob, line );
            bgWeights.at(nMiddle) = atof( line.c_str() );
            getline( fobl, line );
            bgLossWeights.at(nMiddle) = atof( line.c_str() );
            
            outputProbWeights[ netName ]   = probWeights;
            outputGammonWeights[ netName ] = gamWeights;
            outputGammonLossWeights[ netName ] = gamLossWeights;
            outputBgWeights[ netName ] = bgWeights;
            outputBgLossWeights[ netName ] = bgLossWeights;
            middleWeights[ netName ]       = midWeights;
        }
    }
}

const vector<double>& getWeightsFromMap( const hash_map< string, vector<double> >& map, const string& netName );
const vector<double>& getWeightsFromMap( const hash_map< string, vector<double> >& map, const string& netName )
{
    hash_map< string, vector<double> >::const_iterator it=map.find(netName);
    if( it == map.end() ) throw string( "Could not find network name" );
    return it->second;
}

const vector<double>& strategytdmult::getOutputProbWeights( const string& netName ) const
{
    return getWeightsFromMap( outputProbWeights, netName );
}

const vector<double>& strategytdmult::getOutputGammonWeights( const string& netName ) const
{
    return getWeightsFromMap( outputGammonWeights, netName );
}

const vector<double>& strategytdmult::getOutputGammonLossWeights( const string& netName ) const
{
    return getWeightsFromMap( outputGammonLossWeights, netName );
}

const vector<double>& strategytdmult::getOutputBgWeights( const string& netName ) const
{
    return getWeightsFromMap( outputBgWeights, netName );
}

const vector<double>& strategytdmult::getOutputBgLossWeights( const string& netName ) const
{
    return getWeightsFromMap( outputBgLossWeights, netName );
}

const vector< vector<double> >& strategytdmult::getMiddleWeights( const string& netName ) const
{
    hash_map< string, vector< vector<double> > >::const_iterator it=middleWeights.find(netName);
    if( it == middleWeights.end() ) throw string( "Could not find network name" );
    return it->second;
}