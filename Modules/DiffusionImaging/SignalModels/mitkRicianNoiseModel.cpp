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
RicianNoiseModel< ScalarType >::RicianNoiseModel()
{
    m_RandGen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
    m_RandGen->SetSeed();
    this->m_NoiseVariance = 0;
    this->m_ScaleFactor = 0;
}

template< class ScalarType >
RicianNoiseModel< ScalarType >::~RicianNoiseModel()
{

}

template< class ScalarType >
void RicianNoiseModel< ScalarType >::AddNoise(PixelType& pixel)
{
    for( unsigned int i=0; i<pixel.Size(); i++)
    {
        double signal = pixel[i];
        pixel[i] = sqrt(pow(signal + this->m_ScaleFactor*m_RandGen->GetNormalVariate(0.0, this->m_NoiseVariance), 2) +  pow(this->m_ScaleFactor*m_RandGen->GetNormalVariate(0.0, this->m_NoiseVariance),2));
    }
}
