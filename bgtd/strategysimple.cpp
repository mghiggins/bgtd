//
//  strategysimple.cpp
//  bgtd
//
//  Created by Mark Higgins on 7/23/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "strategysimple.h"
#include "randomc.h"

strategySimple::strategySimple()
{
    singletonWeight = 50;
    towerWeight     = 10;
    runWeight       = 50;
}

strategySimple::strategySimple( double singletonWeight, double towerWeight, double runWeight )
{
    this->singletonWeight = singletonWeight;
    this->towerWeight     = towerWeight;
    this->runWeight       = runWeight;
}

double strategySimple::boardValue( const board& brd ) const
{
    // start with pips^2
    
    double val=brd.pipsq();
    
    // dislike singletons, giant towers; like blocked-off spots next each other
    
    int check;
    int run=0, maxRun=0;
    
    for( int i=0; i<24; i++ )
    {
        check = brd.checker(i);
        if( check == 1 )
            val -= singletonWeight;
        if( check > 5 )
            val -= towerWeight * ( check - 5 );
        if( check > 1 )
        {
            run ++;
            if( run > maxRun ) maxRun = run;
        }
        else
            run = 0;
    }
    
    val += runWeight * ( maxRun - 1 );
    
    return val;
}

strategyRandom::strategyRandom( int seed )
{
    rng = new CRandomMersenne(seed);
}

strategyRandom::~strategyRandom()
{
    delete rng;
}

double strategyRandom::boardValue( const board& brd ) const
{
    return rng->IRandom( 0, 100 );
}