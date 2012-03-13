//
//  strategymult.h
//  bgtd
//
//  Created by Mark Higgins on 3/2/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategymult_h
#define bgtd_strategymult_h

#include "fann.h"
#include "strategytdbase.h"

class strategymult : public strategytdbase
{
    // Multiple network strategy, capable of TD learning or supervised learning.
    // Uses FANN for the neural network.
    
public:
    strategymult();
    strategymult( int nMiddle );
    strategymult( const string& path, const string& filePrefix );
    virtual ~strategymult() { fann_destroy( annContact ); fann_destroy( annCrashed ); fann_destroy( annRace ); };
    
    virtual gameProbabilities boardProbabilities( const board& brd, const hash_map<string,int>* context=0 ); 
    
    string evaluator( const board& brd) const;
    
    int nInputs( const string& netName ) const;
    vector<double> getInputValues( const board& brd, const string& netName ) const;
    vector<double> getBaseInputValues( const board& brd ) const;
    vector<double> getMiddleValues( const vector<double>& inputs, const string& netName ) const;
    
    gameProbabilities getOutputProbs( const vector<double>& inputs, const string& netName ) const;
    
    double bearoffValue( const board& brd ) const;
    double bearoffProbabilityWin( const board& brd ) const;
    double bearoffProbabilityGammon( const board& brd ) const;
    double bearoffProbabilityGammonLoss( const board& brd ) const;
    double doneValue( const board& brd ) const;
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights using TD learning. updateFromProbs does supervised learning when
    // supplied with target probabilities.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    void updateFromProbs( const board& brd, double probWin, double probGammonWin, double probGammonLoss, double probBgWin, double probBgLoss );
    
    // writeNets writes the networks to files (one per network, suffixed with the network name).
    // If singleNetName is blank it writes files for all networks' weights; otherwise it just writes files for the
    // specified network name. 
    
    void writeNets( const string& filePrefix, const string& singleNetName="" ) const;
    void loadNets( const string& path, const string& filePrefix );
    
private:
    void setup();
    
    fann * annContact;
    fann * annCrashed;
    fann * annRace;
    
    // parameters for the bearoff db
    
    int bearoffNPnts;
    int bearoffNCheckers;
};

#endif
