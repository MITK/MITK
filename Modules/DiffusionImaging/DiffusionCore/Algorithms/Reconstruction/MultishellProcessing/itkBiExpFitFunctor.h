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

#ifndef _itk_BiExpFitFunctor_h_
#define _itk_BiExpFitFunctor_h_

#include "itkDWIVoxelFunctor.h"
#include <math.h>

// vnl include
#include "vnl/vnl_least_squares_function.h"
#include "vnl/algo/vnl_levenberg_marquardt.h"
#include "vnl/vnl_math.h"

namespace itk
{

class DiffusionCore_EXPORT BiExpFitFunctor : public DWIVoxelFunctor
{
public:
  BiExpFitFunctor(){}
  ~BiExpFitFunctor(){}

  typedef BiExpFitFunctor                         Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef DWIVoxelFunctor                         Superclass;
  /** Method for creation through the object factory. */
  itkNewMacro(Self)
  /** Runtime information support. */
  itkTypeMacro(BiExpFitFunctor, DWIVoxelFunctor)

  void operator()(vnl_matrix<double> & newSignal,const vnl_matrix<double> & SignalMatrix, const double & S0);

  void setTargetBValue(const double & targetBValue){m_TargetBvalue = targetBValue;}
  void setListOfBValues(const vnl_vector<double> & BValueList){m_BValueList = BValueList;}

protected:
  double m_TargetBvalue;
  vnl_vector<double> m_BValueList;

  /**
   * \brief The lestSquaresFunction struct for Non-Linear-Least-Squres fit of Biexponential model
   */
  struct lestSquaresFunction: public vnl_least_squares_function
  {

    void set_measurements(const vnl_vector<double>& x)
    {
      measurements.set_size(x.size());
      measurements.copy_in(x.data_block());
    }

    void set_bvalues(const vnl_vector<double>& x)
    {
      bValueVector.set_size(x.size());
      bValueVector.copy_in(x.data_block());
    }

    void set_reference_measurement(const double & x)
    {
      S0 = x;
    }

    vnl_vector<double> measurements;
    vnl_vector<double> bValueVector;
    double S0;
    int N;

    lestSquaresFunction(unsigned int number_of_measurements) :
      vnl_least_squares_function(3 /*number of unknowns [ ADC_slow ADC_fast lambda]*/, number_of_measurements, no_gradient)
    {
      N = get_number_of_residuals();
    }

    void f(const vnl_vector<double>& x, vnl_vector<double>& fx) {

      const double & ADC_slow = x[0];
      const double & ADC_fast = x[1];
      const double & lambda = x[2];

      const vnl_vector<double> & b = bValueVector;

      for(int s=0; s<N; s++)
      {
        double approx = lambda * std::exp(-b[s] * ADC_slow) + (1-lambda) * std::exp(-b[s] * ADC_fast);
        fx[s] = vnl_math_abs( measurements[s] - approx*S0 );
      }

    }
  };

};

}

#endif
