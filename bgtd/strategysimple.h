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

#ifndef bgtd_strategysimple_h
#define bgtd_strategysimple_h

#include "strategy.h"
#include "randomc.h"

class strategySimple : public strategy
{
public:
    strategySimple();
    strategySimple( double singletonWeight, double towerWeight, double runWeight );
    virtual ~strategySimple() {};
    
    virtual double boardValue( const board& brd, const hash_map<string,int>* context=0 ) const;
    
    double singletonWeight;
    double towerWeight;
    double runWeight;
};

class strategyRandom : public strategy
{
public:
    strategyRandom( int seed );
    virtual ~strategyRandom();
    
    virtual double boardValue( const board& brd ) const;
    
private:
    CRandomMersenne * rng;
};


#endif
