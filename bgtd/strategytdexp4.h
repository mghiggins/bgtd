//
//  strategytdexp4.h
//  bgtd
//
//  Created by Mark Higgins on 9/3/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategytdexp4_h
#define bgtd_strategytdexp4_h

#include "strategytdbase.h"
#include "strategytdexp2.h"

class strategytdexp4 : public strategytdbase
{
    // Class like strategytdexp2, in that it does not make symmetry assumptions and
    // it includes the "whose turn is it" input; but the layout is like Tesauro's
    // classic layout, always laid out from the perspective of player 0 instead of
    // the board perspective. There are five output nodes:
    // probability of any kind of win; probability of a gammon win conditioned on a
    // win; probability of a gammon loss conditioned on a loss; probability of a
    // backgammon conditioned on a gammon win; and probability of a backgammon loss conditioned
    // on a gammon loss.
    // Also adds a bias node to all the middle nodes.
public:
    // initialize with random weights
    strategytdexp4( int nMiddle );
    // initialize with a tdexp2 network
    strategytdexp4( const strategytdexp2& baseStrat );
    // initialize using saved weights; if loadExp2 is true it assumes that the weights
    // are from a tdexp2 network, otherwise from a proper tdexp4 network
    strategytdexp4( const string& pathEnd, const string& fileSuffix, bool loadExp2 );
    virtual ~strategytdexp4() {};
    
    // boardValue returns the expected number of points the player with the dice will
    // win given the board setup. This is what the game uses to determine which move to
    // make. Returns the value from the perspective of the board, since that is the
    // standard API, regardless of how the network is configured internally.
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 ) const;
    
    // the next set of methods is used in evaluating the neural network outputs given the
    // inputs defined by the board. The inputs are always laid out from player 0's perspective
    // and the probabilities always represent the probabilities of win/loss from player 0's
    // perspective.
    
    vector<double> getInputValues( const board& brd, int turn ) const;
    vector<double> getMiddleValues( const vector<double>& inputs ) const;
    double getOutputProbValue( const vector<double>& middles ) const;
    double getOutputGammonWinValue( const vector<double>& middles, const board& brd ) const;
    double getOutputGammonLossValue( const vector<double>& middles, const board& brd ) const;
    double getOutputBackgammonWinValue( const vector<double>& middles, const board& brd ) const;
    double getOutputBackgammonLossValue( const vector<double>& middles, const board& brd ) const;
    
    // the next set of methods is used to introspect on the state of the weights
    
    vector<double> getOutputProbWeights() const;
    vector<double> getOutputGammonWinWeights() const;
    vector<double> getOutputGammonLossWeights() const;
    vector<double> getOutputBackgammonWinWeights() const;
    vector<double> getOutputBackgammonLossWeights() const;
    
    vector< vector<double> > getMiddleWeights() const;
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights. needsUpdate tells the game whether it needs to do an update.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    
protected:
    void setupExp( const strategytdexp2& baseStrat );
    
    vector<double> outputProbWeights;
    vector<double> outputGammonWinWeights;
    vector<double> outputGammonLossWeights;
    vector<double> outputBackgammonWinWeights;
    vector<double> outputBackgammonLossWeights;
    
    vector< vector<double> > middleWeights;
    
    // define some vectors to hold derivative values in the update function. They're
    // overwritten each update, but we allocate memory for the vectors at startup
    // so we don't do it each iteration.
    
    vector<double> probDerivs;                      // derivs of output prob node to its weights vs middle node values
    vector<double> gamWinDerivs;                    // derivs of output gammon win node to its weights vs middle node values
    vector<double> gamLossDerivs;                   // derivs of output gammon loss node to its weights vs middle node values
    vector<double> bgWinDerivs;                     // derivs of output backgammon win node to its weights vs middle node values
    vector<double> bgLossDerivs;                    // derivs of output backgammon loss node to its weights vs middle node values
    vector< vector<double> > probInputDerivs;       // deriv of the prob win output to all the middle->input weights
    vector< vector<double> > gamWinInputDerivs;     // deriv of the gammon win output to all the middle->input weights
    vector< vector<double> > gamLossInputDerivs;    // deriv of the gammon loss output to all the middle->input weights
    vector< vector<double> > bgWinInputDerivs;      // deriv of the backgammon win output to all the middle->input weights
    vector< vector<double> > bgLossInputDerivs;     // deriv of the backgammon loss output to all the middle->input weights
};

#endif
