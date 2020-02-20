/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryTemporaryObjectManager.h"

#include "berryRegistryObjectManager.h"
#include "berryExtensionHandle.h"
#include "berryExtensionPointHandle.h"
#include "berryInvalidRegistryObjectException.h"
#include "berryRegistryObject.h"
#include "berryThirdLevelConfigurationElementHandle.h"

namespace berry {

TemporaryObjectManager::TemporaryObjectManager(const QHash<int, SmartPointer<RegistryObject> >& actualObjects,
                       RegistryObjectManager* parent)
  : actualObjects(actualObjects), parent(parent)
{
}

SmartPointer<Handle> TemporaryObjectManager::GetHandle(int id, short type) const
{
  Handle::Pointer handle;
  switch (type)
  {
  case RegistryObjectManager::EXTENSION_POINT :
    handle = new ExtensionPointHandle(IObjectManager::ConstPointer(this), id);
    return handle;

  case RegistryObjectManager::EXTENSION :
    handle = new ExtensionHandle(IObjectManager::ConstPointer(this), id);
    return handle;

  case RegistryObjectManager::CONFIGURATION_ELEMENT :
    handle = new ConfigurationElementHandle(IObjectManager::ConstPointer(this), id);
    return handle;

    case RegistryObjectManager::THIRDLEVEL_CONFIGURATION_ELEMENT :
    default : //avoid compiler error, type should always be known
      handle = new ThirdLevelConfigurationElementHandle(IObjectManager::ConstPointer(this), id);
      return handle;
  }
}

QList<SmartPointer<Handle> > TemporaryObjectManager::GetHandles(const QList<int>& ids, short type) const
{
  QList<Handle::Pointer> results;
  switch (type)
  {
    case RegistryObjectManager::EXTENSION_POINT :
      for (int i = 0; i < ids.size(); i++)
      {
        Handle::Pointer handle(new ExtensionPointHandle(IObjectManager::ConstPointer(this), ids[i]));
        results.push_back(handle);
      }
      break;

    case RegistryObjectManager::EXTENSION :
      for (int i = 0; i < ids.size(); i++)
      {
        Handle::Pointer handle(new ExtensionHandle(IObjectManager::ConstPointer(this), ids[i]));
        results.push_back(handle);
      }
      break;

    case RegistryObjectManager::CONFIGURATION_ELEMENT :
      for (int i = 0; i < ids.size(); i++)
      {
        Handle::Pointer handle(new ConfigurationElementHandle(IObjectManager::ConstPointer(this), ids[i]));
        results.push_back(handle);
      }
      break;

    case RegistryObjectManager::THIRDLEVEL_CONFIGURATION_ELEMENT :
      for (int i = 0; i < ids.size(); i++)
      {
        Handle::Pointer handle(new ThirdLevelConfigurationElementHandle(IObjectManager::ConstPointer(this), ids[i]));
        results.push_back(handle);
      }
      break;
  }
  return results;
}

SmartPointer<RegistryObject> TemporaryObjectManager::GetObject(int id, short type) const
{
  QMutexLocker l(&mutex);
  RegistryObject::Pointer result;
  try
  {
    result = parent->GetObject(id, type);
  }
  catch (const InvalidRegistryObjectException& /*e*/)
  {
    result = actualObjects.value(id);
  }
  if (result.IsNull())
    throw InvalidRegistryObjectException();
  return result;
}

QList<SmartPointer<RegistryObject> > TemporaryObjectManager::GetObjects(const QList<int>& values, short type) const
{
  QMutexLocker l(&mutex);

  QList<RegistryObject::Pointer> results;
  for (int i = 0; i < values.size(); i++)
  {
    results.push_back(GetObject(values[i], type));
  }
  return results;
}

void TemporaryObjectManager::Close()
{
  QMutexLocker l(&mutex);
  actualObjects.clear();
}

}
