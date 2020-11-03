/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkValueBasedParameterizationDelegate.h"


mitk::ValueBasedParameterizationDelegate::ValueBasedParameterizationDelegate()
{
};

mitk::ValueBasedParameterizationDelegate::~ValueBasedParameterizationDelegate()
{
};

void
mitk::ValueBasedParameterizationDelegate::SetInitialParameterization(ParametersType params)
{
  this->m_Parameterization = params;
};

mitk::ValueBasedParameterizationDelegate::ParametersType
mitk::ValueBasedParameterizationDelegate::GetInitialParameterization() const
{
  return this->m_Parameterization;
};

mitk::ValueBasedParameterizationDelegate::ParametersType
mitk::ValueBasedParameterizationDelegate::GetInitialParameterization(const IndexType&
    /*currentPosition*/) const
{
  return this->m_Parameterization;
};
