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

template< class ScalarType >
class RicianNoiseModel : public DiffusionNoiseModel< ScalarType >
{
public:

    RicianNoiseModel();
    ~RicianNoiseModel();

    typedef typename DiffusionNoiseModel< ScalarType >::PixelType      PixelType;

    /** Adds rician noise to the input pixel **/
    void AddNoise(PixelType& pixel);

protected:

    itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer m_RandGen;

};

}

#include "mitkRicianNoiseModel.cpp"

#endif

