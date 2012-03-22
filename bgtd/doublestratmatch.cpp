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

doublestratmatch::doublestratmatch( strategyprob& strat, const string& METFileName )
: strat(strat), MET(METFileName), currMatch(0)
{
}

doublestratmatch::doublestratmatch( strategyprob& strat, const matchequitytable& MET )
: strat(strat), MET(MET), currMatch(0)
{
}

bool doublestratmatch::offerDouble( const board& b, int cube )
{
    // get the various game probabilities. For this we want game probabilities
    // before the dice are thrown. The strategy's boardProbabilities returns the
    // game probs *after* the dice are thrown, so we need to flip the board around,
    // get the probs, then flip their perspective back.
    
    board fb(b);
    fb.setPerspective(1-b.perspective());
    gameProbabilities probs( strat.boardProbabilities(fb).flippedProbs() );
    
    // if the cubeless equity is greater than the equity we'd get if the
    // opponent passed on a double, it's too good to double (since we ignore
    // the value from owning the cube).
    
    int n = b.perspective() == 0 ? currMatch->getTarget() - currMatch->playerScore() : currMatch->getTarget() - currMatch->opponentScore();
    int m = b.perspective() == 0 ? currMatch->getTarget() - currMatch->opponentScore() : currMatch->getTarget() - currMatch->playerScore();
    
    double singleWinME  = MET.matchEquity(n-1*cube, m);
    double gammonWinME  = MET.matchEquity(n-2*cube, m);
    double bgWinME      = MET.matchEquity(n-3*cube, m);
    double singleLossME = MET.matchEquity(n, m-1*cube);
    double gammonLossME = MET.matchEquity(n, m-2*cube);
    double bgLossME     = MET.matchEquity(n, m-3*cube);
    
    double cubelessME = ( probs.probWin - probs.probGammonWin ) * singleWinME + ( probs.probGammonWin - probs.probBgWin ) * gammonWinME + probs.probBgWin * bgWinME
                      + ( 1 - probs.probWin - probs.probGammonLoss ) * singleLossME + ( probs.probGammonLoss - probs.probBgLoss ) * gammonLossME + probs.probBgLoss * bgLossME;
    double cashME = singleWinME;
    if( cubelessME > cashME ) return false; // too good to double
    
    // get the match equity for this cube level and for a doubled cube. Current cube this player
    // owns the cube; doubled the opponent owns it.
    
    interpMEdata data1( equityInterpFn( probs, b.perspective(), cube, b.perspective() ) );
    interpMEdata data2( equityInterpFn( probs, b.perspective(), 2*cube, 1-b.perspective() ) );
    
    // double if that equity is larger. Leave a little threshold, since we want to double
    // if we're past the cash point.
    
    double equityDouble   = data2(probs.probWin);
    double equityNoDouble = data1(probs.probWin);
    
    // the doubled equity is never more than the pass equity
    
    if( equityDouble > singleWinME ) equityDouble = singleWinME;
    
    return equityDouble > equityNoDouble - 1e-6;
}

bool doublestratmatch::takeDouble( const board& b, int cube )
{
    // need probabilities when the opponent holds the dice; that's what the strategy's
    // boardProbabilities returns so just use that.
    
    gameProbabilities probs( strat.boardProbabilities(b) );
    interpMEdata data( equityInterpFn( probs, b.perspective(), cube, 1-b.perspective() ) );
    
    // take if the prob of win is above the take point
    
    return probs.probWin >= data.takePoint;
}

interpMEdata doublestratmatch::equityInterpFn( const gameProbabilities& probs, int perspective, int cube, int cubeOwner, int nOverride, int mOverride )
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
    // do take
    
    double singleWinME  = MET.matchEquity(n-2*cube, m);
    double gammonWinME  = MET.matchEquity(n-4*cube, m);
    double bgWinME      = MET.matchEquity(n-6*cube, m);
    double singleLossME = MET.matchEquity(n, m-2*cube);
    double gammonLossME = MET.matchEquity(n, m-4*cube);
    double bgLossME     = MET.matchEquity(n, m-6*cube);
    
    // we'll get equity as a fn of prob of win by assuming that the ratios of gammon and backgammon
    // probs to the relevant winning/losing prob stay fixed.
    
    double W = probs.probWin == 0 ? 1 : ( ( probs.probWin - probs.probGammonWin ) * singleWinME + ( probs.probGammonWin - probs.probBgWin ) * gammonWinME + probs.probBgWin * bgWinME ) / probs.probWin;
    double L = 1-probs.probWin == 0 ? 1 : -( ( 1 - probs.probWin - probs.probGammonLoss ) * singleLossME + ( probs.probGammonLoss - probs.probBgLoss ) * gammonLossME + probs.probBgLoss * bgLossME ) / ( 1 - probs.probWin );
    
    // if the cube is centered or the player here owns the cube, the upper end of the
    // range is the cash point; otherwise the cash point is 100% prob and cash equity
    // is the appropriately-weighted win equity
    
    double cashPoint, cashME;
    
    if( cube == 1 or perspective == cubeOwner )
    {
        cashME = MET.matchEquity(n-cube, m);
        cashPoint = ( cashME + L ) / ( W + L );
    }
    else
    {
        cashPoint = 1.;
        cashME    = W;
    }
    
    // if the cube is centered or the opponent owns the cubve, the lower end of the range is
    // the take point; otherwise the take point is 0% prob and take equity is the 
    // appropriately-weighted loss equity
    
    double takePoint, takeME;
    
    if( cube == 1 or perspective != cubeOwner )
    {
        takeME = MET.matchEquity(n, m-cube);
        takePoint = ( takeME + L ) / ( W + L );
    }
    else
    {
        takePoint = 0;
        takeME    = -L;
    }
    
    return interpMEdata( takePoint, cashPoint, takeME, cashME );
}