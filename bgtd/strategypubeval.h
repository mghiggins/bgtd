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

#ifndef bgtd_strategypubeval_h
#define bgtd_strategypubeval_h

#include <vector>
#include "strategy.h"

vector<double> pubEvalInputs( const board& brd );

class strategyPubEval : public strategy
{
public:
    strategyPubEval( bool randomWeights=false, bool valueIsEquity=false, double alpha=0 );
    virtual ~strategyPubEval();
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 );
    virtual hash_map<string,int> boardContext( const board& brd ) const;
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights using TD learning. updateFromEquity does supervised learning when
    // supplied with target equity value.
    
    bool learning;
    bool valueIsEquity;
    double alpha;
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    void updateFromEquity( const board& brd, double equity );
    
    void writeWeights( const string& filePrefix, bool writeContact=true, bool writeRace=true );
    void loadWeights( const string& filePrefix );
    
    vector<double> getWeightsContact() const { return weightsContact; };
    vector<double> getWeightsRace() const { return weightsRace; };
    vector<double>& getWeightsContactRef() { return weightsContact; };
    vector<double>& getWeightsRaceRef() { return weightsRace; };
    
private:
    void initializeRandomWeights();
    void initializeTesauroWeights();
    vector<double> weightsContact;
    vector<double> weightsRace;
};

#endif
