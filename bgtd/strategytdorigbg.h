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

#ifndef bgtd_strategytdorigbg_h
#define bgtd_strategytdorigbg_h

#include "strategytdoriggam.h"

class strategytdorigbg : public strategytdoriggam
{
    // An extension of the "gammon" version of the original TD strategy
    // to backgammon outputs. So it has five outputs: prob of any win;
    // prob of gammon loss (incl backgammon); prob of gammon win (incl backgammon);
    // prob of backgammon loss; and prob of backgammon win.
public:
    strategytdorigbg();
    strategytdorigbg( int nMiddle );
    strategytdorigbg( const string& subPath, const string& filePrefix );
    virtual ~strategytdorigbg() {};
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 ) const;

    double getOutputBackgammon( const vector<double>& middles ) const;
    double getOutputBackgammonLoss( const vector<double>& middles ) const;

    // write and read weights
    
    void writeWeights( const string& filePrefix ) const;
    void loadWeights( const string& subPath, const string& filePrefix );
    
    virtual void update( const board& oldBoard, const board& newBoard );

protected:
    void setup();
    void setupBgWeights();
    
    vector<double> outputBgWeights;     // weights of the prob of backgammon win node to each of the hidden nodes
    vector<double> outputBgLossWeights; // weights of the prob of backgammon loss node to each of the hidden nodes
};

#endif
