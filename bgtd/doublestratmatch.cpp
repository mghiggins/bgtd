/*****************
 Copyright 2011, 2012 Mark Higgins
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 *****************/

#include "doublestratmatch.h"
#include "doublefns.h"

doublestratmatch::doublestratmatch( const string& METFileName, double cubeLifeIndex )
: MET(METFileName), currMatch(0), cubeLifeIndex(cubeLifeIndex)
{
}

doublestratmatch::doublestratmatch( const matchequitytable& MET, double cubeLifeIndex )
: MET(MET), currMatch(0), cubeLifeIndex(cubeLifeIndex)
{
}

bool doublestratmatch::offerDouble( strategyprob& strat, const board& b, int cube )
{
    // get the various game probabilities. For this we want game probabilities
    // before the dice are thrown. The strategy's boardProbabilities returns the
    // game probs *after* the dice are thrown, so we need to flip the board around,
    // get the probs, then flip their perspective back.
    
    gameProbabilities probs(boardProbabilities(strat, b, true));
    
    // calculate the dead cube equity - assuming there's no value to holding the cube
    
    interpMEdata deadME1( equityInterpFn( probs, b.perspective(), cube, b.perspective(), true ) );
    interpMEdata deadME2( equityInterpFn( probs, b.perspective(), 2*cube, 1-b.perspective(), true ) );
    
    // get the live cube match equity for this cube level and for a doubled cube. Current cube this player
    // owns the cube; doubled the opponent owns it.
    
    interpMEdata liveME1( equityInterpFn( probs, b.perspective(), cube, b.perspective(), false ) );
    interpMEdata liveME2( equityInterpFn( probs, b.perspective(), 2*cube, 1-b.perspective(), false ) );
    
    // double if that equity is larger.
    
    double equityDouble   = cubeLifeIndex * liveME2(probs.probWin) + ( 1 - cubeLifeIndex ) * deadME2(probs.probWin);
    double equityNoDouble = cubeLifeIndex * liveME1(probs.probWin) + ( 1 - cubeLifeIndex ) * deadME1(probs.probWin);
    
    // the doubled equity is never more than the pass equity
    
    int n = b.perspective() == 0 ? currMatch->getTarget() - currMatch->playerScore() : currMatch->getTarget() - currMatch->opponentScore();
    int m = b.perspective() == 0 ? currMatch->getTarget() - currMatch->opponentScore() : currMatch->getTarget() - currMatch->playerScore();
    
    double singleWinME( MET.matchEquity(n-cube, m) );
    
    if( equityDouble > singleWinME ) equityDouble = singleWinME;
    
    // Leave a little threshold
    
    return equityDouble > equityNoDouble + 1e-6;
}

bool doublestratmatch::takeDouble( strategyprob& strat, const board& b, int cube )
{
    // need probabilities when the opponent holds the dice; that's what the strategy's
    // boardProbabilities returns so just use that.
    
    gameProbabilities probs( boardProbabilities(strat,b,false) );
    
    // calculate the equity at the doubled cube level assuming the player holds the dice
    
    interpMEdata deadME( equityInterpFn( probs, b.perspective(), 2*cube, b.perspective(), true ) );
    interpMEdata liveME( equityInterpFn( probs, b.perspective(), 2*cube, b.perspective(), false ) );
    
    double equityDouble = cubeLifeIndex*liveME(probs.probWin) + (1-cubeLifeIndex)*deadME(probs.probWin);
    
    // calculate the equity we'd give up if we passed
    
    int n = b.perspective() == 0 ? currMatch->getTarget() - currMatch->playerScore() : currMatch->getTarget() - currMatch->opponentScore();
    int m = b.perspective() == 0 ? currMatch->getTarget() - currMatch->opponentScore() : currMatch->getTarget() - currMatch->playerScore();
    
    double singleLossME( MET.matchEquity(n, m-cube) );

    return equityDouble > singleLossME;
}

