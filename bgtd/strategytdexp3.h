//
//  strategytdexp3.h
//  bgtd
//
//  Created by Mark Higgins on 8/28/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategytdexp3_h
#define bgtd_strategytdexp3_h

#include "strategytdbase.h"
#include "strategytdexp.h"

class strategytdexp3 : public strategytdbase
{
    // Class like strategytdexp, where we still keep the symmetry assumptions, but
    // add an extra input for each player representing whether it's that player's turn.
    // Also always uses lambda=0. 
public:
    // initialize with random weights
    strategytdexp3();
    // initialize with the tdexp2 network
    strategytdexp3( const strategytdexp& baseStrat );
    // initialize using saved weights; if loadExp is true it assumes that the weights
    // are from a tdexp network, otherwise from a proper tdexp3 network
    strategytdexp3( const string& pathEnd, const string& fileSuffix, bool loadExp );
    virtual ~strategytdexp3() {};
    
    // boardValue returns the expected number of points the player with the dice will
    // win given the board setup. This is what the game uses to determine which move to
    // make.
    
    virtual double boardValue( const board& brd ) const;
    
    // the next set of methods is used in evaluating the neural network outputs given the
    // inputs defined by the board. 
    
    vector<double> getInputValues( const board& brd, bool holdDice ) const;
    vector<double> getMiddleValues( const vector<double>& inputs ) const;
    double getOutputProbValue( const vector<double>& middles ) const;
    double getOutputGammonWinValue( const vector<double>& middles, const board& brd ) const;
    double getOutputGammonLossValue( const vector<double>& middles, const board& brd ) const;
    
    // the next set of methods is used to introspect on the state of the weights
    
    vector<double> getOutputProbWeights() const;
    vector<double> getOutputGammonWinWeights() const;
    
    vector< vector<double> > getMiddleWeights() const;
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights. needsUpdate tells the game whether it needs to do an update.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    
protected:
    void updateLocal( const board& oldBoard, const board& newBoard, bool holdDice );
    void setupExp( const strategytdexp& baseStrat );
    
    vector<double> outputProbWeights;
    vector<double> outputGammonWinWeights;
    
    vector< vector<double> > middleWeights;
    
    // define some vectors to hold derivative values in the update function. They're
    // overwritten each update, but we allocate memory for the vectors at startup
    // so we don't do it each iteration.
    
    vector<double> probDerivs;                      // derivs of output prob node to its weights vs middle node values
    vector<double> gamWinDerivs;                    // derivs of output gammon win node to its weights vs middle node values
    vector< vector<double> > probInputDerivs;       // deriv of the prob win output to all the middle->input weights
    vector< vector<double> > gamWinInputDerivs;     // deriv of the gammon win output to all the middle->input weights
};

#endif
