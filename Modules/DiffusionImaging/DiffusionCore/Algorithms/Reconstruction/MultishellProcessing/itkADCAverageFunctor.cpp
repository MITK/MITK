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

#include "itkADCAverageFunctor.h"

vnl_vector<double> itk::ADCAverageFunctor::operator()(const vnl_matrix<double> & SignalMatrix, const double & S0)
{

  vnl_vector<double> newSignal(SignalMatrix.rows());
  vnl_vector<double> averageADC(SignalMatrix.rows());
  vnl_matrix<double> ADCMatrix(SignalMatrix.rows(),SignalMatrix.cols());

  // Calculate ADC for each measurement
  for(unsigned int i = 0 ; i < SignalMatrix.rows(); ++i)
    for(unsigned int j = 0; j < SignalMatrix.cols(); ++j)
      ADCMatrix(i,j) = std::log(SignalMatrix(i,j) / S0) / (-m_BValueList[j]);// D = ln(S/S0)/-b

  // Calculate Average ADC
  for(unsigned int i = 0 ; i < SignalMatrix.rows(); ++i)
    averageADC.put(i,ADCMatrix.get_row(i).mean());

  // Calculate new Signal using Average ADC
  for(unsigned int i = 0 ; i < SignalMatrix.rows(); ++i)
    newSignal.put(i, S0 * std::exp(-m_TargetBvalue * averageADC.get(i)) ); // S = S0*exp(-b*D)

  // return new Signal Vector [S_1 S_2 ... S_N]
  return newSignal;
}
