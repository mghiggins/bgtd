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

#ifndef bgtd_strategytdexp_h
#define bgtd_strategytdexp_h

#include <vector>
#include <string>
#include "strategytdbase.h"

class strategytdexp : public strategytdbase
{
    // An extension of the strategytd class approach - instead of having one output that represents
    // the probability of a win (any win), this has three output nodes that represent probabilities
    // of different kinds of wins and losses. The first output node is the probability of a win. The
    // second is the probability of a gammon conditional on a win. The third is the probability of a
    // gammon conditional on a loss. The probability of a loss is calculated on the fly as 1-first
    // output.
    
public:
    strategytdexp();
    strategytdexp( int nMiddle );
    strategytdexp( const vector<double>& outputProbWeights, vector<double>& outputGammonWeights, const vector< vector<double> >& middleWeights, 
                   const vector<double>& outputProbTraces, const vector<double>& outputGammonTraces, 
                   const vector< vector<double> >& middleProbTraces, const vector< vector<double> >& middleGammonTraces,
                   double alpha, double beta, double lambda );
    strategytdexp( const strategytdexp& otherStrat );
    strategytdexp( const string& pathEnd, const string& fileSuffix );
    virtual ~strategytdexp();
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 ) const;
    
    vector<double> getInputValues( const board& brd ) const;
    vector<double> getMiddleValues( const vector<double>& inputs ) const;
    double getOutputProbValue( const vector<double>& middles ) const;
    double getOutputGammonValue( const vector<double>& middles ) const;
    double getOutputGammonLossValue( const vector<double>& middles ) const;
    
    vector<double> getOutputProbWeights() const;
    vector<double> getOutputGammonWeights() const;
    vector< vector<double> > getMiddleWeights() const;
    vector<double> getOutputProbTraces() const;
    vector<double> getOutputGammonTraces() const;
    vector< vector<double> > getMiddleProbTraces() const;
    vector< vector<double> > getMiddleGammonTraces() const;
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    
private:
    void setup();
    
    vector<double> outputProbWeights; // weights of the first output node (prob of player win) to the middle nodes
    vector<double> outputGammonWeights; // weights of the second output node (prob of player gammon conditional on a player win) to the middle nodes
    vector< vector<double> > middleWeights; // weights of each of the middle nodes to each of the input nodes
    
    vector<double> outputProbTraces;  // eligibility traces for the weights in the first output node
    vector<double> outputGammonTraces;  // eligibility traces for the weights in the second output node
    vector< vector<double> > middleProbTraces;  // eligibility traces for the weights in each of the middle nodes wrt the first output
    vector< vector<double> > middleGammonTraces;  // eligibility traces for the weights in each of the middle nodes wrt the second output
};

#endif
