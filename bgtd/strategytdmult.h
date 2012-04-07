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

#ifndef bgtd_strategytdmult_h
#define bgtd_strategytdmult_h

#include <vector>
#include <string>
#include <hash_map.h>
#include "strategytdbase.h"

class strategytdmult : public strategytdbase
{
    // A generalized TD strategy. Contains multiple networks for different game states
    // plus uses a bearoff database for bearoff races. Cubeless. No symmetry assumption
    // and no input for whose turn it is.
    
public:
    strategytdmult( doublestrat * ds=0 );
    strategytdmult( int nMiddle, bool useShotProbInput=true, bool usePrimesInput=true, bool useExtendedBearoffInputs=true, doublestrat * ds=0 );
    strategytdmult( const string& path, const string& filePrefix, doublestrat * ds=0 );
    virtual ~strategytdmult() {};
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 );
    virtual gameProbabilities boardProbabilities( const board& brd, const hash_map<string,int>* context=0 ); 
    
    string evaluator( const board& brd) const;
    
    int nInputs( const string& netName ) const;
    vector<double> getInputValues( const board& brd, const string& netName ) const;
    vector<double> getBaseInputValues( const board& brd ) const;
    vector<double> getMiddleValues( const vector<double>& inputs, const string& netName ) const;
    double getOutputProbValue( const vector<double>& middles, const string& netName ) const;
    double getOutputGammonValue( const vector<double>& middles, const string& netName ) const;
    double getOutputGammonLossValue( const vector<double>& middles, const string& netName ) const;
    double getOutputBackgammonValue( const vector<double>& middles, const string& netName ) const;
    double getOutputBackgammonLossValue( const vector<double>& middles, const string& netName ) const;
    
    double bearoffValue( const board& brd, bool valIsAnyWin ) const;
    double bearoffProbabilityWin( const board& brd ) const;
    double bearoffProbabilityGammon( const board& brd ) const;
    double bearoffProbabilityGammonLoss( const board& brd ) const;
    double doneValue( const board& brd, bool valIsAnyWin ) const;
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights using TD learning. updateFromProbs does supervised learning when
    // supplied with target probabilities.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    void updateFromProbs( const board& brd, double probWin, double probGammonWin, double probGammonLoss, double probBgWin, double probBgLoss );
    
    // writeWeights writes the weights for each network to files (one per network, suffixed with the network name).
    // If singleNetName is blank it writes files for all networks' weights; otherwise it just writes files for the
    // specified network name. Also writes a file called netNames that lists out the network names for the player
    // along with parameter info; that gets written if singleNetName is blank or if it's "contact".
    
    void writeWeights( const string& filePrefix, const string& singleNetName="" ) const;
    void loadWeights( const string& path, const string& filePrefix );
    
    // define some parameters
    
    bool useShotProbInput;
    bool usePrimesInput;
    bool useExtendedBearoffInputs;
    
    // interface to weights
    
    const vector<double>& getOutputProbWeights( const string& netName ) const;
    const vector<double>& getOutputGammonWeights( const string& netName ) const;
    const vector<double>& getOutputGammonLossWeights( const string& netName ) const;
    const vector<double>& getOutputBgWeights( const string& netName ) const;
    const vector<double>& getOutputBgLossWeights( const string& netName ) const;
    const vector< vector<double> >& getMiddleWeights( const string& netName ) const;
    
    // flag defining whether it optimizes play based on cubeful equity
    
    bool useCubefulEquity;
    
private:
    void setup();
    void setupRandomWeights( int nMiddle );
    
    // network weights
    
    hash_map<string, vector<double> >           outputProbWeights;    // weights of the 1st output node (prob of player win) to the middle nodes, keyed by network
    hash_map<string, vector<double> >           outputGammonWeights;  // weights of the 2nd output node (prob of player gammon win) to the middle nodes, keyed by network
    hash_map<string, vector<double> >           outputGammonLossWeights; // sim for gammon loss output
    hash_map<string, vector<double> >           outputBgWeights;      // weights for backgammon win output
    hash_map<string, vector<double> >           outputBgLossWeights;  // weights for backgammon loss output
    hash_map<string, vector< vector<double> > > middleWeights;        // weights of the middle nodes to the input nodes, keyed by network
    
    // parameters for the bearoff db
    
    int bearoffNPnts;
    int bearoffNCheckers;
};

#endif


