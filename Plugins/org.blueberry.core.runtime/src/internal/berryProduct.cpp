/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
