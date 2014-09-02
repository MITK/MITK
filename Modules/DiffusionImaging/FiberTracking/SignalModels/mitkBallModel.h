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

#ifndef _MITK_BallModel_H
#define _MITK_BallModel_H

#include <mitkDiffusionSignalModel.h>

namespace mitk {

/**
  * \brief Generates direction independent diffusion measurement employing a scalar diffusion constant d: e^(-bd)
  *
  */

template< class ScalarType = double >
class BallModel : public DiffusionSignalModel< ScalarType >
{
public:

    BallModel();
    template< class OtherType >BallModel(BallModel<OtherType>* model)
    {
        this->m_CompartmentId = model->m_CompartmentId;
        this->m_T2 = model->GetT2();
        this->m_FiberDirection = model->GetFiberDirection();
        this->m_GradientList = model->GetGradientList();
        this->m_VolumeFractionImage = model->GetVolumeFractionImage();
        this->m_RandGen = model->GetRandomGenerator();

        this->m_BValue = model->GetBvalue();
        this->m_Diffusivity = model->GetDiffusivity();
    }
    ~BallModel();

    typedef typename DiffusionSignalModel< ScalarType >::PixelType      PixelType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientType   GradientType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientListType   GradientListType;


    /** Actual signal generation **/
    PixelType SimulateMeasurement();
    ScalarType SimulateMeasurement(unsigned int dir);

    void SetDiffusivity(double D) { m_Diffusivity = D; }
    double GetDiffusivity() { return m_Diffusivity; }
    void SetBvalue(double bValue) { m_BValue = bValue; }                     ///< b-value used to generate the artificial signal
    double GetBvalue() { return m_BValue; }

    void SetFiberDirection(GradientType fiberDirection){ this->m_FiberDirection = fiberDirection; }
    void SetGradientList(GradientListType gradientList) { this->m_GradientList = gradientList; }

protected:

    double  m_Diffusivity;  ///< Scalar diffusion constant
    double  m_BValue;       ///< b-value used to generate the artificial signal
};

}

#include "mitkBallModel.cpp"

#endif