double doublestratmatch::equity( strategyprob& strat, const board& b, int cube, bool ownsCube, bool holdsDice )
{
    gameProbabilities probs( boardProbabilities(strat, b, holdsDice) );
    int cubeOwner = ownsCube ? b.perspective() : 1-b.perspective();
    interpMEdata deadME( equityInterpFn(probs, b.perspective(), cube, cubeOwner,true) );
    interpMEdata liveME( equityInterpFn(probs, b.perspective(), cube, cubeOwner,false) );
    
    return cubeLifeIndex*liveME(probs.probWin) + (1-cubeLifeIndex)*deadME(probs.probWin);
}

interpMEdata doublestratmatch::equityInterpFn( const gameProbabilities& probs, int perspective, int cube, int cubeOwner, bool isDead, int nOverride, int mOverride )
{
    if( currMatch == 0 ) throw string( "Set the match first" );
    
    // look at the match to figure out the score
    
    int n, m; // n is the # of games away for this guy, m for the other
    
    if( nOverride == 0 )
        n = perspective == 0 ? currMatch->getTarget() - currMatch->playerScore() : currMatch->getTarget() - currMatch->opponentScore();
    else
        n = nOverride;
    if( mOverride == 0 )
        m = perspective == 0 ? currMatch->getTarget() - currMatch->opponentScore() : currMatch->getTarget() - currMatch->playerScore();
    else
        m = mOverride;
    
    // figure out the match equities on different kinds of wins and losses if we
    // do take/cash
    
    double singleWinME  = MET.matchEquity(n-1*cube, m);
    double gammonWinME  = MET.matchEquity(n-2*cube, m);
    double bgWinME      = MET.matchEquity(n-3*cube, m);
    double singleLossME = MET.matchEquity(n, m-1*cube);
    double gammonLossME = MET.matchEquity(n, m-2*cube);
    double bgLossME     = MET.matchEquity(n, m-3*cube);
    
    // we'll get equity as a fn of prob of win by assuming that the ratios of gammon and backgammon
    // probs to the relevant winning/losing prob stay fixed.
    
    double W = probs.probWin == 0 ? 1 : ( ( probs.probWin - probs.probGammonWin ) * singleWinME + ( probs.probGammonWin - probs.probBgWin ) * gammonWinME + probs.probBgWin * bgWinME ) / probs.probWin;
    double L = 1-probs.probWin == 0 ? 1 : -( ( 1 - probs.probWin - probs.probGammonLoss ) * singleLossME + ( probs.probGammonLoss - probs.probBgLoss ) * gammonLossME + probs.probBgLoss * bgLossME ) / ( 1 - probs.probWin );
    
    if( isDead )
        return interpMEdata( 0, 1, -L, W, W, L );
    
    // if the cube is centered or the player here owns the cube and still can double, the upper end of the
    // range is the cash point; otherwise the cash point is 100% prob and cash equity
    // is the appropriately-weighted win equity
    
    double cashPoint, cashME;
    
    if( ( cube == 1 or perspective == cubeOwner ) and n-cube > 0 )
    {
        cashME = MET.matchEquity(n-cube, m);
        
        // need to get the match equity fn for a state where the cube is doubled and owned by the 
        // opponent; find where that matches the cash match equity.
        
        interpMEdata data2( equityInterpFn( probs, perspective, cube*2, 1-perspective, W, L ) );
        cashPoint = data2.solve( cashME );
    }
    else
    {
        cashPoint = 1.;
        cashME    = W;
    }
    
    // if the cube is centered or the opponent owns the cube and can still double, the lower end of the range is
    // the take point; otherwise the take point is 0% prob and take equity is the 
    // appropriately-weighted loss equity
    
    double takePoint, takeME;
    
    if( ( cube == 1 or perspective != cubeOwner ) and m-cube>0 )
    {
        takeME = MET.matchEquity(n, m-cube);
        
        // find the prob such that the match equity equals the take equity in a state 
        // where the cube is doubled and the player owns it
        
        interpMEdata data2( equityInterpFn( probs, perspective, cube*2, perspective, W, L ) );
        takePoint = data2.solve( takeME );
    }
    else
    {
        takePoint = 0;
        takeME    = -L;
    }
    
    return interpMEdata( takePoint, cashPoint, takeME, cashME, W, L );
}