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

#include <MitkFiberTrackingExports.h>
#include <itkVariableLengthVector.h>
#include <itkVector.h>
#include <vnl/vnl_vector_fixed.h>
#include <boost/random.hpp>

namespace mitk {

/**
  * \brief Abstract class for diffusion noise models
  *
  */

template< class ScalarType = double >
class DiffusionNoiseModel
{
public:

    DiffusionNoiseModel(){}
    virtual ~DiffusionNoiseModel(){}

    typedef itk::VariableLengthVector< ScalarType > PixelType;

    /** Adds noise according to model to the input pixel. Has to be implemented in subclass. **/
    virtual void AddNoise(PixelType& pixel) = 0;

    /** Seed for random generator. Has to be implemented in subclass. **/
    virtual void SetSeed(int seed) = 0;

    virtual double GetNoiseVariance() = 0;
    virtual void SetNoiseVariance(double var) = 0;

protected:

};

}

#endif

