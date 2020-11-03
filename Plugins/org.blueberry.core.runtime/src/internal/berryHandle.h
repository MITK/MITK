/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYHANDLE_H
#define BERRYHANDLE_H

#include <berryObject.h>

namespace berry {

struct IObjectManager;

class RegistryObject;


/**
 * A handle is the super class to all registry objects that are now served to users.
 * The handles never hold on to any "real" content of the object being represented.
 * A handle can become stale if its referenced object has been removed from the registry.
 */
class Handle : public virtual Object
{
public:

  berryObjectMacro(berry::Handle);

  explicit Handle(const SmartPointer<const IObjectManager>& objectManager, int value);
  explicit Handle(const IObjectManager* objectManager, int value);

  ~Handle() override;

  /**
   * Return the actual object corresponding to this handle.
   * @throws InvalidRegistryObjectException when the handle is stale.
   */
  virtual SmartPointer<RegistryObject> GetObject() const = 0;

  int GetId() const;

  bool operator==(const Object* object) const override;

  uint HashCode() const override;

protected:

  const IObjectManager* const objectManager;

private:

  int objectId;
  bool isStrongRef;
};

}

#endif // BERRYHANDLE_H
