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

#ifndef _MITK_ChiSquareNoiseModel_H
#define _MITK_ChiSquareNoiseModel_H

#include <mitkDiffusionNoiseModel.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

namespace mitk {

/**
  * \brief Implementation of noise following a chi-squared distribution
  *
  */

template< class ScalarType >
class ChiSquareNoiseModel : public DiffusionNoiseModel< ScalarType >
{
public:

    ChiSquareNoiseModel();
    ~ChiSquareNoiseModel();

    typedef typename DiffusionNoiseModel< ScalarType >::PixelType      PixelType;

    /** Adds rician noise to the input pixel **/
    void AddNoise(PixelType& pixel);

    void SetNoiseVariance(double var){ m_Distribution = boost::random::chi_squared_distribution<double>(var/2); }
    double GetNoiseVariance(){ return m_Distribution.n()*2; }
    void SetSeed(int seed); ///< seed for random number generator

protected:

    boost::random::mt19937 m_RandGen;
    boost::random::chi_squared_distribution<double> m_Distribution;
};

}

#include "mitkChiSquareNoiseModel.cpp"

#endif

