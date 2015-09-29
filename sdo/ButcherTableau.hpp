#ifndef _BUTCHER_TABLEAU_H_
#define _BUTCHER_TABLEAU_H_

#include <vector>
#include <map>

namespace sdo
{

/**
 * \brief Class representing a predefined butcher tableau.
 */
class ButcherTableau
{
public:
   static constexpr int MAX_COLS() { return 4; }
   static constexpr int MAX_ROWS() { return 5; }
   /**
    * \brief The names of the predefined butcher tableaus.
    */
   enum Name
   {
      RUNGE_KUTTA_2, /*!< Butcher tableau for Runge Kutta method (order 2) */
      RUNGE_KUTTA_3, /*!< Butcher tableau for Runge Kutta method (order 3) */
      RUNGE_KUTTA_4, /*!< Butcher tableau for Runge Kutta method (order 4) */
      IMPLICIT_MIDPOINT_2, /*!< Butcher tableau for implicit midpoint method (order 2) */
      GAUSS_LEGENDRE_4,    /*!< Butcher tableau for implicit Gauß-Legendre method (order 4) */
      EULER                /*!< Butcher tableau for euler method (order 1) */
   };

   ButcherTableau() {}

   /**
    * Access the values in the butcher tableau.
    * \return pointer to row i
    */
   const double* operator[]( unsigned i ) const
   {
      return &TABLEAU_[i * NPOINTS_];
   }

   /**
    * \return Number of rows in the butcher tableau.
    */
   int rows() const
   {
      return NPOINTS_ + 1;
   }
   /**
    * \return Number of columns in the butcher tableau.
    */
   int columns() const
   {
      return NPOINTS_;
   }

   /**
    * \return Number of Stages in the scheme
    */
   int stages() const
   {
      return NPOINTS_;
   }

   void setTableau( Name name );

   /**
    * \return the name of current tableau.
    */
   Name getName() const;

   std::vector<double> getRow(int i);


private:
   int NPOINTS_;
   const double* TABLEAU_;
   Name name_;
   std::map<int, std::vector<double> > rows_;
};

}

#endif
