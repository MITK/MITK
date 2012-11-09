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

#ifndef _MITK_DiffusionSignalModel_H
#define _MITK_DiffusionSignalModel_H

#include <MitkDiffusionImagingExports.h>
#include <itkVariableLengthVector.h>
#include <itkVector.h>
#include <vnl/vnl_vector_fixed.h>

namespace mitk {

template< class ScalarType >
class DiffusionSignalModel
{
public:

    DiffusionSignalModel(){}
    ~DiffusionSignalModel(){}

    typedef itk::VariableLengthVector< ScalarType > PixelType;
    typedef itk::Vector<double,3>                   GradientType;
    typedef std::vector<GradientType>               GradientListType;

    virtual PixelType SimulateMeasurement() = 0;
    void SetFiberDirection(vnl_vector_fixed<double, 3> fiberDirection){ m_FiberDirection = fiberDirection; }
    void SetGradientList(GradientListType gradientList) { m_GradientList = gradientList; }
    int GetNumGradients(){ return m_GradientList.size(); }

protected:

    vnl_vector_fixed<double, 3>     m_FiberDirection;
    GradientListType                m_GradientList;

};

}

#endif

