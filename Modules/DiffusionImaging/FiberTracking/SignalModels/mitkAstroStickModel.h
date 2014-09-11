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

#ifndef _MITK_AstroStickModel_H
#define _MITK_AstroStickModel_H

#include <mitkDiffusionSignalModel.h>
#include <itkPointShell.h>

namespace mitk {

/**
  * \brief Generates the diffusion signal using a collection of idealised cylinder with zero radius: e^(-bd(ng)²)
  *
  */

template< class ScalarType = double >
class AstroStickModel : public DiffusionSignalModel< ScalarType >
{
public:

    AstroStickModel();
    template< class OtherType >AstroStickModel(AstroStickModel<OtherType>* model)
    {
        this->m_CompartmentId = model->m_CompartmentId;
        this->m_T2 = model->GetT2();
        this->m_FiberDirection = model->GetFiberDirection();
        this->m_GradientList = model->GetGradientList();
        this->m_VolumeFractionImage = model->GetVolumeFractionImage();
        this->m_RandGen = model->GetRandomGenerator();

        this->m_BValue = model->GetBvalue();
        this->m_Diffusivity = model->GetDiffusivity();
        this->m_Sticks = model->GetSticks();
        this->m_NumSticks = model->GetNumSticks();
        this->m_RandomizeSticks = model->GetRandomizeSticks();
    }
    ~AstroStickModel();

    typedef typename DiffusionSignalModel< ScalarType >::PixelType          PixelType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientType       GradientType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientListType   GradientListType;

    /** Actual signal generation **/
    PixelType SimulateMeasurement();
    ScalarType SimulateMeasurement(unsigned int dir);


    void SetFiberDirection(GradientType fiberDirection){ this->m_FiberDirection = fiberDirection; }
    void SetGradientList(GradientListType gradientList) { this->m_GradientList = gradientList; }

    void SetRandomizeSticks(bool randomize=true){ m_RandomizeSticks=randomize; } ///< Random stick configuration in each voxel
    bool GetRandomizeSticks() { return m_RandomizeSticks; }

    void SetBvalue(double bValue) { m_BValue = bValue; }                     ///< b-value used to generate the artificial signal
    double GetBvalue() { return m_BValue; }

    void SetDiffusivity(double diffusivity) { m_Diffusivity = diffusivity; } ///< Scalar diffusion constant
    double GetDiffusivity() { return m_Diffusivity; }

    void SetNumSticks(unsigned int order)
    {
        vnl_matrix<double> sticks;
        switch (order)
        {
        case 1:
            m_NumSticks = 12;
            sticks = itk::PointShell<12, vnl_matrix_fixed<double, 3, 12> >::DistributePointShell()->as_matrix();
            break;
        case 2:
            m_NumSticks = 42;
            sticks = itk::PointShell<42, vnl_matrix_fixed<double, 3, 42> >::DistributePointShell()->as_matrix();
            break;
        case 3:
            m_NumSticks = 92;
            sticks = itk::PointShell<92, vnl_matrix_fixed<double, 3, 92> >::DistributePointShell()->as_matrix();
            break;
        case 4:
            m_NumSticks = 162;
            sticks = itk::PointShell<162, vnl_matrix_fixed<double, 3, 162> >::DistributePointShell()->as_matrix();
            break;
        case 5:
            m_NumSticks = 252;
            sticks = itk::PointShell<252, vnl_matrix_fixed<double, 3, 252> >::DistributePointShell()->as_matrix();
            break;
        default:
            m_NumSticks = 42;
            sticks = itk::PointShell<42, vnl_matrix_fixed<double, 3, 42> >::DistributePointShell()->as_matrix();
            break;
        }
        for (int i=0; i<m_NumSticks; i++)
        {
            GradientType stick;
            stick[0] = sticks.get(0,i); stick[1] = sticks.get(1,i); stick[2] = sticks.get(2,i);
            stick.Normalize();
            m_Sticks.push_back(stick);
        }
    }
    unsigned int GetNumSticks(){ return m_NumSticks; }
    GradientListType GetSticks(){ return m_Sticks; }

protected:

    GradientType GetRandomDirection();
    double   m_BValue;              ///< b-value used to generate the artificial signal
    double   m_Diffusivity;         ///< Scalar diffusion constant
    GradientListType m_Sticks;          ///< Stick container
    unsigned int m_NumSticks;           ///< Number of sticks
    bool m_RandomizeSticks;
};

}

#include "mitkAstroStickModel.cpp"

#endif

