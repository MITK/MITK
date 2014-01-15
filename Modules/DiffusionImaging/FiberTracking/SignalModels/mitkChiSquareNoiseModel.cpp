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
#include <mitkChiSquareNoiseModel.h>

using namespace mitk;

template< class ScalarType >
ChiSquareNoiseModel< ScalarType >::ChiSquareNoiseModel()
{
    m_RandGen.seed();
}

template< class ScalarType >
ChiSquareNoiseModel< ScalarType >::~ChiSquareNoiseModel()
{

}

template< class ScalarType >
void ChiSquareNoiseModel< ScalarType >::SetSeed(int seed)
{
    if (seed>=0)
        m_RandGen.seed(seed);
    else
        m_RandGen.seed();
}

template< class ScalarType >
void ChiSquareNoiseModel< ScalarType >::AddNoise(PixelType& pixel)
{
    for( unsigned int i=0; i<pixel.Size(); i++)
        pixel[i] += (ScalarType)(m_Distribution(m_RandGen)-m_Distribution.n());
}
