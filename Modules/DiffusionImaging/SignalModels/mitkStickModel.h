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

#ifndef _MITK_StickModel_H
#define _MITK_StickModel_H

#include <mitkDiffusionSignalModel.h>

namespace mitk {

/**
  * \brief Generates the diffusion signal using an idealised cylinder with zero radius: e^(-bd(ng)²)
  *
  */

template< class ScalarType >
class StickModel : public DiffusionSignalModel< ScalarType >
{
public:

    StickModel();
    ~StickModel();

    typedef typename DiffusionSignalModel< ScalarType >::PixelType      PixelType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientType   GradientType;

    /** Actual signal generation **/
    PixelType SimulateMeasurement();

    void SetBvalue(float bValue) { m_BValue = bValue; }                     ///< b-value used to generate the artificial signal
    void SetDiffusivity(float diffusivity) { m_Diffusivity = diffusivity; } ///< Scalar diffusion constant

protected:

    float   m_BValue;       ///< b-value used to generate the artificial signal
    float   m_Diffusivity;  ///< Scalar diffusion constant
};

}

#include "mitkStickModel.cpp"

#endif

