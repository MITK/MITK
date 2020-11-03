/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIOBJECTMANAGER_H
#define BERRYIOBJECTMANAGER_H

#include <berryObject.h>

namespace berry {

class Handle;
class RegistryObject;

struct IObjectManager : public Object
{
  berryObjectMacro(berry::IObjectManager);

  ~IObjectManager() override;

  virtual SmartPointer<Handle> GetHandle(int id, short type) const = 0;

  virtual QList<SmartPointer<Handle> > GetHandles(const QList<int>& ids, short type) const = 0;

  virtual SmartPointer<RegistryObject> GetObject(int id, short type) const = 0;

  virtual QList<SmartPointer<RegistryObject> > GetObjects(const QList<int>& values, short type) const = 0;

  virtual void Close() = 0;
};

}
#endif // BERRYIOBJECTMANAGER_H
