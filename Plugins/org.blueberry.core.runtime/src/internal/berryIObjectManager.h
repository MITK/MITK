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

#ifndef BERRYIOBJECTMANAGER_H
#define BERRYIOBJECTMANAGER_H

#include <berryObject.h>

namespace berry {

class Handle;
class RegistryObject;

struct IObjectManager : public Object
{
  berryObjectMacro(berry::IObjectManager)

  ~IObjectManager() override;

  virtual SmartPointer<Handle> GetHandle(int id, short type) const = 0;

  virtual QList<SmartPointer<Handle> > GetHandles(const QList<int>& ids, short type) const = 0;

  virtual SmartPointer<RegistryObject> GetObject(int id, short type) const = 0;

  virtual QList<SmartPointer<RegistryObject> > GetObjects(const QList<int>& values, short type) const = 0;

  virtual void Close() = 0;
};

}
#endif // BERRYIOBJECTMANAGER_H
