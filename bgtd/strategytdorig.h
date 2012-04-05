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

#ifndef bgtd_strategytdorig_h
#define bgtd_strategytdorig_h

#include "strategytdbase.h"

class strategytdorig : public strategytdbase
{
    // TD gammon strategy with one output node that represents the probability of
    // any kind of a win. It has a sigmoid activation function that depends on the
    // weighted sum of middle node values plus a bias weight. Middle nodes have
    // sigmoid activation functions that depend on the weighted sum of input value,
    // with no bias weight.
    // The inputs do not contain a flag for whose turn it is, and no symmetry 
    // assumptions are made.
    
public:
    strategytdorig( doublestrat * ds=0 );
    strategytdorig( int nMiddle, doublestrat * ds=0 );
    virtual ~strategytdorig();
    
    virtual gameProbabilities boardProbabilities( const board& brd, const hash_map<string,int>* context=0 ); 
    
    vector<double> getInputValues( const board& brd ) const;
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
