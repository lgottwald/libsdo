#ifndef _LOCATION_HPP_
#define _LOCATION_HPP_

#include <ostream>
#include <utility>

namespace sdo {

struct Location {
   int first_line;
   int first_column;
   int last_line;
   int last_column;
};

using FileLocation = std::pair<std::string, Location>;

inline std::ostream& operator<<(std::ostream& os, const Location& loc)
{
   os << loc.first_line << '.' << loc.first_column << '-' << loc.last_line << '.' << loc.last_column;
   return os;
}

inline std::ostream& operator<<(std::ostream& os, const FileLocation& loc)
{
    os << loc.first << ":" << loc.second;
    return os;
}

}

#define YYLTYPE sdo::Location
#define YYLTYPE_IS_TRIVIAL 1

#endif