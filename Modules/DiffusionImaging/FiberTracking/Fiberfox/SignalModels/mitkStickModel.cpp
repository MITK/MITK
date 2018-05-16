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
#include <vnl/vnl_cross.h>
#include <vnl/vnl_quaternion.h>
#include <mitkStickModel.h>

using namespace mitk;

template< class ScalarType >
StickModel< ScalarType >::StickModel()
  : m_Diffusivity(0.001)
{

}

template< class ScalarType >
StickModel< ScalarType >::~StickModel()
{

}

template< class ScalarType >
ScalarType StickModel< ScalarType >::SimulateMeasurement(unsigned int dir, GradientType& fiberDirection)
{
  ScalarType signal = 0;

  if (dir>=this->m_GradientList.size())
    return signal;

  GradientType g = this->m_GradientList[dir];
  if (g.GetNorm()>0.0001)
  {
    ScalarType dot = fiberDirection*g;
    signal = std::exp( -this->m_BValue*m_Diffusivity*dot*dot ); // skip * bVal becaus bVal is already encoded in the dot product (norm of g encodes b-value relative to baseline b-value m_BValue)
  }
  else
    signal = 1;

  return signal;
}

template< class ScalarType >
typename StickModel< ScalarType >::PixelType StickModel< ScalarType >::SimulateMeasurement(GradientType &fiberDirection)
{
  PixelType signal;
  signal.SetSize(this->m_GradientList.size());

  for( unsigned int i=0; i<this->m_GradientList.size(); i++)
  {
    GradientType g = this->m_GradientList[i];
    if (g.GetNorm()>0.0001)
    {
      ScalarType dot = fiberDirection*g;
      signal[i] = std::exp( -this->m_BValue*m_Diffusivity*dot*dot ); // skip * bVal becaus bVal is already encoded in the dot product (norm of g encodes b-value relative to baseline b-value m_BValue)
    }
    else
      signal[i] = 1;
  }

  return signal;
}
