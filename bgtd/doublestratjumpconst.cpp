//
//  doublestratjumpconst.cpp
//  bgtd
//
//  Created by Mark Higgins on 3/26/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#include <cmath>
#include "doublestratjumpconst.h"

bool doublestratjumpconst::offerDouble( const board& b, int cube )
{
    // compare the equity after double with the equity before. The
    // player holds the dice here.
    
    double equityDoubled = equity( b, 2*cube, false, true );
    
    // if the doubled equity is more than the cash amount, cap it there;
    // the opponent will pass.
    
    if( equityDoubled > cube ) equityDoubled = cube;
    
    double equityNoDouble = equity( b, cube, true, true );
    
    return equityDoubled > equityNoDouble - 1e-6; // double if it's better from an equity perspective - leave a wee margin
}

bool doublestratjumpconst::takeDouble( const board& b, int cube )
{
    // compare the equity in the state where the player holds the cube doubled
    // with -1. In this case the player does not hold the dice; the opponent does.
    
    double equityDoubled = equity( b, 2*cube, true, false );
    return equityDoubled > -cube; // take if we're better off being doubled than passing
}

double doublestratjumpconst::equity( const board& b, int cube, bool ownsCube, bool holdsDice )
{
    if( useLinear )
        return equityLinear( b, cube, ownsCube, holdsDice );
    else
        return equityNonlinear( b, cube, ownsCube, holdsDice );
}

double doublestratjumpconst::equityLinear( const board& b, int cube, bool ownsCube, bool holdsDice )
{
    // get the game probs from the right perspective
    
    gameProbabilities probs;
    
    if( holdsDice )
    {
        board fb(b);
        fb.setPerspective(1-b.perspective());
        probs = strat.boardProbabilities(fb).flippedProbs();
    }
    else
        probs = strat.boardProbabilities(b);
    
    double W=probs.probWin==0 ? 1 : (probs.probWin + probs.probGammonWin + probs.probBgWin)/probs.probWin;
    double L=1-probs.probWin==0 ? 1 : (1-probs.probWin + probs.probGammonLoss + probs.probBgLoss)/(1-probs.probWin);
    
    // calculate the take and cash points
    
    double P  = probs.probWin;
    double TPl = (L-0.5)/(W+L+0.5);
    double TP = TPl*(L+1)/(L+1-jumpVol/4.*(W+L+0.5)/(W-0.5));
    double CPl = (L+1)/(W+L+0.5);
    double CP = CPl - jumpVol*(W-0.5)/(2*(2*L*W+2*L-W-1)-jumpVol*(W+L+0.5));
    
    // define the linear fit for the appropriate part of the curve
    
    double A, B;
    if( cube == 1 )
    {
        double B2 = (2-jumpVol/6.*(W+L+0.5)*(W+L-1)/(W-0.5)/(L-0.5))/(CPl-TPl);
        double A2 = 1-jumpVol/6.*(W+L+0.5)/(W-0.5)-B2*CPl;
        
        if( P<TP )
        {
            double ETP = A2+B2*TP;
            B = (ETP+L)/TP;
            A = -L;
        }
        else if( P>CP )
        {
            double ECP = A2+B2*CP;
            B = (W-ECP)/(1-CP);
            A = W-B;
        }
        else
        {
            A = A2;
            B = B2;
        }
    }
    else if( ownsCube )
    {
        double B1 = W+L+0.5-jumpVol/4.*pow(W+L+0.5,2)/(W-0.5)/(L+1);
        double A1 = -L;
        
        if( P<CP )
        {
            A = A1;
            B = B1;
        }
        else
        {
            double ECP=A1+B1*CP;
            B = (W-ECP)/(1-CP);
            A = W-B;
        }
    }
    else
    {
        // opponent-owned cube
        
        double B2 = W+L+0.5-jumpVol/4.*pow(W+L+0.5,2)/(L-0.5)/(W+1);
        double A2 = W-B2;
        
        if( P<TP )
        {
            double ETP = A2+B2*TP;
            B = (ETP+L)/TP;
            A = -L;
        }
        else
        {
            A = A2;
            B = B2;
        }
    }
    
    return cube * ( A + B*P );
}

