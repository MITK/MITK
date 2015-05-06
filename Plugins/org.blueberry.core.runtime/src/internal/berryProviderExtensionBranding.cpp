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
