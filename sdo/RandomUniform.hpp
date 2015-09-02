#ifndef _MDL_RANDOM_UNIFORM_HPP_
#define _MDL_RANDOM_UNIFORM_HPP_

#include <random>
#include <type_traits>

namespace sdo {

namespace random {
extern std::random_device rd;
extern std::mt19937 gen;
}

template<typename REAL>
REAL random_uniform( const REAL a, const REAL b )
{
   static_assert(std::is_floating_point<REAL>::value, "random_uniform expects floating point type argument but got something else");
   std::uniform_real_distribution<REAL> dis( a, b );
   return dis( random::gen );
}

}

#endif