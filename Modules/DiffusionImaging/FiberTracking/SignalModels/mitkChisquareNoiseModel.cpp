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
#include <mitkChisquareNoiseModel.h>

using namespace mitk;

template< class ScalarType >
ChisquareNoiseModel< ScalarType >::ChisquareNoiseModel()
{
    this->m_NoiseVariance = 0;
    m_Distribution = std::chi_squared_distribution<double>(4.0);
}

template< class ScalarType >
ChisquareNoiseModel< ScalarType >::~ChisquareNoiseModel()
{

}

template< class ScalarType >
void ChisquareNoiseModel< ScalarType >::AddNoise(PixelType& pixel)
{
    for( unsigned int i=0; i<pixel.Size(); i++)
    {
        pixel[i] += m_Distribution(m_RandGen);
    }
}
