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

  SmartPointer<Handle> GetHandle(int id, short type) const;

  QList<SmartPointer<Handle> > GetHandles(const QList<int>& ids, short type) const;

  SmartPointer<RegistryObject> GetObject(int id, short type) const;

  QList<SmartPointer<RegistryObject> > GetObjects(const QList<int>& values, short type) const;

  void Close();
};

}

#endif // BERRYTEMPORARYOBJECTMANAGER_H
