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

#ifndef _MITK_ChisquareNoiseModel_H
#define _MITK_ChisquareNoiseModel_H

#include <mitkDiffusionNoiseModel.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>
#include <random>

namespace mitk {

/**
  * \brief Implementation of noise following a rician distribution
  *
  */

template< class ScalarType >
class ChisquareNoiseModel : public DiffusionNoiseModel< ScalarType >
{
public:

    ChisquareNoiseModel();
    ~ChisquareNoiseModel();

    typedef typename DiffusionNoiseModel< ScalarType >::PixelType      PixelType;

    /** Adds rician noise to the input pixel **/
    void AddNoise(PixelType& pixel);

protected:

    std::default_random_engine              m_RandGen;
    std::chi_squared_distribution<double>   m_Distribution;
};

}

#include "mitkChisquareNoiseModel.cpp"

#endif

