#ifndef _MDL_PARSE_ERRORS_HPP_
#define _MDL_PARSE_ERRORS_HPP_
#include <list>
#include <vector>
#include <string>
#include "Location.hpp"
#include <stdexcept>
#include <sstream>

namespace sdo {

   struct FileMessage {
      bool error;
      std::vector<FileLocation> locations;
      std::string msg;
   };

   /**
    * Class to gather errors and warnings. Used by classes that
    * process files as a base class. Is not used for polymorphism
    * so no virtual destructor required.
    */
   class FileStatus {
   public:
      FileStatus() : num_errors_(0) {}

      void error(const std::string &filename, const Location &loc, std::string msg );

      void warning(const std::string &filename, const Location &loc, std::string msg );

      void error(const std::vector<FileLocation> &locs, std::string msg );

      void warning(const std::vector<FileLocation> &locs, std::string msg );

      bool hasErrors() const;

      bool hasWarnings() const;

      void report(std::ostream &os, bool errors = true, bool warnings = true) const;

      std::string report(bool errors = true, bool warnings = true) const;

   private:
      std::vector<FileMessage> messages_;
      unsigned num_errors_;
   };

   /**
    * Class to create a parse_error exception with a message about all errors
    * from a FileStatus object.
    */
   class parse_error : public std::runtime_error {
   public:
      parse_error( const FileStatus &status ) : std::runtime_error(status.report()) {}
   };
}

#endif