double _doubleExpCum(double J,double lam);
double _doubleExpInt(double J,double lam);

double _doubleExpCum(double J,double lam)
{
    if( J<0 )
        return 0.5*exp(lam*J);
    else
        return 1-0.5*exp(-lam*J);
}

double _doubleExpInt(double J,double lam)
{
    if( J<0 )
        return -0.5*(1./lam-J)*exp(lam*J);
    else
        return -0.5*(1./lam+J)*exp(-lam*J);
}

double doublestratjumpconst::equityNonlinear( const board& b, int cube, bool ownsCube, bool holdsDice )
{
    // get the game probs from the right perspective
    
    gameProbabilities probs;
    
    if( holdsDice )
    {
        board fb(b);
        fb.setPerspective(1-b.perspective());
        probs = strat.boardProbabilities(fb).flippedProbs();
    }
    else
        probs = strat.boardProbabilities(b);
    
    double W=probs.probWin==0 ? 1 : (probs.probWin + probs.probGammonWin + probs.probBgWin)/probs.probWin;
    double L=1-probs.probWin==0 ? 1 : (1-probs.probWin + probs.probGammonLoss + probs.probBgLoss)/(1-probs.probWin);
    

    double TP  = (L-0.5)/(W+L+0.5)*(L+1)/(L+1-jumpVol/4.*(W+L+0.5)/(W-0.5));
    double CP  = (L+1)/(W+L+0.5)-jumpVol*(W-0.5)/(2*(2*L*W+2*L-W-1)-jumpVol*(W+L+0.5));
    
    
    double A1o = -L;
    double B1o = W+L+0.5 - jumpVol/4.*pow(W+L+0.5,2)/(W-0.5)/(L+1);
    double ECP = A1o + B1o*CP;
    
    double B2u = W+L+0.5 - jumpVol/4.*pow(W+L+0.5,2)/(L-0.5)/(W+1);
    double A2u = W-B2u;
    double ETP = A2u + B2u*TP;
    
    double B2o = (W-ECP)/(1-CP);
    double A2o = W - B2o;
    
    double A1u = -L;
    double B1u = (ETP+L)/TP;
    
    double A2c = -(4*L+1)/3. + jumpVol*(-1+3*W+10*W*W+4*L*L*L*(1+4*W)+2*L*(-5-W+4*W*W)+2*L*L*(-7+10*W+8*W*W))/72./(W-0.5)/(L-0.5);
    double B2c = 4/3.*(W+L+0.5) - jumpVol*pow(W+L+0.5,2)*(-2+L+W+4*L*W)/18./(L-0.5)/(W-0.5);
    
    ECP = A2c + B2c*CP;
    ETP = A2c + B2c*TP;
    
    double A1c = -L;
    double B1c = (ETP+L)/TP;
    double B3c = (W-ECP)/(1-CP);
    double A3c = W-B3c;
    
    double RDo = (A1o-2*A2u)/(2*B2u-B1o);
    double RDu = (A2u-2*A1o)/(2*B1o-B2u);
    double TGo = (1-A2o)/B2o;
    double TGu = (-1-A1u)/B1u;
    double IDo = (A2c-2*A2u)/(2*B2u-B2c);
    double IDu = (A2c-2*A1o)/(2*B1o-B2c);
    
    double lam = 1./jumpVol;
    double P = probs.probWin;
    double equityNorm;
    
    if( cube==1 )
    {
        // figure out the slope of equity vs prob we need for P<0 and P>1 such that we get the right
        // values at each boundary
        
        double Al = -L;
        double Bl = 2./jumpVol*( L/2. + (A1c)*(_doubleExpCum(TGu,lam)-0.5) + B1c*(_doubleExpInt(TGu,lam)+jumpVol/2.) 
                       + (-1)*( _doubleExpCum(TP,lam) - _doubleExpCum(TGu,lam) ) 
                       + 2*(A1o)*( _doubleExpCum(IDu,lam) - _doubleExpCum(TP,lam) ) + 2*B1o*( _doubleExpInt(IDu,lam) - _doubleExpInt(TP,lam) ) 
                       + (A2c)*( _doubleExpCum(IDo,lam) - _doubleExpCum(IDu,lam) ) + B2c*( _doubleExpInt(IDo,lam) - _doubleExpInt(IDu,lam) ) 
                       + 2*(A2u)*( _doubleExpCum(CP,lam) - _doubleExpCum(IDo,lam) ) + 2*B2u*( _doubleExpInt(CP,lam) - _doubleExpInt(IDo,lam) ) 
                       + _doubleExpCum(TGo,lam) - _doubleExpCum(CP,lam) 
                       + (A3c)*(1-_doubleExpCum(TGo,lam)) - B3c*_doubleExpInt(TGo,lam) );
        double Bh = 2./jumpVol*( W/2. - ( (A1c+B1c)*_doubleExpCum(TGu-1,lam) + B1c*_doubleExpInt(TGu-1,lam) 
                                + (-1)*( _doubleExpCum(TP-1,lam) - _doubleExpCum(TGu-1,lam) ) 
                                + 2*(A1o+B1o)*( _doubleExpCum(IDu-1,lam) - _doubleExpCum(TP-1,lam) ) + 2*B1o*( _doubleExpInt(IDu-1,lam) - _doubleExpInt(TP-1,lam) ) 
                                + (A2c+B2c)*( _doubleExpCum(IDo-1,lam) - _doubleExpCum(IDu-1,lam) ) + B2c*( _doubleExpInt(IDo-1,lam) - _doubleExpInt(IDu-1,lam) ) 
                                + 2*(A2u+B2u)*( _doubleExpCum(CP-1,lam) - _doubleExpCum(IDo-1,lam) ) + 2*B2u*( _doubleExpInt(CP-1,lam) - _doubleExpInt(IDo-1,lam) ) 
                                + _doubleExpCum(TGo-1,lam) - _doubleExpCum(CP-1,lam) \
                                       + (A3c+B3c)*(0.5-_doubleExpCum(TGo-1,lam)) + B3c*(-jumpVol/2.-_doubleExpInt(TGo-1,lam)) ) );
        double Ah = W-Bh;
        
        equityNorm = (Al+Bl*P)*_doubleExpCum(-P,lam)+Bl*_doubleExpInt(-P,lam) 
                    + (A1c+B1c*P)*(_doubleExpCum(TGu-P,lam)-_doubleExpCum(-P,lam)) + B1c*(_doubleExpInt(TGu-P,lam)-_doubleExpInt(-P,lam)) 
                    + (-1)*( _doubleExpCum(TP-P,lam) - _doubleExpCum(TGu-P,lam) ) \
                    + 2*(A1o+B1o*P)*( _doubleExpCum(IDu-P,lam) - _doubleExpCum(TP-P,lam) ) + 2*B1o*( _doubleExpInt(IDu-P,lam) - _doubleExpInt(TP-P,lam) ) 
                    + (A2c+B2c*P)*( _doubleExpCum(IDo-P,lam) - _doubleExpCum(IDu-P,lam) ) + B2c*( _doubleExpInt(IDo-P,lam) - _doubleExpInt(IDu-P,lam) ) 
                    + 2*(A2u+B2u*P)*( _doubleExpCum(CP-P,lam) - _doubleExpCum(IDo-P,lam) ) + 2*B2u*( _doubleExpInt(CP-P,lam) - _doubleExpInt(IDo-P,lam) ) 
                    + _doubleExpCum(TGo-P,lam) - _doubleExpCum(CP-P,lam) 
                    + (A3c+B3c*P)*(_doubleExpCum(1-P,lam)-_doubleExpCum(TGo-P,lam)) + B3c*(_doubleExpInt(1-P,lam)-_doubleExpInt(TGo-P,lam)) 
                    + (Ah+Bh*P)*(1-_doubleExpCum(1-P,lam))-Bh*_doubleExpInt(1-P,lam);
    }
    else if( ownsCube )
    {
        // adjust the function at P=1 to make it better hit W at P=1 - find the approximate linear fn for P>1 to make it so
        
        double Bh = 2./jumpVol*( W/2 - ( (A1o+B1o)*_doubleExpCum(RDo-1,lam) + B1o * _doubleExpInt(RDo-1,lam) 
                               + 2*(A2u+B2u)*( _doubleExpCum(CP-1,lam) - _doubleExpCum(RDo-1,lam) ) + 2*B2u*( _doubleExpInt(CP-1,lam) - _doubleExpInt(RDo-1,lam) ) 
                               + _doubleExpCum(TGo-1,lam) - _doubleExpCum(CP-1,lam) 
                               + (A2o+B2o)*(0.5-_doubleExpCum(TGo-1,lam)) + B2o*(-jumpVol/2.-_doubleExpInt(TGo-1,lam)) ) );
        double Ah = W - Bh;
        
        equityNorm = (A1o+B1o*P)*_doubleExpCum(RDo-P,lam) + B1o * _doubleExpInt(RDo-P,lam) 
                    + 2*(A2u+B2u*P)*( _doubleExpCum(CP-P,lam) - _doubleExpCum(RDo-P,lam) ) + 2*B2u*( _doubleExpInt(CP-P,lam) - _doubleExpInt(RDo-P,lam) ) 
                    + _doubleExpCum(TGo-P,lam) - _doubleExpCum(CP-P,lam) 
                    + (A2o+B2o*P)*(_doubleExpCum(1-P,lam)-_doubleExpCum(TGo-P,lam)) + B2o*(_doubleExpInt(1-P,lam)-_doubleExpInt(TGo-P,lam)) 
                    + (Ah+Bh*P)*(1-_doubleExpCum(1-P,lam)) - Bh*_doubleExpInt(1-P,lam);
    }
    else
    {
        // adjust the function at P=0 to make it better hit -L at P=0
            
        double Al = -L;
        double Bl = 2./jumpVol*( L/2. + (A1u)*(_doubleExpCum(TGu,lam)-0.5) + B1u*(_doubleExpInt(TGu,lam)+jumpVol/2.) 
                       + (-1)*( _doubleExpCum(TP,lam) - _doubleExpCum(TGu,lam) ) 
                       + 2*(A1o)*( _doubleExpCum(RDu,lam) - _doubleExpCum(TP,lam) ) + 2*B1o*( _doubleExpInt(RDu,lam) - _doubleExpInt(TP,lam) ) 
                       + (A2u)*( 1 - _doubleExpCum(RDu,lam) ) - B2u*_doubleExpInt(RDu,lam) );
        
        equityNorm = (Al+Bl*P)*_doubleExpCum(-P,lam) + Bl*_doubleExpInt(-P,lam) 
                    + (A1u+B1u*P)*(_doubleExpCum(TGu-P,lam)-_doubleExpCum(-P,lam)) + B1u*(_doubleExpInt(TGu-P,lam)-_doubleExpInt(-P,lam)) 
                    + (-1)*( _doubleExpCum(TP-P,lam) - _doubleExpCum(TGu-P,lam) ) 
                    + 2*(A1o+B1o*P)*( _doubleExpCum(RDu-P,lam) - _doubleExpCum(TP-P,lam) ) + 2*B1o*( _doubleExpInt(RDu-P,lam) - _doubleExpInt(TP-P,lam) ) 
                    + (A2u+B2u*P)*( 1 - _doubleExpCum(RDu-P,lam) ) - B2u*_doubleExpInt(RDu-P,lam);
    }
        
    return equityNorm * cube;
}