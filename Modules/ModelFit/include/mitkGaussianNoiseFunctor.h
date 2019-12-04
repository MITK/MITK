/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKGAUSSIANNOISEFUNCTOR_H
#define MITKGAUSSIANNOISEFUNCTOR_H

#include <iostream>
#include <cstdlib>

namespace mitk
{
  template <class TInputPixel, class TOutputPixel>
  class GaussianNoiseFunctor
  {

  public:
    GaussianNoiseFunctor(): m_Mu(0), m_Sigma(0) {};
    ~GaussianNoiseFunctor() {};

    void SetMean(double mu)
    {
      this->m_Mu = mu;
    }
    void SetSigma(double sigma)
    {
      this->m_Sigma = sigma;
    }

    bool operator!=(const GaussianNoiseFunctor& other) const
    {
      return !(*this == other);
    }

    bool operator==(const GaussianNoiseFunctor& other) const
    {
      return (this->m_Mu == other.m_Mu) && (this->m_Sigma == other.m_Sigma) ;
    }

    inline TOutputPixel operator()(const TInputPixel& value) const
    {
      double n = noise(this->m_Mu, this->m_Sigma);

      TOutputPixel result = value + n;

      return result;
    }

  private:
    double m_Mu, m_Sigma;


    /** @todo #2 Better function?
     * This function is meant to generate a random number from a normal distribution with the passed mean and standard deviation.
     * I found this code online, under c++11 there is supposed to be a std - libary for that: std::normal_distribution
     **/
    inline double noise(double mu,  double sigma) const
    {
      double u1, u2, W, mult;
      static double x1, x2;
      static int i = 0;

      if (i == 1)
      {
        i = !i;
        return (mu + sigma * (double)x2);
      }

      do
      {
        u1 = -1 + (static_cast <double>(rand()) / static_cast <double>(RAND_MAX)) * 2;
        u2 = -1 + (static_cast <double>(rand()) / static_cast <double>(RAND_MAX)) * 2;
        W = u1 * u1 + u2 * u2;
      }
      while (W >= 1 || W == 0);

      mult = sqrt((-2 * log(W)) / W);
      x1 = u1 * mult;
      x2 = u2 * mult;

      i = !i;

      return (mu + sigma * (double)x1);

    }
  };
}


#endif // MITKGAUSSIANNOISEFUNCTOR_H
