#include "ButcherTableau.hpp"
#include <cmath>

using std::sqrt;

static const int RUNGE_KUTTA_2_NPOINTS = 2;

static const double RUNGE_KUTTA_2_TABLEAU[] =
{
   0.0, 0.0, 0.0,
   0.5, 0.5, 0.0,
   0.0, 0.0, 1.0
};

static const int RUNGE_KUTTA_3_NPOINTS = 3;

static const double RUNGE_KUTTA_3_TABLEAU[] =
{
   0.0, 0.0, 0.0, 0.0,
   0.5, 0.5, 0.0, 0.0,
   1.0, -1.0, 2.0, 0.0,
   0.0, 1.0 / 6.0, 2.0 / 3.0, 1.0 / 6.0
};

static const int HEUN_NPOINTS = 3;

static const double HEUN_TABLEAU[] =
{
   0.0      , 0.0      , 0.0      , 0.0,
   1.0 / 3.0, 1.0 / 3.0, 0.0      , 0.0,
   2.0 / 3.0, 0.0      , 2.0 / 3.0, 0.0,
   0.0      , 1.0 / 4.0, 0.0      , 3.0 / 4.0, 1.0 / 6.0
};

static const int RUNGE_KUTTA_4_NPOINTS = 4;

static const double RUNGE_KUTTA_4_TABLEAU[] =
{
   0.0, 0.0, 0.0, 0.0, 0.0,
   0.5, 0.5, 0.0, 0.0, 0.0,
   0.5, 0.0, 0.5, 0.0, 0.0,
   1.0, 0.0, 0.0, 1.0, 0.0,
   0.0, 1.0 / 6.0, 1.0 / 3.0, 1.0 / 3.0, 1.0 / 6.0
};

static const int IMPLICIT_MIDPOINT_2_NPOINTS = 1;

static const double IMPLICIT_MIDPOINT_2_TABLEAU[] =
{
   0.5, 0.5,
   0.0, 1.0
};

static const int  GAUSS_LEGENDRE_4_NPOINTS = 2;

static const double GAUSS_LEGENDRE_4_TABLEAU[] =
{
   0.5 - sqrt( 3 ) / 6,   0.25,                         0.25 - sqrt( 3 ) / 6,
   0.5 + sqrt( 3 ) / 6,   0.25 + sqrt( 3 ) / 6, 0.25,
   0.0,                   0.5,                          0.5
};

static const int EULER_1_NPOINTS = 1;

static const double EULER_1_TABLEAU[] =
{
   0.0, 0.0,
   0.0, 1.0
};

namespace sdo {

void ButcherTableau::setRows( ButcherTableau::Name name )
{
   /* rows_[i < nStages] contains the combined vector of c_i and coefficient matrix a_{ij} (c_i, a_i_1, a_i_2, ...)*/
   for( int i = 0; i < NPOINTS_ ; ++i)
   {
      std::vector<double> row;
      for( int j = 0; j <= i; ++j)
         row.push_back(TABLEAU_[i * (NPOINTS_ + 1) + j]);
      rows_.push_back(row);
   }
   /* rows_[i = nStages] contains the coefficients b_j (b_1, b_2, ...)*/
   {
      int i = NPOINTS_;
      std::vector<double> row;
      for( int j = 1; j <= i; ++j)
         row.push_back(TABLEAU_[i * (NPOINTS_ + 1) + j]);
      rows_.push_back(row);
   }


}

void ButcherTableau::setTableau( ButcherTableau::Name name )
{
   name_ = name;
   switch( name )
   {
   case ButcherTableau::GAUSS_LEGENDRE_4:
      NPOINTS_ = GAUSS_LEGENDRE_4_NPOINTS;
      TABLEAU_ = GAUSS_LEGENDRE_4_TABLEAU;
      break;

   case ButcherTableau::IMPLICIT_MIDPOINT_2:
      NPOINTS_ = IMPLICIT_MIDPOINT_2_NPOINTS;
      TABLEAU_ = IMPLICIT_MIDPOINT_2_TABLEAU;
      break;

   default:
   case ButcherTableau::RUNGE_KUTTA_2:
      NPOINTS_ = RUNGE_KUTTA_2_NPOINTS;
      TABLEAU_ = RUNGE_KUTTA_2_TABLEAU;

      break;

   case ButcherTableau::RUNGE_KUTTA_3:
      NPOINTS_ = RUNGE_KUTTA_3_NPOINTS;
      TABLEAU_ = RUNGE_KUTTA_3_TABLEAU;

      break;

   case ButcherTableau::HEUN:
      NPOINTS_ = HEUN_NPOINTS;
      TABLEAU_ = HEUN_TABLEAU;

      break;

   case ButcherTableau::RUNGE_KUTTA_4:
      NPOINTS_ = RUNGE_KUTTA_4_NPOINTS;
      TABLEAU_ = RUNGE_KUTTA_4_TABLEAU;

      break;

   case ButcherTableau::EULER:
      NPOINTS_ = EULER_1_NPOINTS;
      TABLEAU_ = EULER_1_TABLEAU;

   };
   setRows(name);
}

ButcherTableau::Name ButcherTableau::getName() const
{
   return name_;
}

}
