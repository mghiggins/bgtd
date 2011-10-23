//
//  strategytdmult.cpp
//  bgtd
//
//  Created by Mark Higgins on 10/20/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "strategytdmult.h"


strategytdmult::strategytdmult()
{
    nMiddle = 40; // set # of middle nodes to some sensible default
    
    // set up the network vectors for all the different networks
    
    vector<string> nets(2);
    nets[0] = "normal";
    nets[1] = "race";
    
    for( vector<string>::iterator it=nets.begin(); it!=nets.end(); it++ )
    {
        vector<double> outputs;
    }
}

strategytdmult::strategytdmult( int nMiddle )
{
}

strategytdmult::strategytdmult( const string& pathEnd, const string& fileSuffix )
{
    
}

void strategytdmult::setup()
{
    
}

double strategytdmult::boardValue( const board& brd ) const
{
    
}

string strategytdmult::evaluator( const board& brd) const
{
    
}

bool strategytdmult::needsUpdate() const
{
    return learning;
}

void strategytdmult::update( const board& oldBoard, const board& newBoard )
{
    
}
