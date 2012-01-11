//
//  strategytdorig.h
//  bgtd
//
//  Created by Mark Higgins on 12/10/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategytdorig_h
#define bgtd_strategytdorig_h

#include "strategytdbase.h"

class strategytdorig : public strategytdbase
{
    // TD gammon strategy with one output node that represents the probability of
    // any kind of a win. It has a sigmoid activation function that depends on the
    // weighted sum of middle node values plus a bias weight. Middle nodes have
    // sigmoid activation functions that depend on the weighted sum of input value,
    // with no bias weight.
    // The inputs do not contain a flag for whose turn it is, and no symmetry 
    // assumptions are made.
    
public:
    strategytdorig();
    strategytdorig( int nMiddle );
    virtual ~strategytdorig();
    
    virtual double boardValue( const board& brd ) const;
    
    vector<double> getInputValues( const board& brd ) const;
    vector<double> getMiddleValues( const vector<double>& inputs ) const;
    double getOutput( const vector<double>& middles ) const;
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    
private:
    void setup();
    
    vector<double> outputWeights;           // weights of the output node to each of the hidden nodes
    vector< vector<double> > middleWeights; // weights of each of the middle nodes to each of the input nodes
};

#endif
