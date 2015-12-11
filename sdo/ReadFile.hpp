#ifndef _MDL_READFILE_HPP_
#define _MDL_READFILE_HPP_

#include <list>
#include <string>
#include <boost/optional.hpp>

namespace sdo {
   /**
    * Read contents of file into a string. If necessary convert latin1 to utf-8.
    */
   std::string read_file( const std::string & );
} //mdl

#endif // _MDL_READFILE_HPP_
