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

#ifndef _MITK_ScalarModel_H
#define _MITK_ScalarModel_H

#include <mitkDiffusionSignalModel.h>

namespace mitk {

/**
  * \brief Generates direction independent diffusion measurement employing a scalar diffusion constant D: e^(-bD)
  *
  */

template< class ScalarType >
class ScalarModel : public DiffusionSignalModel< ScalarType >
{
public:

    ScalarModel();
    ~ScalarModel();

    typedef typename DiffusionSignalModel< ScalarType >::PixelType      PixelType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientType   GradientType;


    /** Actual signal generation **/
    PixelType SimulateMeasurement();

    void SetDiffusivity(float D) { m_Diffusivity = D; }
    void SetBvalue(float bValue) { m_BValue = bValue; }

protected:

    float                           m_Diffusivity;  ///< Scalar diffusion constant
    float                           m_BValue;       ///< b-value used to generate the artificial signal
};

}

#include "mitkScalarModel.cpp"

#endif

