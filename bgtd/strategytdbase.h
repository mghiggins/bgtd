//
//  strategytdbase.h
//  bgtd
//
//  Created by Mark Higgins on 8/2/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategytdbase_h
#define bgtd_strategytdbase_h

#include "strategy.h"

class strategytdbase : public strategy
{
public:
    strategytdbase() {};
    virtual ~strategytdbase() {};
    
    int nMiddle;   // number of middle nodes
    
    // then stuff used for learning
    
    bool learning; // true, it updates weights as it goes; false, it doesn't
    
    double alpha;  // how far we move the output weights to get them closer to the goal
    double beta;   // sim for middle weights
    double lambda; // how much we weight past estimates of target - btw 0 and 1 (0 means no memory)
};

#endif
