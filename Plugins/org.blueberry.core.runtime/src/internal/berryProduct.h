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

#ifndef BERRYPRODUCT_H
#define BERRYPRODUCT_H

#include <berryIProduct.h>

namespace berry {

struct IBranding;

class Product : public IProduct
{

public:

  berryObjectMacro(Product, IProduct)

  Product(IBranding* branding);

  QString GetApplication() const;

  QSharedPointer<ctkPlugin> GetDefiningPlugin() const;

  QString GetDescription() const;

  QString GetId() const;

  QString GetName() const;

  QString GetProperty(const QString& key) const;

private:

  IBranding* branding;

};

}
#endif // BERRYPRODUCT_H
