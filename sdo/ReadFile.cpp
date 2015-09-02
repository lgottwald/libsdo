#include "ReadFile.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <boost/locale/encoding.hpp>

namespace sdo {
struct Latin1Char
{
   bool operator()( const char c ) const
   {
      return   c == '\xC4' || // Ä
               c == '\xD6' || // Ö
               c == '\xDC' || // Ü
               c == '\xE4' || // ä
               c == '\xF6' || // ö
               c == '\xFC' || // ü
               c == '\xDF';   // ß
   }
};

std::string read_file( const std::string &file )
{
   std::string out;
   //read file into string
   std::ifstream in;
   in.exceptions( std::ifstream::failbit | std::ifstream::badbit);
   in.open(file, std::ios::in | std::ios::binary);
   in.seekg(0, std::ios::end);
   out.resize(in.tellg());
   in.seekg(0, std::ios::beg);
   in.read(&out[0], out.size());
   in.close();
   
   //Convert to utf-8 if necessary
   if( std::find_if( out.begin(), out.end(), Latin1Char() ) != out.end() )
      return boost::locale::conv::to_utf<char>(out,"Latin1");

   return out;
}

}
