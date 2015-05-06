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

#ifndef BERRYPRODUCTEXTENSIONBRANDING_H
#define BERRYPRODUCTEXTENSIONBRANDING_H

#include "berryIBranding.h"

#include <berrySmartPointer.h>

namespace berry {

struct IConfigurationElement;

class ProductExtensionBranding : public IBranding
{
private:

  static const QString ATTR_DESCRIPTION; // = "description";
  static const QString ATTR_NAME; // = "name";
  static const QString ATTR_APPLICATION; // = "application";
  static const QString ATTR_VALUE; // = "value";

  QString application;
  QString name;
  QString id;
  QString description;
  QHash<QString, QString> properties;
  QSharedPointer<ctkPlugin> definingPlugin;

public:

  ProductExtensionBranding(const QString& id, const SmartPointer<IConfigurationElement>& element);

  QSharedPointer<ctkPlugin> GetDefiningPlugin() const override;

  QString GetApplication() const override;

  QString GetName() const override;

  QString GetDescription() const override;

  QString GetId() const override;

  QString GetProperty(const QString& key) const override;

  SmartPointer<IProduct> GetProduct() const override;

private:

  void LoadProperties(const SmartPointer<IConfigurationElement>& element);

};

}

#endif // BERRYPRODUCTEXTENSIONBRANDING_H
