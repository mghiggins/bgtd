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
    // plus uses a bearoff database for bearoff races. Cubeless. No symmetry assumption
    // and no input for whose turn it is.
    
public:
    strategytdmult();
    strategytdmult( int nMiddle );
    strategytdmult( const string& path, const string& filePrefix, bool useShotProbInput=false, bool randomShotInput=true );
    virtual ~strategytdmult() {};
    
    virtual double boardValue( const board& brd ) const;
    
    string evaluator( const board& brd) const;
    
    int nInputs( const string& netName ) const;
    vector<double> getInputValues( const board& brd, const string& netName ) const;
    vector<double> getBaseInputValues( const board& brd ) const;
    vector<double> getMiddleValues( const vector<double>& inputs, const string& netName ) const;
    double getOutputProbValue( const vector<double>& middles, const string& netName ) const;
    double getOutputGammonValue( const vector<double>& middles, const string& netName ) const;
    double getOutputGammonLossValue( const vector<double>& middles, const string& netName ) const;
    double getOutputBackgammonValue( const vector<double>& middles, const string& netName ) const;
    double getOutputBackgammonLossValue( const vector<double>& middles, const string& netName ) const;
    
    double bearoffValue( const board& brd ) const;
    double bearoffProbabilityWin( const board& brd ) const;
    double bearoffProbabilityGammon( const board& brd ) const;
    double bearoffProbabilityGammonLoss( const board& brd ) const;
    double doneValue( const board& brd ) const;
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    
    // writeWeights writes the weights for each network to files (one per network, suffixed with the network name)
    
    void writeWeights( const string& filePrefix ) const;
    void loadWeights( const string& path, const string& filePrefix, bool randomShotInput );
    
    // define some parameters
    
    bool useShotProbInput;
    
private:
    void setup();
    void setupRandomWeights( int nMiddle );
    
    // network weights
    
    hash_map<string, vector<double> >           outputProbWeights;    // weights of the 1st output node (prob of player win) to the middle nodes, keyed by network
    hash_map<string, vector<double> >           outputGammonWeights;  // weights of the 2nd output node (prob of player gammon win) to the middle nodes, keyed by network
    hash_map<string, vector<double> >           outputGammonLossWeights; // sim for gammon loss output
    hash_map<string, vector<double> >           outputBgWeights;      // weights for backgammon win output
    hash_map<string, vector<double> >           outputBgLossWeights;  // weights for backgammon loss output
    hash_map<string, vector< vector<double> > > middleWeights;        // weights of the middle nodes to the input nodes, keyed by network
    
    // parameters for the bearoff db
    
    int bearoffNPnts;
    int bearoffNCheckers;
};

#endif


