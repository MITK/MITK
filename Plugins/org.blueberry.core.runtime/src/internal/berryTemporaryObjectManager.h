/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYTEMPORARYOBJECTMANAGER_H
#define BERRYTEMPORARYOBJECTMANAGER_H

#include "berryIObjectManager.h"

#include <QMutex>

namespace berry {

class RegistryObjectManager;

class TemporaryObjectManager : public IObjectManager
{
private:

  mutable QMutex mutex;
  QHash<int, SmartPointer<RegistryObject> > actualObjects; //id --> registry objects
  RegistryObjectManager* const parent; //the main object manager (should be equals to extensionRegistry.getObjectManager)

public:

  TemporaryObjectManager(const QHash<int, SmartPointer<RegistryObject> >& actualObjects,
                         RegistryObjectManager* parent);

  SmartPointer<Handle> GetHandle(int id, short type) const override;

  QList<SmartPointer<Handle> > GetHandles(const QList<int>& ids, short type) const override;

  SmartPointer<RegistryObject> GetObject(int id, short type) const override;

  QList<SmartPointer<RegistryObject> > GetObjects(const QList<int>& values, short type) const override;

  void Close() override;
};

}

#endif // BERRYTEMPORARYOBJECTMANAGER_H
