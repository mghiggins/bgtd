//
//  strategytdmult.h
//  bgtd
//
//  Created by Mark Higgins on 10/20/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategytdmult_h
#define bgtd_strategytdmult_h

#include <vector>
#include <string>
#include <hash_map.h>
#include "strategytdbase.h"

class strategytdmult : public strategytdbase
{
    // A generalized TD strategy. Contains multiple networks for different game states
    // plus uses a bearoff database for bearoff races. Cubeless.
    
public:
    strategytdmult();
    strategytdmult( int nMiddle );
    strategytdmult( const string& pathEnd, const string& fileSuffix );
    virtual ~strategytdmult() {};
    
    virtual double boardValue( const board& brd ) const;
    
    string evaluator( const board& brd) const;
    /*
    vector<double> getInputValues( const board& brd ) const;
    vector<double> getMiddleValues( const vector<double>& inputs, const string& net ) const;
    double getOutputProbValue( const vector<double>& middles, const string& net ) const;
    double getOutputGammonValue( const vector<double>& middles, const string& net ) const;
    double getOutputGammonLossValue( const vector<double>& middles, const string& net ) const;
    
    vector<double> getOutputProbWeights( const string& net ) const;
    vector<double> getOutputGammonWeights( const string& net ) const;
    vector< vector<double> > getMiddleWeights( const string& net ) const;
    */
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    
private:
    void setup();
    
    hash_map<string, vector<double> >           outputProbWeights;    // weights of the 1st output node (prob of player win) to the middle nodes, keyed by network
    hash_map<string, vector<double> >           outputGammonWeights;  // weights of the 2nd output node (cond prob of player gammon win) to the middle nodes, keyed by network
    hash_map<string, vector< vector<double> > > middleWeights;        // weights of the middle nodes to the input nodes, keyed by network
};

#endif


