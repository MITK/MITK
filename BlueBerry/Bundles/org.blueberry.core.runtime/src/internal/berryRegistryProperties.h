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
