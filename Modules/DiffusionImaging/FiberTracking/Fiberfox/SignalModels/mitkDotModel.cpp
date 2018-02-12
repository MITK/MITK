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
#include <mitkDotModel.h>

using namespace mitk;

template< class ScalarType >
DotModel< ScalarType >::DotModel()
{

}

template< class ScalarType >
DotModel< ScalarType >::~DotModel()
{

}

template< class ScalarType >
ScalarType DotModel< ScalarType >::SimulateMeasurement(unsigned int /*dir*/, GradientType& )
{
    return 1;
}

template< class ScalarType >
typename DotModel< ScalarType >::PixelType DotModel< ScalarType >::SimulateMeasurement(GradientType& )
{
    PixelType signal;
    signal.SetSize(this->m_GradientList.size());
    signal.Fill(1);
    return signal;
}
