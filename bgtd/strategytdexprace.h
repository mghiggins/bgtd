//
//  strategytdexprace.h
//  bgtd
//
//  Created by Mark Higgins on 8/21/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef bgtd_strategytdexprace_h
#define bgtd_strategytdexprace_h

#include <string>
#include "strategytdexp.h"

class strategytdexprace : public strategytdexp
{
    // Like strategytdexp, except there are two neural nets: one for 
    // contact and one for a race.
    
public:
    strategytdexprace();
    strategytdexprace( const strategytdexp& baseStrat );
    strategytdexprace( const string& pathEnd, const string& fileSuffix, bool loadRace );
    virtual ~strategytdexprace() {};
    
    virtual double boardValue( const board& brd ) const;
    
    double getOutputProbValue( const vector<double>& middles ) const;
    double getOutputGammonValue( const vector<double>& middles, const board& brd ) const;
    double getOutputGammonLossValue( const vector<double>& middles, const board& brd ) const;
    
    // add new values for the network we use when there's a race
    
    vector<double> getMiddleValuesRace( const vector<double>& inputs ) const;
    double getOutputProbValueRace( const vector<double>& middles ) const;
    double getOutputGammonValueRace( const vector<double>& middles, const board& brd ) const;
    double getOutputGammonLossValueRace( const vector<double>& middles, const board& brd ) const;
    
    vector<double> getOutputProbWeightsRace() const;
    vector<double> getOutputGammonWeightsRace() const;
    vector< vector<double> > getMiddleWeightsRace() const;
    vector<double> getOutputProbTracesRace() const;
    vector<double> getOutputGammonTracesRace() const;
    vector< vector<double> > getMiddleProbTracesRace() const;
    vector< vector<double> > getMiddleGammonTracesRace() const;
    
    virtual void update( const board& oldBoard, const board& newBoard );

private:
    void setupAll();
    void setupRace();
    
    void loadExpValues( const strategytdexp& baseStrat );
    
    vector<double> outputProbWeightsRace; // weights of the first output node (prob of player win) to the middle nodes
    vector<double> outputGammonWeightsRace; // weights of the second output node (prob of player gammon conditional on a player win) to the middle nodes
    vector< vector<double> > middleWeightsRace; // weights of each of the middle nodes to each of the input nodes
    
    vector<double> outputProbTracesRace;  // eligibility traces for the weights in the first output node
    vector<double> outputGammonTracesRace;  // eligibility traces for the weights in the second output node
    vector< vector<double> > middleProbTracesRace;  // eligibility traces for the weights in each of the middle nodes wrt the first output
    vector< vector<double> > middleGammonTracesRace;  // eligibility traces for the weights in each of the middle nodes wrt the second output

};


#endif
