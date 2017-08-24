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

#ifndef _MITK_MultiTensorFitter_H
#define _MITK_MultiTensorFitter_H

#include <mitkAbstractFitter.h>

namespace mitk {

class MultiTensorFitter: public AbstractFitter
{
public:

  MultiTensorFitter(unsigned int number_of_tensors=2, unsigned int number_of_measurements=1) :
    AbstractFitter(check(number_of_tensors), number_of_measurements)
  {
    num_tensors = number_of_tensors;
  }

  int num_tensors;

  int check(int n)
  {
    if (n==1)
      return 6;
    return n*7;
  }

  vnl_vector_fixed<double,3> GetLargestEv(TensorType& tensor)
  {
    TensorType::EigenValuesArrayType eigenvalues;
    TensorType::EigenVectorsMatrixType eigenvectors;
    tensor.ComputeEigenAnalysis(eigenvalues, eigenvectors);
    vnl_vector_fixed<double,3> ev;
    ev[0] = eigenvectors(2, 0);
    ev[1] = eigenvectors(2, 1);
    ev[2] = eigenvectors(2, 2);
    if (ev.magnitude()>mitk::eps)
      ev.normalize();
    else
      ev.fill(0.0);

    return ev;
  }

  double penalty(const vnl_vector<double>& x)
  {
    double p = 0;

    if (num_tensors>1)
    {
      double f = 0;
      for (int i=0; i<num_tensors; i++)
      {
        if (x[6+i*7]<0)
          p += 10e6;
        f += x[6+i*7];
      }
      p += 10e7*fabs(1-f);
    }

    return p;
  }

  void f(const vnl_vector<double>& x, vnl_vector<double>& fx) override
  {
    int elements = 7;
    for(auto s : weightedIndices)
    {
      GradientDirectionType g = gradientDirections->GetElement(s);
      g.normalize();

      itk::DiffusionTensor3D< double > S;
      S[0] = g[0]*g[0];
      S[1] = g[1]*g[0];
      S[2] = g[2]*g[0];
      S[3] = g[1]*g[1];
      S[4] = g[2]*g[1];
      S[5] = g[2]*g[2];

      double approx = 0;
      for (int i=0; i<num_tensors; i++)
      {
        double D = x[0+i*elements]*S[0] + x[1+i*elements]*S[1] + x[2+i*elements]*S[2] +
                   x[1+i*elements]*S[1] + x[3+i*elements]*S[3] + x[4+i*elements]*S[4] +
                   x[2+i*elements]*S[2] + x[4+i*elements]*S[4] + x[5+i*elements]*S[5];
        double signal = S0 * std::exp ( -bValues[s] * D );
        if (num_tensors>1 && x.size()>6)
          signal *= x[elements-1+i*elements];
        approx += signal;
      }

      const double factor = measurements[s] - approx;

      fx[s] = factor*factor + penalty(x);
    }
  }

};

}

#endif
