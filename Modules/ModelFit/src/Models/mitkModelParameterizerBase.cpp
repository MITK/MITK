/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
