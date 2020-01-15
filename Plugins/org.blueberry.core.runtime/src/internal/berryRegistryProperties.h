/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYREGISTRYPROPERTIES_H
#define BERRYREGISTRYPROPERTIES_H

#include <QHash>

class ctkPluginContext;

namespace berry {

/**
 * Simple Property mechanism to chain property lookup from local registry properties,
 * to ctkPluginContext properties or System properties otherwise.
 */
class RegistryProperties
{

private:

  static QHash<QString,QString> registryProperties;
  static ctkPluginContext* context;

public:

  static void SetContext(ctkPluginContext* context);

  static QString GetProperty(const QString& propertyName);

  static QString GetProperty(const QString& property, const QString& defaultValue);

  static void SetProperty(const QString& propertyName, const QString& propertyValue);

private:

  static QString GetContextProperty(const QString& propertyName);
};

}

#endif // BERRYREGISTRYPROPERTIES_H
