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

#ifndef bgtd_strategytdexp3_h
#define bgtd_strategytdexp3_h

#include "strategytdbase.h"
#include "strategytdexp.h"

class strategytdexp3 : public strategytdbase
{
    // Class like strategytdexp, where we still keep the symmetry assumptions, but
    // add an extra input for each player representing whether it's that player's turn.
    // Also always uses lambda=0. Also includes prob of backgammon node.
public:
    // initialize with random weights
    strategytdexp3( int nMiddle );
    // initialize with the tdexp network
    strategytdexp3( const strategytdexp& baseStrat );
    // initialize using saved weights; if loadExp is true it assumes that the weights
    // are from a tdexp network, otherwise from a proper tdexp3 network
    strategytdexp3( const string& pathEnd, const string& fileSuffix, bool loadExp );
    virtual ~strategytdexp3() {};
    
    // boardValue returns the expected number of points the player with the dice will
    // win given the board setup. This is what the game uses to determine which move to
    // make.
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 ) const;
    
    // the next set of methods is used in evaluating the neural network outputs given the
    // inputs defined by the board. 
    
    vector<double> getInputValues( const board& brd, bool holdDice ) const;
    vector<double> getMiddleValues( const vector<double>& inputs ) const;
    double getOutputProbValue( const vector<double>& middles ) const;
    double getOutputGammonWinValue( const vector<double>& middles ) const;
    double getOutputGammonLossValue( const vector<double>& middles ) const;
    double getOutputBackgammonWinValue( const vector<double>& middles ) const;
    double getOutputBackgammonLossValue( const vector<double>& middles ) const;
    
    // the next set of methods is used to introspect on the state of the weights
    
    vector<double> getOutputProbWeights() const;
    vector<double> getOutputGammonWinWeights() const;
    vector<double> getOutputBackgammonWinWeights() const;
    
    vector< vector<double> > getMiddleWeights() const;
    
    // update takes the old and new boards (before and after what it guesses is the optimal move)
    // and updates the weights. needsUpdate tells the game whether it needs to do an update.
    
    virtual bool needsUpdate() const;
    virtual void update( const board& oldBoard, const board& newBoard );
    
    // these flags control the training/valuing behaviour
    
    // useBg determines whether the backgammon node is used - if off, it only uses the win & gammon
    // nodes to evaluate the board (and for training).
    
    bool useBg;
    
    // weightLearning: false means that all nodes use unweighed alpha and beta for learning; true
    // means that the alpha & beta for the gammon node are weighted by prob of win, and for the
    // backgammon node by prob of win * cond prob of gammon.
    
    bool weightLearning;
    
    // stopGammonTraining: true, training of the gammon & backgammon nodes stops once the other
    // player has taken in at least one checker; false, trains those nodes all the way to the end.
    
    bool stopGammonTraining;
    
    // exactGammon: true, uses zero for gammon prob when the other player has taken in at least
    // one checker; false, always uses the network estimate.
    
    bool exactGammon;
    
    // symmetric: true, assigns zero weight to the "whose turn is it" input; false, trains that.
    
    bool symmetric;
    
    // trainFlipped: true, trains the flipped-perspective board each time too
    
    bool trainFlipped;
    
protected:
    void updateLocal( const board& oldBoard, const board& newBoard, bool holdDice );
    void setupExp( const strategytdexp& baseStrat );
    void setFlags();
    
    vector<double> outputProbWeights;
    vector<double> outputGammonWinWeights;
    vector<double> outputBackgammonWinWeights;
    
    vector< vector<double> > middleWeights;
    
    // define some vectors to hold derivative values in the update function. They're
    // overwritten each update, but we allocate memory for the vectors at startup
    // so we don't do it each iteration.
    
    vector<double> probDerivs;                      // derivs of output prob node to its weights vs middle node values
    vector<double> gamWinDerivs;                    // derivs of output gammon win node to its weights vs middle node values
    vector<double> bgWinDerivs;                     // derivs of output backgammon win node to its weights vs middle node values
    vector< vector<double> > probInputDerivs;       // deriv of the prob win output to all the middle->input weights
    vector< vector<double> > gamWinInputDerivs;     // deriv of the gammon win output to all the middle->input weights
    vector< vector<double> > bgWinInputDerivs;      // deriv of the backgammon win output to all the middle->input weights
};

#endif
