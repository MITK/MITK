/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIPropertyOwner.h>

mitk::IPropertyOwner::~IPropertyOwner()
{
}

bool mitk::IPropertyOwner::PropertyIsOwned(const std::string& propertyKey,
                                           const std::string& contextName /* = "" */,
                                           bool fallBackOnDefaultContext /* = true */) const
{
  auto keys = this->GetPropertyKeys(contextName, fallBackOnDefaultContext);

  if (std::find(keys.begin(), keys.end(), propertyKey) != keys.end())
    return true;
  else
    return false;
}
