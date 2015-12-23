#ifndef _INCL_GUARD
#define _INCL_GUARD
#include "includes.h"
#endif

#ifndef _PATCH_H
#define _PATCH_H

namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

#endif
