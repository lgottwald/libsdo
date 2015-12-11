#ifndef _MDL_OBJECTIVE_H_
#define _MDL_OBJECTIVE_H_

#include <vector>
#include "FileStatus.hpp"
#include "Symbol.hpp"

namespace sdo
{

/**
 * Class representing the content of a vpd file, i.e. the objective function of a mdl file.
 */
class Objective : public FileStatus
{
public:

   /**
    * \brief Struct that represents a summand in the objective function.
    */
   struct Summand
   {
      enum Type { 
         MAYER, /*!< Value at final time is summand in objective */ 
         LAGRANGE /*!<  Value at all times are summands in objective */ 
      };

      Summand( Type _type, Symbol _variable, double _coefficient ) :
         type( _type ),
         variable( _variable ),
         coefficient( _coefficient ) {}

      Summand( Type _type, Symbol _variable ) :
         type( _type ),
         variable( _variable ),
         coefficient( 1.0 ) {}

      Type type;
      Symbol variable;
      double coefficient;

   };

   std::vector<Summand>& getSummands()
   {
      return summands_;
   }

   const std::vector<Summand>& getSummands() const
   {
      return summands_;
   }

   bool isMinimized() const
   {
      return !maximize_;
   }

   bool isMaximized() const
   {
      return maximize_;
   }

   void setMinimized( bool b )
   {
      maximize_ = !b;
   }

   void setMaximized( bool b )
   {
      maximize_ = b;
   }

   void addSummand( Summand::Type type, Symbol variable, double coefficient )
   {
      summands_.push_back( Summand( type, variable, coefficient ) );
   }

   bool empty() const {
      return summands_.empty();
   }

private:
   std::vector<Summand> summands_;
   bool maximize_ = false;
};

}
#endif // _MDL_OBJECTIVE_H_
