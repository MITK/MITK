/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkDiffusionFunctionCollection.h"
#include <math.h>
#include "mitkVector.h"

// for Windows
#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif

// Namespace ::SH
#include <boost/math/special_functions/legendre.hpp>
#include <boost/math/special_functions/spherical_harmonic.hpp>
#include <boost/version.hpp>

// Namespace ::vnl_function
#include "vnl/vnl_vector.h"

//------------------------- SH-function ------------------------------------

double mitk::sh::factorial(int number) {
    if(number <= 1) return 1;
    double result = 1.0;
    for(int i=1; i<=number; i++)
        result *= i;
    return result;
}

void mitk::sh::Cart2Sph(double x, double y, double z, double *cart)
{
    double phi, th, rad;
    rad = sqrt(x*x+y*y+z*z);
    if( rad < mitk::eps )
    {
        th = M_PI/2;
        phi = M_PI/2;
    }
    else
    {
        th = acos(z/rad);
        phi = atan2(y, x);
    }
    cart[0] = phi;
    cart[1] = th;
    cart[2] = rad;
}

double mitk::sh::legendre0(int l)
{
    if( l%2 != 0 )
    {
        return 0;
    }
    else
    {
        double prod1 = 1.0;
        for(int i=1;i<l;i+=2) prod1 *= i;
        double prod2 = 1.0;
        for(int i=2;i<=l;i+=2) prod2 *= i;
        return pow(-1.0,l/2.0)*(prod1/prod2);
    }
}


double mitk::sh::Yj(int m, int l, double theta, double phi)
{
  if (m<0)
      return sqrt(2.0)*::boost::math::spherical_harmonic_r(l, -m, theta, phi);
  else if (m==0)
      return ::boost::math::spherical_harmonic_r(l, m, theta, phi);
  else
      return pow(-1.0,m)*sqrt(2.0)*::boost::math::spherical_harmonic_i(l, m, theta, phi);

  return 0;
}



