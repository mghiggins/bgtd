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

#ifndef bgtd_strategytdorigsym_h
#define bgtd_strategytdorigsym_h

#include "strategytdbase.h"

class strategytdorigsym : public strategytdbase
{
    // TD gammon strategy with one output node that represents the probability of
    // any kind of a win. It has a sigmoid activation function that depends on the
    // weighted sum of middle node values plus a bias weight. Middle nodes have
    // sigmoid activation functions that depend on the weighted sum of input value,
    // with no bias weight.
    // The inputs contain two elements for whose turn it is (1 or 0 for each player
    // for on turn or not), and the network enforces perspective-flip symmetry.
    
public:
    strategytdorigsym( doublestrat * ds=0 );
    strategytdorigsym( int nMiddle, doublestrat * ds=0 );
    virtual ~strategytdorigsym();
    
    virtual gameProbabilities boardProbabilities( const board& brd, const hash_map<string,int>* context=0 ); 
    
    vector<double> getInputValues( const board& brd, bool holdDice ) const;
    vector<double> getMiddleValues( const vector<double>& inputs ) const;
    double getOutput( const vector<double>& middles ) const;
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    
private:
    void setup();
    
    vector<double> outputWeights;           // weights of the output node to each of the hidden nodes
    vector< vector<double> > middleWeights; // weights of each of the middle nodes to each of the input nodes
};

#endif
