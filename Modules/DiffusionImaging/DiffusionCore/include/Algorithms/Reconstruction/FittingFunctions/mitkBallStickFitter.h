#include <mitkAbstractFitter.h>
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

#ifndef _MITK_BallStickFitter_H
#define _MITK_BallStickFitter_H

#include <mitkAbstractFitter.h>

namespace mitk {

struct BallStickFitter: public AbstractFitter
{
public :

  BallStickFitter(unsigned int number_of_parameters, unsigned int number_of_measurements) :
    AbstractFitter(number_of_parameters, number_of_measurements)
  {

  }

  double penalty(const vnl_vector<double>& x)
  {
    double p = 0;

    if (x[0]<0 || x[0]>1)
      p += 10e6;
    if (x[1]<0)
      p += 10e6;

    return p;
  }

  void f(const vnl_vector<double>& x, vnl_vector<double>& fx) override
  {
    const double & f = x[0];
    const double & d = x[1];
    const double & theta = x[2];
    const double & phi = x[3];
    vnl_vector_fixed<double,3> dir;
    Sph2Cart(dir, theta, phi);

    for(auto s : weightedIndices)
    {
      double s_iso = S0 * std::exp(-bValues[s] * d);
      GradientDirectionType g = gradientDirections->GetElement(s);
      g.normalize();
      double dot = dot_product(g, dir);
      double s_aniso = S0 * std::exp(-bValues[s] * d * dot*dot );
      double approx = (1-f)*s_iso + f*s_aniso;
      const double factor = measurements[s] - approx;
      fx[s] = factor*factor + penalty(x);
    }
  }
};

}

#endif
