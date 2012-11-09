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

template< class ScalarType >
class ScalarModel : public DiffusionSignalModel< ScalarType >
{
public:

    ScalarModel();
    ~ScalarModel();

    typedef typename DiffusionSignalModel< ScalarType >::PixelType      PixelType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientType   GradientType;

    PixelType SimulateMeasurement();
    void SetDiffusivity(float D) { m_Diffusivity = D; }
    void SetBvalue(float bValue) { m_BValue = bValue; }

protected:

    float                           m_Diffusivity;
    float                           m_BValue;
};

}

#include "mitkScalarModel.cpp"

#endif

