/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryProduct.h"

#include "berryIBranding.h"

namespace berry {

Product::Product(IBranding* branding)
  : branding(branding)
{
}

QString Product::GetApplication() const
{
  return branding->GetApplication();
}

QSharedPointer<ctkPlugin> Product::GetDefiningPlugin() const
{
  return branding->GetDefiningPlugin();
}

QString Product::GetDescription() const
{
  return branding->GetDescription();
}

QString Product::GetId() const
{
  return branding->GetId();
}

QString Product::GetName() const
{
  return branding->GetName();
}

QString Product::GetProperty(const QString& key) const
{
  return branding->GetProperty(key);
}

}
