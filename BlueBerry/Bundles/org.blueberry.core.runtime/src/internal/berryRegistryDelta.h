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

#ifndef BERRYREGISTRYDELTA_H
#define BERRYREGISTRYDELTA_H

#include <QList>
#include <QExplicitlySharedDataPointer>

namespace berry {

struct IObjectManager;

class ExtensionDelta;
class RegistryDeltaData;

/**
 * The extension deltas are grouped by namespace. There is one registry delta by namespace.
 */
class RegistryDelta
{

public:

  RegistryDelta();
  ~RegistryDelta();

  int GetExtensionDeltasCount() const;

  QList<ExtensionDelta> GetExtensionDeltas() const;

  QList<ExtensionDelta> GetExtensionDeltas(const QString& extensionPoint) const;

  /**
   * @param extensionPointId
   * @param extensionId must not be null
   */
  ExtensionDelta GetExtensionDelta(const QString& extensionPointId, const QString& extensionId) const;

  void AddExtensionDelta(ExtensionDelta &extensionDelta);

  QString ToString() const;

  void SetObjectManager(const SmartPointer<IObjectManager>& objectManager);

  SmartPointer<IObjectManager> GetObjectManager() const;

private:

  QExplicitlySharedDataPointer<RegistryDeltaData> d;
};

}

#endif // BERRYREGISTRYDELTA_H
