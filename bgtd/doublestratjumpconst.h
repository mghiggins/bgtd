//
//  doublestratjumpconst.h
//  bgtd
//
//  Created by Mark Higgins on 3/26/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_doublestratjumpconst_h
#define bgtd_doublestratjumpconst_h

#include "doublestrat.h"
#include "strategyprob.h"

class doublestratjumpconst : public doublestrat
{
    // money game doubling strategy that uses the jump model for
    // probability of win evolution. This version uses a constant
    // jump volatility. Either the linear or nonlinear approximation
    // can be used.
    
public:
    doublestratjumpconst( double jumpVol, bool useLinear ) : jumpVol(jumpVol), useLinear(useLinear) {};
    virtual ~doublestratjumpconst() {};
    
    double getJumpVol() const { return jumpVol; };
    void setJumpVol( double newVol ) { if( newVol < 0 ) throw string( "Jump volatility cannot be negative" ); jumpVol=newVol; };
    
    virtual double equity( strategyprob& strat, const board& b, int cube, bool ownsCube, bool holdsDice );
    double equityLinear( strategyprob& strat, const board& b, int cube, bool ownsCube, bool holdsDice );
    double equityNonlinear( strategyprob& strat, const board& b, int cube, bool ownsCube, bool holdsDice );
    
    bool useLinear;
    
private:
    double jumpVol;
};


#endif
