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

#ifndef bgtd_strategytdoriggam_h
#define bgtd_strategytdoriggam_h

#include "strategytdbase.h"

class strategytdoriggam : public strategytdbase
{
    // TD gammon strategy with three output nodes: prob of win (of any kind),
    // prob of gammon win, and prob of gammon loss. Original Tesauro inputs
    // except no input for whose turn it is (network always assumes it's the
    // player's turn to roll).
    
public:
    strategytdoriggam();
    strategytdoriggam( int nMiddle );
    strategytdoriggam( const string& subPath, const string& filePrefix );
    virtual ~strategytdoriggam() {};
    
    virtual gameProbabilities boardProbabilities( const board& brd, const hash_map<string,int>* context=0 ) const; 
    
    vector<double> getInputValues( const board& brd ) const;
    vector<double> getMiddleValues( const vector<double>& inputs ) const;
    double getOutputWin( const vector<double>& middles ) const;
    double getOutputGammon( const vector<double>& middles ) const;
    double getOutputGammonLoss( const vector<double>& middles ) const;
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    
    // write and read weights
    
    void writeWeights( const string& filePrefix ) const;
    void loadWeights( const string& subPath, const string& filePrefix );
    
protected:
    void setup();
    
    vector<double> outputWinWeights;        // weights of the prob of win node to each of the hidden nodes
    vector<double> outputGammonWeights;     // weights of the prob of gammon win node to each of the hidden nodes
    vector<double> outputGammonLossWeights; // weights of the prob of gammon loss node to each of the hidden nodes
    vector< vector<double> > middleWeights; // weights of each of the middle nodes to each of the input nodes
};


#endif
