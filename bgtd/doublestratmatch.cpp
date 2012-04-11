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

#include <cmath>
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
    // no double allowed if the player can already win at this level
    
    int n = b.perspective() == 0 ? currMatch->getTarget() - currMatch->playerScore() : currMatch->getTarget() - currMatch->opponentScore();
    int m = b.perspective() == 0 ? currMatch->getTarget() - currMatch->opponentScore() : currMatch->getTarget() - currMatch->playerScore();
    
    if( n-cube<=0 ) return false;
    
    // automatic double case: if the opponent is definitely going to win the match on a win
    // the player should double.
    
    if( m-cube<=0 and n-cube>0 ) return true;
    
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
    
    // the doubled equity is never more than the pass equity. Respect the Crawford rule.
    
    double singleWinME;
    if( n == 1 )
        singleWinME = 1;
    else if( m == 1 )
        singleWinME = -MET.matchEquityPostCrawford(n-cube);
    else
        singleWinME = MET.matchEquity(n-cube, m);
    
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
    
    // calculate the equity we'd give up if we passed. Respect the Crawford rule.
    
    int n = b.perspective() == 0 ? currMatch->getTarget() - currMatch->playerScore() : currMatch->getTarget() - currMatch->opponentScore();
    int m = b.perspective() == 0 ? currMatch->getTarget() - currMatch->opponentScore() : currMatch->getTarget() - currMatch->playerScore();
    
    double singleLossME;
    if( n == 1 )
        singleLossME = MET.matchEquityPostCrawford(m-cube);
    else if( m == 1 )
        singleLossME = -1;
    else
        singleLossME = MET.matchEquity(n, m-cube);

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
    
    bool noDoubleAllowed=false;
    
    // Crawford game - no doubling allowed
    
    if( ( n == 1 or m == 1 ) and !currMatch->doneCrawford ) noDoubleAllowed = true;
    
    // cube already at max
    
    if( ( perspective == cubeOwner and n-cube<=0 ) or ( perspective != cubeOwner and m-cube<=0 ) ) noDoubleAllowed = true;
    
    // if this is an automatic double, return that equity (regardless of whether it's dead or live). If the player owns the cube
    // and the cube is such that the opponent will win the match on any kind of win, the player will always double. If he loses
    // doubled he still just loses the match; but if he wins doubled he's twice as close to the target.
    
    if( !noDoubleAllowed and ( ( cubeOwner != perspective and n-cube<=0 and m-cube>0 ) or ( cubeOwner == perspective and m-cube<=0 and n-cube>0 ) ) )
        return equityInterpFn( probs, perspective, 2*cube, 1-cubeOwner, isDead );
    
    // figure out the match equities on different kinds of wins and losses if we
    // do take/cash. Respect the Crawford rule.
    
    double singleWinME, gammonWinME, bgWinME, singleLossME, gammonLossME, bgLossME;
    
    if( n==1 )
    {
        singleWinME  = 1;
        gammonWinME  = 1;
        bgWinME      = 1;
        singleLossME = MET.matchEquityPostCrawford(m-cube);
        gammonLossME = MET.matchEquityPostCrawford(m-2*cube);
        bgLossME     = MET.matchEquityPostCrawford(m-3*cube);
    }
    else if( m == 1 )
    {
        singleWinME  = -MET.matchEquityPostCrawford(n-cube);
        gammonWinME  = -MET.matchEquityPostCrawford(n-2*cube);
        bgWinME      = -MET.matchEquityPostCrawford(n-3*cube);
        singleLossME = -1;
        gammonLossME = -1;
        bgLossME     = -1;
    }
    else
    {
        singleWinME  = MET.matchEquity(n-1*cube, m);
        gammonWinME  = MET.matchEquity(n-2*cube, m);
        bgWinME      = MET.matchEquity(n-3*cube, m);
        singleLossME = MET.matchEquity(n, m-1*cube);
        gammonLossME = MET.matchEquity(n, m-2*cube);
        bgLossME     = MET.matchEquity(n, m-3*cube);
    }
    
    // we'll get equity as a fn of prob of win by assuming that the ratios of gammon and backgammon
    // probs to the relevant winning/losing prob stay fixed.
    
    double W = probs.probWin == 0 ? 1 : ( ( probs.probWin - probs.probGammonWin ) * singleWinME + ( probs.probGammonWin - probs.probBgWin ) * gammonWinME + probs.probBgWin * bgWinME ) / probs.probWin;
    double L = 1-probs.probWin == 0 ? 1 : -( ( 1 - probs.probWin - probs.probGammonLoss ) * singleLossME + ( probs.probGammonLoss - probs.probBgLoss ) * gammonLossME + probs.probBgLoss * bgLossME ) / ( 1 - probs.probWin );
    
    // if we're calculating the dead cube limit, or the cube is legitimately dead because we can't double
    // anymore, return the dead cube equity.
    
    bool returnDead = noDoubleAllowed or isDead;
    
    if( returnDead )
        return interpMEdata( 0, 1, -L, W, W, L );
    
    // if the cube is centered or the player here owns the cube and still can double, the upper end of the
    // range is the cash point; otherwise the cash point is 100% prob and cash equity
    // is the appropriately-weighted win equity
    
    double cashPoint, cashME;
    
    if( ( cube == 1 or perspective == cubeOwner ) and n-cube > 0 )
    {
        if( n == 1 )
            cashME = 1;
        else if( m == 1 )
            cashME = -MET.matchEquityPostCrawford(n-cube);
        else
            cashME = MET.matchEquity(n-cube, m);
        
        // need to get the match equity fn for a state where the cube is doubled and owned by the 
        // opponent; find where that matches the cash match equity.
        
        interpMEdata data2( equityInterpFn( probs, perspective, cube*2, 1-perspective, isDead ) );
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
        if( n == 1 )
            takeME = MET.matchEquityPostCrawford(m-cube);
        else if( m == 1 )
            takeME = -1;
        else
            takeME = MET.matchEquity(n, m-cube);
        
        // find the prob such that the match equity equals the take equity in a state 
        // where the cube is doubled and the player owns it
        
        interpMEdata data2( equityInterpFn( probs, perspective, cube*2, perspective, isDead ) );
        takePoint = data2.solve( takeME );
    }
    else
    {
        takePoint = 0;
        takeME    = -L;
    }
    
    return interpMEdata( takePoint, cashPoint, takeME, cashME, W, L );
}