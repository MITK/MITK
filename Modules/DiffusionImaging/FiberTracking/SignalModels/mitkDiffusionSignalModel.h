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

#include <FiberTrackingExports.h>
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

    DiffusionSignalModel()
        : m_T2(100)
        , m_Weight(1)
    {}
    ~DiffusionSignalModel(){}

    typedef itk::VariableLengthVector< ScalarType > PixelType;
    typedef itk::Vector<double,3>                   GradientType;
    typedef std::vector<GradientType>               GradientListType;

    /** Realizes actual signal generation. Has to be implemented in subclass. **/
    virtual PixelType SimulateMeasurement() = 0;
    virtual ScalarType SimulateMeasurement(unsigned int dir) = 0;

    GradientType GetGradientDirection(int i) { return m_GradientList.at(i); }
    void SetFiberDirection(GradientType fiberDirection){ m_FiberDirection = fiberDirection; }
    void SetGradientList(GradientListType gradientList) { m_GradientList = gradientList; }
    void SetT2(double T2) { m_T2 = T2; }
    void SetWeight(double Weight) { m_Weight = Weight; }

    double GetWeight() { return m_Weight; }
    double GetT2() { return m_T2; }
    int GetNumGradients(){ return m_GradientList.size(); }
    std::vector< int > GetBaselineIndices()
    {
        std::vector< int > result;
        for( unsigned int i=0; i<this->m_GradientList.size(); i++)
            if (m_GradientList.at(i).GetNorm()<0.0001)
                result.push_back(i);
        return result;
    }
    int GetFirstBaselineIndex()
    {
        for( unsigned int i=0; i<this->m_GradientList.size(); i++)
            if (m_GradientList.at(i).GetNorm()<0.0001)
                return i;
        return -1;
    }
    bool IsBaselineIndex(unsigned int idx)
    {
        if (m_GradientList.size()>idx && m_GradientList.at(idx).GetNorm()<0.0001)
            return true;
        return false;
    }

protected:

    GradientType        m_FiberDirection;   ///< Needed to generate anisotropc signal to determin direction of anisotropy
    GradientListType    m_GradientList;     ///< Diffusion gradient direction container
    double              m_T2;               ///< Tissue specific relaxation time
    double              m_Weight;
};

}

#endif

