/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryProviderExtensionBranding.h"

#include <berryIProduct.h>

namespace berry {

ProviderExtensionBranding::ProviderExtensionBranding(const SmartPointer<IProduct>& product)
  : product(product)
{}

QString ProviderExtensionBranding::GetApplication() const
{
  return product->GetApplication();
}

QString ProviderExtensionBranding::GetName() const
{
  return product->GetName();
}

QString ProviderExtensionBranding::GetDescription() const
{
  return product->GetDescription();
}

QString ProviderExtensionBranding::GetId() const
{
  return product->GetId();
}

QString ProviderExtensionBranding::GetProperty(const QString& key) const
{
  return product->GetProperty(key);
}

QSharedPointer<ctkPlugin> ProviderExtensionBranding::GetDefiningPlugin() const
{
  return product->GetDefiningPlugin();
}

IProduct::Pointer ProviderExtensionBranding::GetProduct() const
{
  return product;
}

}
