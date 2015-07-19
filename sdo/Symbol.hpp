#ifndef _MDL_SYMBOL_HPP_
#define _MDL_SYMBOL_HPP_

#include <boost/flyweight.hpp>
#include <boost/flyweight/no_locking.hpp>
#include <functional>
#include <string>

namespace sdo {

using Symbol = boost::flyweight<std::string, boost::flyweights::no_locking>;

} //mdl

namespace std {
    template<>
    struct hash<sdo::Symbol> : public hash<string> {
    };
}

#endif