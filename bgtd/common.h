//
//  common.h
//  bgtd
//
//  Created by Mark Higgins on 1/21/12.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

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
