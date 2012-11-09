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

template< class ScalarType >
class  MitkDiffusionImaging_EXPORT DiffusionNoiseModel
{
public:

    DiffusionNoiseModel(){}
    ~DiffusionNoiseModel(){}

    typedef itk::VariableLengthVector< ScalarType > PixelType;

    virtual void AddNoise(PixelType& pixel) = 0;
    void SetScaleFactor(ScalarType scale){ m_ScaleFactor = scale; }
    void SetNoiseVariance(double var){ m_NoiseVariance = var; }

protected:

    ScalarType  m_ScaleFactor;
    double      m_NoiseVariance;
};

}

#endif

