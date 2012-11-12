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

/**
  * \brief Abstract class for diffusion signal models
  *
  */

template< class ScalarType >
class DiffusionSignalModel
{
public:

    DiffusionSignalModel() : m_SignalScale(200){}
    ~DiffusionSignalModel(){}

    typedef itk::VariableLengthVector< ScalarType > PixelType;
    typedef itk::Vector<double,3>                   GradientType;
    typedef std::vector<GradientType>               GradientListType;

    /** Realizes actual signal generation. Has to be implemented in subclass. **/
    virtual PixelType SimulateMeasurement() = 0;
    void SetFiberDirection(GradientType fiberDirection){ m_FiberDirection = fiberDirection; }
    void SetGradientList(GradientListType gradientList) { m_GradientList = gradientList; }
    void SetSignalScale(ScalarType signalScale) { m_SignalScale = signalScale; }

    ScalarType GetSignalScale() { return m_SignalScale; }
    int GetNumGradients(){ return m_GradientList.size(); }

protected:

    GradientType        m_FiberDirection;   ///< Needed to generate anisotropc signal to determin direction of anisotropy
    GradientListType    m_GradientList;     ///< Diffusion gradient direction container
    ScalarType          m_SignalScale;      ///< Scaling factor for signal value
};

}

#endif

