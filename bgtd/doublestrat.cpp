//
//  doublestrat.cpp
//  bgtd
//
//  Created by Mark Higgins on 4/5/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#include "doublestrat.h"
#include "strategyprob.h"

bool doublestrat::offerDouble( strategyprob& strat, const board& b, int cube )
{
    // compare the equity after double with the equity before. The
    // player holds the dice here.
    
    double equityDoubled = equity( strat, b, 2*cube, false, true );
    
    // if the doubled equity is more than the cash amount, cap it there;
    // the opponent will pass.
    
    if( equityDoubled > cube ) equityDoubled = cube;
    
    double equityNoDouble = equity( strat, b, cube, true, true );
    
    return equityDoubled > equityNoDouble - 1e-6; // double if it's better from an equity perspective - leave a wee margin
}

bool doublestrat::takeDouble( strategyprob& strat, const board& b, int cube )
{
    // compare the equity in the state where the player holds the cube doubled
    // with -cube. In this case the player does not hold the dice; the opponent does.
    // This is appropriate only for money games; match play needs to redefine this
    // to use eg match equity tables.
    
    double equityDoubled = equity( strat, b, 2*cube, true, false );
    return equityDoubled > -cube; // take if we're better off being doubled than passing
}

gameProbabilities doublestrat::boardProbabilities( strategyprob& strat, const board& b, bool holdsDice )
{
    if( holdsDice )
    {
        board fb(b);
        fb.setPerspective(1-b.perspective());
        return strat.boardProbabilities(fb).flippedProbs();
    }
    else
        return strat.boardProbabilities(b);
}