/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIBRANDING_H
#define BERRYIBRANDING_H

#include <berrySmartPointer.h>

#include <QSharedPointer>

class ctkPlugin;

namespace berry {

struct IProduct;

struct IBranding
{
  virtual ~IBranding();

  virtual QString GetApplication() const = 0;

  virtual QString GetName() const = 0;

  virtual QString GetDescription() const = 0;

  virtual QString GetId() const = 0;

  virtual QString GetProperty(const QString& key) const = 0;

  virtual QSharedPointer<ctkPlugin> GetDefiningPlugin() const = 0;

  virtual SmartPointer<IProduct> GetProduct() const = 0;
};

}

#endif // BERRYIBRANDING_H
