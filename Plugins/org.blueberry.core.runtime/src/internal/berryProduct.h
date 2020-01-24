/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPRODUCT_H
#define BERRYPRODUCT_H

#include <berryIProduct.h>

namespace berry {

struct IBranding;

class Product : public IProduct
{

public:

  berryObjectMacro(Product, IProduct);

  Product(IBranding* branding);

  QString GetApplication() const override;

  QSharedPointer<ctkPlugin> GetDefiningPlugin() const override;

  QString GetDescription() const override;

  QString GetId() const override;

  QString GetName() const override;

  QString GetProperty(const QString& key) const override;

private:

  IBranding* branding;

};

}
#endif // BERRYPRODUCT_H
