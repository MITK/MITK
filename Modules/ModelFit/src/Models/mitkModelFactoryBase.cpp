/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkModelFactoryBase.h"

mitk::ModelParameterizerBase::Pointer mitk::ModelFactoryBase::CreateParameterizer(
  const mitk::modelFit::ModelFitInfo* fit) const
{
  if (!fit)
  {
    mitkThrow() << "Cannot generate parameterizer. Passed model fit is Null.";
  }

  return DoCreateParameterizer(fit);
};

mitk::ModelFactoryBase::ModelFactoryBase()
{
};

mitk::ModelFactoryBase::~ModelFactoryBase()
{
};
