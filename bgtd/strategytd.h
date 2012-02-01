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

#ifndef bgtd_strategytd_h
#define bgtd_strategytd_h

#include "strategytdbase.h"

class strategytd : public strategytdbase
{
    // TD gammon strategy - sets up a little neural network for predicting the
    // probability of winning a game. Can work in training mode where the weights
    // in the net are updated as the game progresses.
    //
    // A relatively simple net that has only one output node, reflecting the probability
    // of winning the game. So it does not optimize for gammons.
    //
    // There are 196 input nodes that represent the board layout (always from the perspective
    // of the person moving, so a bit different than the original Tesauro version which kept
    // the perspective the same and added two input nodes to represent whose turn it is).
    
public:
    strategytd();
    strategytd( int nMiddle );
    strategytd( const vector<double>& outputWeights, const vector< vector<double> >& middleWeights, const vector<double>& outputTraces, const vector< vector<double> >& middleTraces, double alpha, double beta, double lambda );
    strategytd( const strategytd& otherStrat );
    virtual ~strategytd();
    
    virtual gameProbabilities boardProbabilities( const board& brd, const hash_map<string,int>* context=0 ) const; 
    
    vector<double> getInputValues( const board& brd ) const;
    vector<double> getMiddleValues( const vector<double>& inputs ) const;
    double getOutput( const vector<double>& middles ) const;
    
    vector<double> getOutputWeights() const;
    vector< vector<double> > getMiddleWeights() const;
    vector<double> getOutputTraces() const;
    vector< vector<double> > getMiddleTraces() const;
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    
private:
    void setup();
    
    vector<double> outputWeights;           // weights of the output node to each of the hidden nodes
    vector< vector<double> > middleWeights; // weights of each of the middle nodes to each of the input nodes
    
    vector<double> outputTraces;            // eligibility traces for the weights in the output node
    vector< vector<double> > middleTraces;  // eligibility traces for the weights in each of the middle nodes
};

#endif
