#ifndef _LOOKUP_TABLE_H_
#define _LOOKUP_TABLE_H_

#include <boost/tuple/tuple_comparison.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <memory>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <limits>
#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>

namespace sdo
{

using boost::optional;

class LookupTable
{
public:
   using iterator = boost::zip_iterator<
      boost::tuple <std::vector<double>::const_iterator, std::vector<double>::const_iterator>
   >;

   void addPoint( double, double );

   iterator getPoint( unsigned i ) const;
   iterator begin() const;
   iterator end() const;

   std::vector<double>& getXvals()
   {
      return x;
   }

   std::vector<double>& getYvals()
   {
      return y;
   }

   const std::vector<double>& getXvals() const
   {
      return x;
   }

   const std::vector<double>& getYvals() const
   {
      return y;
   }

   std::size_t size() const {
      return x.size();
   }

   double operator()( double ) const;

   template<class Archive>
   void serialize( Archive& ar, const unsigned int version )
   {
      ar& x;
      ar& y;
   }

private:
   std::vector<double> x;
   std::vector<double> y;
};

// =================== Inline implementation =====================

inline void LookupTable::addPoint( double px, double py )
{
   x.push_back( px );
   y.push_back( py );
}


inline LookupTable::iterator LookupTable::getPoint( unsigned i ) const
{
   return boost::make_zip_iterator(
             boost::make_tuple(
                x.begin() + i,
                y.begin() + i ) );
}

inline LookupTable::iterator LookupTable::begin() const
{
   return boost::make_zip_iterator(
             boost::make_tuple(
                x.begin(),
                y.begin() ) );
}

inline LookupTable::iterator LookupTable::end() const
{
   return boost::make_zip_iterator(
             boost::make_tuple(
                x.end(),
                y.end() ) );
}


inline double LookupTable::operator()( double v ) const
{
   if( v >= *( x.end() - 1 ) )
      return *( y.end() - 1 );

   if( v <= *x.begin() )
      return *y.begin();

   int j = std::upper_bound(
              x.begin(),
              x.end(), v ) - x.begin() - 1;
   return  y[j] + ( y[j + 1] - y[j] ) * ( v - x[j] ) / ( x[j + 1] - x[j] );
}

inline
bool operator==( const LookupTable &a, const LookupTable &b )
{
   LookupTable::iterator aiter, biter;
   aiter = a.begin();
   biter = b.begin();

   while( aiter != a.end() && biter != b.end() )
   {
      if( *( aiter++ ) != *( biter++ ) )
         return false;
   }

   return aiter == a.end() && biter == b.end();
}

inline
std::size_t hash_value( const LookupTable& lkp )
{
   std::size_t hash = boost::hash_range( lkp.getXvals().begin(), lkp.getXvals().end() );
   boost::hash_combine(
      hash,
      boost::hash_range( lkp.getYvals().begin(), lkp.getYvals().end() )
   );
   return hash;
}


}


#endif // _LOOKUP_TABLE_H_

