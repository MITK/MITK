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

template< class ScalarType = double >
class StickModel : public DiffusionSignalModel< ScalarType >
{
public:

    StickModel();
    ~StickModel();

    typedef typename DiffusionSignalModel< ScalarType >::PixelType      PixelType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientType   GradientType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientListType   GradientListType;

    /** Actual signal generation **/
    PixelType SimulateMeasurement();
    ScalarType SimulateMeasurement(unsigned int dir);

    void SetBvalue(ScalarType bValue) { m_BValue = bValue; }                     ///< b-value used to generate the artificial signal
    void SetDiffusivity(ScalarType diffusivity) { m_Diffusivity = diffusivity; } ///< Scalar diffusion constant
    ScalarType GetDiffusivity() { return m_Diffusivity; }

    void SetFiberDirection(GradientType fiberDirection){ this->m_FiberDirection = fiberDirection; }
    void SetGradientList(GradientListType gradientList) { this->m_GradientList = gradientList; }

protected:

    ScalarType   m_Diffusivity;  ///< Scalar diffusion constant
    ScalarType   m_BValue;       ///< b-value used to generate the artificial signal
};

}

#include "mitkStickModel.cpp"

#endif

