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


#ifndef bgtd_doublestrat_h
#define bgtd_doublestrat_h

#include "board.h"

class doublestrat
{
    // base class for doubling strategies.
    
public:
    doublestrat() {};
    virtual ~doublestrat() {};
    
    // offerDouble takes a board, assuming the player holds the dice and has the opportunity
    // to double. Returns true if the player should double and false if not. centeredCube
    // should be true if the cube is in the center and false if the player holds the cube.
    
    virtual bool offerDouble( const board& b, bool centeredCube ) = 0;
    
    // takeDouble takes a board, assuming the player is being offered the cube. If it returns
    // true the player takes; false, the player passes. 
    
    virtual bool takeDouble( const board& b, bool centeredCube ) = 0;
};

#endif
