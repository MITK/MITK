/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPROVIDEREXTENSIONBRANDING_H
#define BERRYPROVIDEREXTENSIONBRANDING_H

#include "berryIBranding.h"

#include <berrySmartPointer.h>

namespace berry {

struct IProduct;

class ProviderExtensionBranding : public IBranding
{
public:

  ProviderExtensionBranding(const SmartPointer<IProduct>& product);

  QString GetApplication() const override;

  QString GetName() const override;

  QString GetDescription() const override;

  QString GetId() const override;

  QString GetProperty(const QString& key) const override;

  QSharedPointer<ctkPlugin> GetDefiningPlugin() const override;

  SmartPointer<IProduct> GetProduct() const override;

private:

  SmartPointer<IProduct> product;
};

}

#endif // BERRYPROVIDEREXTENSIONBRANDING_H
