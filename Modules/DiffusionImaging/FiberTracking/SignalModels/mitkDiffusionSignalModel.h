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

#include <MitkFiberTrackingExports.h>
#include <itkVariableLengthVector.h>
#include <itkVector.h>
#include <itkImage.h>
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

    DiffusionSignalModel()
        : m_T2(100)
    {}
    ~DiffusionSignalModel(){}

    typedef itk::Image<double, 3>                   ItkDoubleImgType;
    typedef itk::VariableLengthVector< ScalarType > PixelType;
    typedef itk::Vector<double,3>                   GradientType;
    typedef std::vector<GradientType>               GradientListType;

    /** Realizes actual signal generation. Has to be implemented in subclass. **/
    virtual PixelType SimulateMeasurement() = 0;
    virtual ScalarType SimulateMeasurement(unsigned int dir) = 0;

    virtual void SetFiberDirection(GradientType fiberDirection) = 0;
    virtual void SetGradientList(GradientListType gradientList) = 0;
    void SetT2(double T2) { m_T2 = T2; }
    void SetVolumeFractionImage(ItkDoubleImgType::Pointer img){ m_VolumeFractionImage = img; }

    ItkDoubleImgType::Pointer GetVolumeFractionImage(){ return m_VolumeFractionImage; }
    GradientType GetGradientDirection(int i) { return m_GradientList.at(i); }
    double GetT2() { return m_T2; }

protected:

    GradientType                m_FiberDirection;       ///< Needed to generate anisotropc signal to determin direction of anisotropy
    GradientListType            m_GradientList;         ///< Diffusion gradient direction container
    double                      m_T2;                   ///< Tissue specific relaxation time
    ItkDoubleImgType::Pointer   m_VolumeFractionImage;  ///< Tissue specific volume fraction for each voxel (only relevant for non fiber compartments)
};

}

#endif

