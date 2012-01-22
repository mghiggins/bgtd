//
//  strategypubeval.h
//  bgtd
//
//  Created by Mark Higgins on 7/29/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategypubeval_h
#define bgtd_strategypubeval_h

#include <vector>
#include "strategy.h"

class strategyPubEval : public strategy
{
public:
    strategyPubEval();
    virtual ~strategyPubEval();
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 ) const;
    virtual hash_map<string,int> boardContext( const board& brd ) const;
    
private:
    vector<double> weightsContact;
    vector<double> weightsRace;
};

#endif
