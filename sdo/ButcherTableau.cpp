#include "ButcherTableau.hpp"
#include <cmath>

using std::sqrt;

static const int RUNGE_KUTTA_2_NPOINTS = 2;

static const double RUNGE_KUTTA_2_TABLEAU[] =
{
   0.0, 0.0,
   0.5, 0.0,
   0.0, 1.0
};

static const int RUNGE_KUTTA_3_NPOINTS = 3;

static const double RUNGE_KUTTA_3_TABLEAU[] =
{
   0.0, 0.0, 0.0,
   0.5, 0.0, 0.0,
   -1.0, 2.0, 0.0,
   1.0 / 6.0, 2.0 / 3.0, 1.0 / 6.0
};


static const int RUNGE_KUTTA_4_NPOINTS = 4;

static const double RUNGE_KUTTA_4_TABLEAU[] =
{
   0.0, 0.0, 0.0, 0.0,
   0.5, 0.0, 0.0, 0.0,
   0.0, 0.5, 0.0, 0.0,
   0.0, 0.0, 1.0, 0.0,
   1.0 / 6.0, 1.0 / 3.0, 1.0 / 3.0, 1.0 / 6.0
};

static const int IMPLICIT_MIDPOINT_2_NPOINTS = 1;

static const double IMPLICIT_MIDPOINT_2_TABLEAU[] =
{
   0.5,
   1.0
};

static const int  GAUSS_LEGENDRE_4_NPOINTS = 2;

static const double GAUSS_LEGENDRE_4_TABLEAU[] =
{
   0.25,                         0.25 - sqrt( 3 ) / 6,
   0.25 + sqrt( 3 ) / 6, 0.25,
   0.5,                          0.5
};

static const int EULER_1_NPOINTS = 1;

static const double EULER_1_TABLEAU[] =
{
   0.0,
   1.0
};

namespace sdo {

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

   case ButcherTableau::RUNGE_KUTTA_4:
      NPOINTS_ = RUNGE_KUTTA_4_NPOINTS;
      TABLEAU_ = RUNGE_KUTTA_4_TABLEAU;
      break;

   case ButcherTableau::EULER:
      NPOINTS_ = EULER_1_NPOINTS;
      TABLEAU_ = EULER_1_TABLEAU;
   };
}

ButcherTableau::Name ButcherTableau::getName() const
{
   return name_;
}

}
