//
//  strategypimm.h
//  bgtd
//
//  Created by Mark Higgins on 3/21/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategypimm_h
#define bgtd_strategypimm_h

#include "strategy.h"

class strategypimm : public strategy
{
    // strategy from Kevin Pimm. Like PubEval in that the evaluation function is
    // just a number and doesn't represent a probability of win etc.
    
public:
    strategypimm() {};
    virtual ~strategypimm() {};
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 );
};

#endif
