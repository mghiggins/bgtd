//
//  strategysimple.h
//  bgtd
//
//  Created by Mark Higgins on 7/23/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategysimple_h
#define bgtd_strategysimple_h

#include "strategy.h"
#include "randomc.h"

class strategySimple : public strategy
{
public:
    strategySimple();
    strategySimple( double singletonWeight, double towerWeight, double runWeight );
    virtual ~strategySimple() {};
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 ) const;
    
    double singletonWeight;
    double towerWeight;
    double runWeight;
};

class strategyRandom : public strategy
{
public:
    strategyRandom( int seed );
    virtual ~strategyRandom();
    
    virtual double boardValue( const board& brd ) const;
    
private:
    CRandomMersenne * rng;
};


#endif
