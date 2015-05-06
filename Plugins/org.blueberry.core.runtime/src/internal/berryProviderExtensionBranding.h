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
