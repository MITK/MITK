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

#ifndef _itk_ADCAverageFunctor_h_
#define _itk_ADCAverageFunctor_h_

#include "itkDWIVoxelFunctor.h"
#include <math.h>

namespace itk
{

class ADCAverageFunctor : public DWIVoxelFunctor
{
public:

  /**
   * @brief ADCAverageFunctor
   * @param targetBValue b-Value for the new singleshell
   * @param BValueList containing the multishell b-Values (e.g. [b=1000, b=2000, b=3000])
   */
  ADCAverageFunctor(const double & targetBValue, const vnl_vector<double> & BValueList)
    :m_TargetBvalue(targetBValue),m_BValueList(BValueList){}
  ~ADCAverageFunctor(){}

  /**
   * @brief operator ()
   * @param SignalMatrix is a NxM matrix (N = Number of gradients; M = Number of Shells)
   * @param S0 is the reference signal (Signal for b=0)
   * @return 1xP Signal vector containing the new signal (e.g. [S_1 S_2 S_3 ... S_N] -> only diffusion weighted signal)
   */
  vnl_vector<double> operator()(const vnl_matrix<double> & SignalMatrix, const double & S0);

protected:
  double m_TargetBvalue;
  vnl_vector<double> m_BValueList;
  vnl_vector<double> m_NewSignal;
  vnl_vector<double> m_AverageADC;
  vnl_matrix<double> m_ADCMatrix;

};

}

#endif
