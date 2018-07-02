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