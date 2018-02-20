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
#include <mitkDiffusionPropertyHelper.h>

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
        , m_T1(0)
        , m_BValue(1000)
    {}
    ~DiffusionSignalModel(){}

    typedef itk::Image<double, 3>                   ItkDoubleImgType;
    typedef itk::VariableLengthVector< ScalarType > PixelType;
    typedef itk::Vector<double,3>                   GradientType;
    typedef std::vector<GradientType>               GradientListType;
    typedef itk::Statistics::MersenneTwisterRandomVariateGenerator          ItkRandGenType;
    typedef mitk::DiffusionPropertyHelper           DPH;

    /** Realizes actual signal generation. Has to be implemented in subclass. **/
    virtual PixelType SimulateMeasurement(GradientType& fiberDirection) = 0;
    virtual ScalarType SimulateMeasurement(unsigned int dir, GradientType& fiberDirection) = 0;

    void SetGradientList(DPH::GradientDirectionsContainerType* gradients)
    {
      m_GradientList.clear();
      for ( unsigned int i=0; i<gradients->Size(); ++i )
      {
        DPH::GradientDirectionType g_vnl = gradients->GetElement(i);
        GradientType g_itk;
        g_itk[0] = g_vnl[0];
        g_itk[1] = g_vnl[1];
        g_itk[2] = g_vnl[2];
        m_GradientList.push_back(g_itk);
      }
    }

    void SetGradientList(GradientListType gradientList) { this->m_GradientList = gradientList; }
    GradientListType GetGradientList(){ return m_GradientList; }
    GradientType GetGradientDirection(int i) { return m_GradientList.at(i); }

    void SetT2(double T2) { m_T2 = T2; }
    double GetT2() { return m_T2; }

    void SetT1(double T1) { m_T1 = T1; }
    double GetT1() { return m_T1; }

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

    void SetBvalue(double bValue) { m_BValue = bValue; }                     ///< b-value used to generate the artificial signal
    double GetBvalue() { return m_BValue; }

    unsigned int                m_CompartmentId;        ///< GUI flag. Which compartment is this model assigned to?

protected:

    GradientListType            m_GradientList;         ///< Diffusion gradient direction container
    double                      m_T2;                   ///< Tissue specific transversal relaxation time
    double                      m_T1;                   ///< Tissue specific longitudinal relaxation time
    ItkDoubleImgType::Pointer   m_VolumeFractionImage;  ///< Tissue specific volume fraction for each voxel (only relevant for non fiber compartments)
    ItkRandGenType::Pointer     m_RandGen;              ///< Random number generator
    double                      m_BValue;
};

}

#endif

