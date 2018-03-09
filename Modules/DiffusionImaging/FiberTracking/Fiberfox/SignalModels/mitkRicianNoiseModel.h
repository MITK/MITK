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

#ifndef _MITK_RicianNoiseModel_H
#define _MITK_RicianNoiseModel_H

#include <mitkDiffusionNoiseModel.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

namespace mitk {

/**
  * \brief Implementation of noise following a rician distribution
  *
  */

template< class ScalarType = double >
class RicianNoiseModel : public DiffusionNoiseModel< ScalarType >
{
public:

    RicianNoiseModel();
    ~RicianNoiseModel() override;

    typedef typename DiffusionNoiseModel< ScalarType >::PixelType      PixelType;

    /** Adds rician noise to the input pixel **/
    void AddNoise(PixelType& pixel) override;

    void SetNoiseVariance(double var) override{ m_NoiseVariance = var; }
    double GetNoiseVariance() override{ return m_NoiseVariance; }
    void SetSeed(int seed) override; ///< Set seed for random number generator

protected:

    itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer m_RandGen;
    double      m_NoiseVariance;    ///< variance of underlying distribution

};

}

#include "mitkRicianNoiseModel.cpp"

#endif

