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

#ifndef bgtd_strategytdbase_h
#define bgtd_strategytdbase_h

#include "strategyprob.h"

class strategytdbase : public strategyprob
{
public:
    strategytdbase( doublestrat * ds=0 ) : strategyprob(ds) {};
    virtual ~strategytdbase() {};
    
    int nMiddle;   // number of middle nodes
    
    // then stuff used for learning
    
    bool learning; // true, it updates weights as it goes; false, it doesn't
    
    double alpha;  // how far we move the output weights to get them closer to the goal
    double beta;   // sim for middle weights
    double lambda; // how much we weight past estimates of target - btw 0 and 1 (0 means no memory)
};

#endif
