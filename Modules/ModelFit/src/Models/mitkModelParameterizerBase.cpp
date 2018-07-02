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

#include "mitkModelParameterizerBase.h"


mitk::ModelParameterizerBase::ModelParameterizerBase()
{
  m_DefaultTimeGrid.SetSize(1);
  m_DefaultTimeGrid[0] = 0.0;
};

mitk::ModelParameterizerBase::~ModelParameterizerBase()
{
};

mitk::ModelParameterizerBase::ParametersType
mitk::ModelParameterizerBase::GetInitialParameterization() const
{
  if (m_InitialDelegate.IsNotNull())
  {
    return m_InitialDelegate->GetInitialParameterization();
  }

  return this->GetDefaultInitialParameterization();
}

mitk::ModelParameterizerBase::ParametersType
mitk::ModelParameterizerBase::GetInitialParameterization(const IndexType& currentPosition) const
{
  if (m_InitialDelegate.IsNotNull())
  {
    return m_InitialDelegate->GetInitialParameterization(currentPosition);
  }

  return this->GetDefaultInitialParameterization();
}

void
mitk::ModelParameterizerBase::
SetInitialParameterizationDelegate(const InitialParameterizationDelegateBase* delegate)
{
  this->m_InitialDelegate = delegate;
};