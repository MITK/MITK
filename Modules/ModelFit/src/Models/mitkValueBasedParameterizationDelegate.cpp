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
