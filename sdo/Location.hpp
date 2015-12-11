#ifndef _LOCATION_HPP_
#define _LOCATION_HPP_

#include <ostream>
#include <utility>

namespace sdo {
/**
 * Struct to store the location in a file
 */
struct Location {
   int first_line;
   int first_column;
   int last_line;
   int last_column;
};

/**
 * File name together with a location defined as FileLocation
 */
using FileLocation = std::pair<std::string, Location>;

/**
 * Overload operator << to stream a location.
 */
inline std::ostream& operator<<(std::ostream& os, const Location& loc)
{
   os << loc.first_line << '.' << loc.first_column << '-' << loc.last_line << '.' << loc.last_column;
   return os;
}

/**
 * Overload operator << to stream a file location.
 */
inline std::ostream& operator<<(std::ostream& os, const FileLocation& loc)
{
    os << loc.first << ":" << loc.second;
    return os;
}

}

#define YYLTYPE sdo::Location
#define YYLTYPE_IS_TRIVIAL 1

#endif