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

#ifndef _MITK_DotModel_H
#define _MITK_DotModel_H

#include <mitkDiffusionSignalModel.h>

namespace mitk {

/**
  * \brief Generates direction independent diffusion measurement employing a scalar diffusion constant d: e^(-bd)
  *
  */

template< class ScalarType >
class DotModel : public DiffusionSignalModel< ScalarType >
{
public:

    DotModel();
    ~DotModel();

    typedef typename DiffusionSignalModel< ScalarType >::PixelType      PixelType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientType   GradientType;

    /** Actual signal generation **/
    PixelType SimulateMeasurement();
    ScalarType SimulateMeasurement(int dir);

protected:

};

}

#include "mitkDotModel.cpp"

#endif

