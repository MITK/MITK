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
typename StickModel< ScalarType >::PixelType StickModel< ScalarType >::SimulateMeasurement()
{
    PixelType signal;
    signal.SetSize(this->m_GradientList.size());

    for( unsigned int i=0; i<this->m_GradientList.size(); i++)
    {
        GradientType g = this->m_GradientList[i];
        double bVal = g.GetNorm(); bVal *= bVal;

        if (bVal>0.0001)
        {
            double dot = this->m_FiberDirection*g;
            signal[i] = exp( -m_BValue * bVal * m_Diffusivity*dot*dot );
        }
        else
            signal[i] = 1;
    }

    return signal;
}
