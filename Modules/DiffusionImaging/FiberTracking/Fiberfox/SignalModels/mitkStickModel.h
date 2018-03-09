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

#ifndef _MITK_StickModel_H
#define _MITK_StickModel_H

#include <mitkDiffusionSignalModel.h>

namespace mitk {

/**
  * \brief Generates the diffusion signal using an idealised cylinder with zero radius: e^(-bd(ng)²)
  *
  */

template< class ScalarType = double >
class StickModel : public DiffusionSignalModel< ScalarType >
{
public:

  StickModel();
  template< class OtherType >StickModel(StickModel<OtherType>* model)
  {
    this->m_CompartmentId = model->m_CompartmentId;
    this->m_T1 = model->GetT1();
    this->m_T2 = model->GetT2();
    this->m_GradientList = model->GetGradientList();
    this->m_VolumeFractionImage = model->GetVolumeFractionImage();
    this->m_RandGen = model->GetRandomGenerator();

    this->m_BValue = model->GetBvalue();
    this->m_Diffusivity = model->GetDiffusivity();
  }
  ~StickModel();

  typedef typename DiffusionSignalModel< ScalarType >::PixelType      PixelType;
  typedef typename DiffusionSignalModel< ScalarType >::GradientType   GradientType;
  typedef typename DiffusionSignalModel< ScalarType >::GradientListType   GradientListType;

  /** Actual signal generation **/
  PixelType SimulateMeasurement(GradientType& fiberDirection) override;
  ScalarType SimulateMeasurement(unsigned int dir, GradientType& fiberDirection) override;

  void SetDiffusivity(double diffusivity) { m_Diffusivity = diffusivity; } ///< Scalar diffusion constant
  double GetDiffusivity() { return m_Diffusivity; }

protected:

  double   m_Diffusivity;  ///< Scalar diffusion constant
};

}

#include "mitkStickModel.cpp"

#endif

