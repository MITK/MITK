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
#include <itkMersenneTwisterRandomVariateGenerator.h>
#include <vnl/vnl_vector_fixed.h>

namespace mitk {

/**
  * \brief Abstract class for diffusion signal models
  *
  */

template< class ScalarType = double >
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
    typedef itk::Statistics::MersenneTwisterRandomVariateGenerator          ItkRandGenType;

    /** Realizes actual signal generation. Has to be implemented in subclass. **/
    virtual PixelType SimulateMeasurement() = 0;
    virtual ScalarType SimulateMeasurement(unsigned int dir) = 0;

    virtual void SetFiberDirection(GradientType fiberDirection) = 0;
    GradientType GetFiberDirection(){ return m_FiberDirection; }

    virtual void SetGradientList(GradientListType gradientList) = 0;
    GradientListType GetGradientList(){ return m_GradientList; }
    GradientType GetGradientDirection(int i) { return m_GradientList.at(i); }

    void SetT2(double T2) { m_T2 = T2; }
    double GetT2() { return m_T2; }

    void SetVolumeFractionImage(ItkDoubleImgType::Pointer img){ m_VolumeFractionImage = img; }
    ItkDoubleImgType::Pointer GetVolumeFractionImage(){ return m_VolumeFractionImage; }

    void SetRandomGenerator(ItkRandGenType::Pointer randgen){ m_RandGen = randgen; }
    ItkRandGenType::Pointer GetRandomGenerator(){ return m_RandGen; }

    void SetSeed(int s)     ///< set seed for random generator
    {
        if (m_RandGen.IsNull())
            m_RandGen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
        m_RandGen->SetSeed(s);
    }

    unsigned int                m_CompartmentId;        ///< GUI flag. Which compartment is this model assigned to?

protected:

    GradientType                m_FiberDirection;       ///< Needed to generate anisotropc signal to determin direction of anisotropy
    GradientListType            m_GradientList;         ///< Diffusion gradient direction container
    double                      m_T2;                   ///< Tissue specific relaxation time
    ItkDoubleImgType::Pointer   m_VolumeFractionImage;  ///< Tissue specific volume fraction for each voxel (only relevant for non fiber compartments)
    ItkRandGenType::Pointer     m_RandGen;              ///< Random number generator
};

}

#endif

