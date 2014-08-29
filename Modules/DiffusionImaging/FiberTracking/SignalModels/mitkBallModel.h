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

#ifndef _MITK_BallModel_H
#define _MITK_BallModel_H

#include <mitkDiffusionSignalModel.h>

namespace mitk {

/**
  * \brief Generates direction independent diffusion measurement employing a scalar diffusion constant d: e^(-bd)
  *
  */

template< class ScalarType = double >
class BallModel : public DiffusionSignalModel< ScalarType >
{
public:

    BallModel();
    ~BallModel();

    typedef typename DiffusionSignalModel< ScalarType >::PixelType      PixelType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientType   GradientType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientListType   GradientListType;


    /** Actual signal generation **/
    PixelType SimulateMeasurement();
    ScalarType SimulateMeasurement(unsigned int dir);

    void SetDiffusivity(ScalarType D) { m_Diffusivity = D; }
    ScalarType GetDiffusivity() { return m_Diffusivity; }
    void SetBvalue(ScalarType bValue) { m_BValue = bValue; }

    void SetFiberDirection(GradientType fiberDirection){ this->m_FiberDirection = fiberDirection; }
    void SetGradientList(GradientListType gradientList) { this->m_GradientList = gradientList; }

protected:

    ScalarType  m_Diffusivity;  ///< Scalar diffusion constant
    ScalarType  m_BValue;       ///< b-value used to generate the artificial signal
};

}

#include "mitkBallModel.cpp"

#endif

