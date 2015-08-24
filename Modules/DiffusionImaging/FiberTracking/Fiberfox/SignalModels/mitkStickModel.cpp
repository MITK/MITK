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
    , m_BValue(1000)
{

}

template< class ScalarType >
StickModel< ScalarType >::~StickModel()
{

}

template< class ScalarType >
ScalarType StickModel< ScalarType >::SimulateMeasurement(unsigned int dir)
{
    ScalarType signal = 0;

    if (dir>=this->m_GradientList.size())
        return signal;

    this->m_FiberDirection.Normalize();

    GradientType g = this->m_GradientList[dir];
    ScalarType bVal = g.GetNorm(); bVal *= bVal;

    if (bVal>0.0001)
    {
        ScalarType dot = this->m_FiberDirection*g;
        signal = std::exp( -m_BValue * bVal * m_Diffusivity*dot*dot );
    }
    else
        signal = 1;

    return signal;
}

template< class ScalarType >
typename StickModel< ScalarType >::PixelType StickModel< ScalarType >::SimulateMeasurement()
{
    this->m_FiberDirection.Normalize();
    PixelType signal;
    signal.SetSize(this->m_GradientList.size());

    for( unsigned int i=0; i<this->m_GradientList.size(); i++)
    {
        GradientType g = this->m_GradientList[i];
        ScalarType bVal = g.GetNorm(); bVal *= bVal;

        if (bVal>0.0001)
        {
            ScalarType dot = this->m_FiberDirection*g;
            signal[i] = std::exp( -m_BValue * bVal * m_Diffusivity*dot*dot );
        }
        else
            signal[i] = 1;
    }

    return signal;
}
