#ifndef _MDL_SYMBOL_HPP_
#define _MDL_SYMBOL_HPP_

#include <boost/flyweight.hpp>
#include <boost/flyweight/no_locking.hpp>
#include <functional>
#include <string>

namespace sdo {

/**
 * Typedef for a symbol using boost flyweights in order to store variable
 * names only once.
 */
using Symbol = boost::flyweight<std::string, boost::flyweights::no_locking>;

} //mdl

namespace std {

    /**
     * Inherit std::hash from string since flyweight for string has automatic conversion
     */
    template<>
    struct hash<sdo::Symbol> : public hash<string> {
    };
}

#endif