//
//  strategytdexp2.h
//  bgtd
//
//  Created by Mark Higgins on 8/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategytdexp2_h
#define bgtd_strategytdexp2_h

#include "strategytdbase.h"
#include "strategytdexp.h"

class strategytdexp2 : public strategytdbase
{
    // Class like strategytdexp, in that it tracks the probability of gammon as well as
    // the probability of a win. The difference is that it drops the relationships between
    // weights etc that comes from the assumption of symmetry on board flip, because that
    // symmetry is only approximate. We also assume lambda=0 to avoid having to bother with
    // traces (since mostly I've been choosing lambda=0 in practice). It has three output
    // nodes: probability of (any kind of) win; probability of a gammon win conditioned on
    // a regular win; and probability of a gammon loss conditioned on a loss.
public:
    // initialize with the regular tdexp network
    strategytdexp2( const strategytdexp& baseStrat );
    // initialize using saved weights; if loadExp is true it assumes that the weights
    // are from a regular tdexp network, otherwise from a proper tdexp2 network
    strategytdexp2( const string& pathEnd, const string& fileSuffix, bool loadExp );
    virtual ~strategytdexp2() {};
    
    // boardValue returns the expected number of points the player with the dice will
    // win given the board setup. This is what the game uses to determine which move to
    // make.
    
    virtual double boardValue( const board& brd ) const;
    
    // the next set of methods is used in evaluating the neural network outputs given the
    // inputs defined by the board.
    
    vector<double> getInputValues( const board& brd ) const;
    vector<double> getMiddleValues( const vector<double>& inputs ) const;
    double getOutputProbValue( const vector<double>& middles ) const;
    double getOutputGammonWinValue( const vector<double>& middles, const board& brd ) const;
    double getOutputGammonLossValue( const vector<double>& middles, const board& brd ) const;
    
    // the next set of methods is used to introspect on the state of the weights
    
    vector<double> getOutputProbWeights() const;
    vector<double> getOutputGammonWinWeights() const;
    vector<double> getOutputGammonLossWeights() const;
    
    vector< vector<double> > getMiddleWeights() const;
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights. needsUpdate tells the game whether it needs to do an update.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    void updateLocal( const board& oldBoard, const board& newBoard, bool firstTime );
    
protected:
    void setupExp( const strategytdexp& baseStrat );
    
    vector<double> outputProbWeights;
    vector<double> outputGammonWinWeights;
    vector<double> outputGammonLossWeights;
    
    vector< vector<double> > middleWeights;
    
    // define some vectors to hold derivative values in the update function. They're
    // overwritten each update, but we allocate memory for the vectors at startup
    // so we don't do it each iteration.
    
    vector<double> probDerivs;                      // derivs of output prob node to its weights vs middle node values
    vector<double> gamWinDerivs;                    // derivs of output gammon win node to its weights vs middle node values
    vector<double> gamLossDerivs;                   // derivs of output gammon loss node to its weights vs middle node values
    vector< vector<double> > probInputDerivs;       // deriv of the prob win output to all the middle->input weights
    vector< vector<double> > gamWinInputDerivs;     // deriv of the gammon win output to all the middle->input weights
    vector< vector<double> > gamLossInputDerivs;    // deriv of the gammon loss output to all the middle->input weights
};

#endif
