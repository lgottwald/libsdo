#ifndef _MDL_VOP_FILE_HPP_
#define _MDL_VOP_FILE_HPP_

#include <vector>
#include <string>
#include <list>
#include <boost/filesystem/path.hpp>
#include "FileStatus.hpp"
namespace sdo
{

/**
 * Class to store contents of a vop file
 */
class VopFile : public FileStatus
{
public:

   void addCategory( const std::string &s )
   {
      categories_.emplace_back( s );
   }
   void setModelFile( std::string m )
   {
      model_ = std::move(m);
   }
   void setObjectiveFile( std::string o )
   {
      objective_ = std::move(o);
   }
   void setControlFile( std::string c )
   {
      control_ = std::move(c);
   }

   const std::string &getModelFile() const
   {
      return model_;
   }
   const std::string &getObjectiveFile() const
   {
      return objective_;
   }
   const std::string &getControlFile() const
   {
      return control_;
   }

   const std::vector<std::string>& getCategories() const
   {
      return categories_;
   }

   std::vector<std::string>& getCategories()
   {
      return categories_;
   }

private:
   std::string model_;
   std::string control_;
   std::string objective_;
   std::vector<std::string> categories_;
};

} // mdl


#endif
