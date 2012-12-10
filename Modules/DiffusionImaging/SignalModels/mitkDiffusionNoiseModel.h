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

#ifndef _MITK_DiffusionNoiseModel_H
#define _MITK_DiffusionNoiseModel_H

#include <MitkDiffusionImagingExports.h>
#include <itkVariableLengthVector.h>
#include <itkVector.h>
#include <vnl/vnl_vector_fixed.h>

namespace mitk {

/**
  * \brief Abstract class for diffusion noise models
  *
  */

template< class ScalarType >
class DiffusionNoiseModel
{
public:

    DiffusionNoiseModel(){}
    ~DiffusionNoiseModel(){}

    typedef itk::VariableLengthVector< ScalarType > PixelType;

    /** Adds noise according to model to the input pixel. Has to be implemented in subclass. **/
    virtual void AddNoise(PixelType& pixel) = 0;

    void SetNoiseVariance(double var){ m_NoiseVariance = var; }

protected:

    double      m_NoiseVariance;    ///< variance of underlying distribution
};

}

#endif

