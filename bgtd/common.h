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

#ifndef bgtd_common_h
#define bgtd_common_h

#include <string>

// define a hash fn for string so that hash_map can use strings as keys

namespace __gnu_cxx                                                                                 
{                                                                                             
    template<> struct hash< std::string >                                                       
    {                                                                                           
        size_t operator()( const std::string& x ) const                                           
        {                                                                                         
            return hash< const char* >()( x.c_str() );                                              
        }                                                                                         
    };                                                                                          
}   



#endif
