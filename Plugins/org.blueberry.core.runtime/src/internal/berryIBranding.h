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
