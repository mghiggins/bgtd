//
//  strategytdorigbg.h
//  bgtd
//
//  Created by Mark Higgins on 1/16/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategytdorigbg_h
#define bgtd_strategytdorigbg_h

#include "strategytdoriggam.h"

class strategytdorigbg : public strategytdoriggam
{
    // An extension of the "gammon" version of the original TD strategy
    // to backgammon outputs. So it has five outputs: prob of any win;
    // prob of gammon loss (incl backgammon); prob of gammon win (incl backgammon);
    // prob of backgammon loss; and prob of backgammon win.
public:
    strategytdorigbg();
    strategytdorigbg( int nMiddle );
    strategytdorigbg( const string& subPath, const string& filePrefix );
    virtual ~strategytdorigbg() {};
    
    virtual double boardValue( const board& brd ) const;

    double getOutputBackgammon( const vector<double>& middles ) const;
    double getOutputBackgammonLoss( const vector<double>& middles ) const;

    // write and read weights
    
    void writeWeights( const string& filePrefix ) const;
    void loadWeights( const string& subPath, const string& filePrefix );
    
    virtual void update( const board& oldBoard, const board& newBoard );

protected:
    void setup();
    void setupBgWeights();
    
    vector<double> outputBgWeights;     // weights of the prob of backgammon win node to each of the hidden nodes
    vector<double> outputBgLossWeights; // weights of the prob of backgammon loss node to each of the hidden nodes
};

#endif
