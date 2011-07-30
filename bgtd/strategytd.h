//
//  strategytd.h
//  bgtd
//
//  Created by Mark Higgins on 7/24/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategytd_h
#define bgtd_strategytd_h

#include "strategy.h"

class strategytd : public strategy
{
    // TD gammon strategy - sets up a little neural network for predicting the
    // probability of winning a game. Can work in training mode where the weights
    // in the net are updated as the game progresses.
    //
    // A relatively simple net that has only one output node, reflecting the probability
    // of winning the game. So it does not optimize for gammons.
    //
    // There are 196 input nodes that represent the board layout (always from the perspective
    // of the person moving, so a bit different than the original Tesauro version which kept
    // the perspective the same and added two input nodes to represent whose turn it is).
    
public:
    strategytd();
    strategytd( int nMiddle );
    strategytd( const vector<double>& outputWeights, const vector< vector<double> >& middleWeights, const vector<double>& outputTraces, const vector< vector<double> >& middleTraces, double alpha, double beta, double lambda );
    strategytd( const strategytd& otherStrat );
    virtual ~strategytd();
    
    virtual double boardValue( const board& brd ) const;
    
    vector<double> getInputValues( const board& brd ) const;
    vector<double> getMiddleValues( const vector<double>& inputs ) const;
    double getOutput( const vector<double>& middles ) const;
    
    int nMiddle;   // number of middle nodes
    
    // then stuff used for learning
    
    bool learning; // true, it updates weights as it goes; false, it doesn't
    double alpha;  // how far we move the weights to get them closer to the goal
    double beta;   // sim for middle weights
    double lambda; // how much we weight past estimates of target - btw 0 and 1 (0 means no memory)
    
    vector<double> getOutputWeights() const;
    vector< vector<double> > getMiddleWeights() const;
    vector<double> getOutputTraces() const;
    vector< vector<double> > getMiddleTraces() const;
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    
private:
    void setup();
    
    vector<double> outputWeights;           // weights of the output node to each of the hidden nodes
    vector< vector<double> > middleWeights; // weights of each of the middle nodes to each of the input nodes
    
    vector<double> outputTraces;            // eligibility traces for the weights in the output node
    vector< vector<double> > middleTraces;  // eligibility traces for the weights in each of the middle nodes
};

#endif
