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
  // initialize vectors and matrices
  m_NewSignal(SignalMatrix.rows());
  m_AverageADC(SignalMatrix.rows());
  m_ADCMatrix(SignalMatrix);

  // Calculate ADC for each measurement
  for(unsigned int i = 0 ; i < SignalMatrix.rows(); ++i)
    for(unsigned int j = 0; j < SignalMatrix.cols(); ++j)
      m_ADCMatrix(i,j) = std::log(SignalMatrix(i,j)/S0) / m_BValueList[j];

  // Calculate Average ADC

  for(unsigned int i = 0 ; i < SignalMatrix.rows(); ++i)
    m_AverageADC.put(i,m_ADCMatrix.get_row(i).mean());

  // Calculate new Signal using Average ADC
  for(unsigned int i = 0 ; i < SignalMatrix.rows(); ++i)
    m_NewSignal.put(i, S0 * std::exp(-m_TargetBvalue * m_AverageADC.get(i)) );

  // return new Signal Vector [S_1 S_2 ... S_N]
  return m_NewSignal;
}
