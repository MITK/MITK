/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryApplicationDescriptor.h"

#include "berryApplicationHandle.h"
#include "berryApplicationContainer.h"

#include "berryIRuntimeConstants.h"

#include <service/application/ctkApplicationException.h>

namespace berry {

const QString ApplicationDescriptor::APP_TYPE = "blueberry.application.type";
const QString ApplicationDescriptor::APP_DEFAULT = "blueberry.application.default";
const QString ApplicationDescriptor::APP_TYPE_MAIN_THREAD = "main.thread";
const QString ApplicationDescriptor::APP_TYPE_ANY_THREAD = "any.thread";

ApplicationDescriptor::ApplicationDescriptor(const QSharedPointer<ctkPlugin>& contributor, const QString& pid, const QString& name, const QString& iconPath, const Flags& flags, int cardinality, ApplicationContainer* appContainer)
  : pid(pid)
  , name(name)
  , contributor(contributor)
  , appContainer(appContainer)
  , flags(flags)
  , cardinality(cardinality)
  , iconPath(iconPath)
  , instanceId(0)
  , locked(false)
{
  if (pid.isEmpty())
  {
    throw std::invalid_argument("Application ID must not be empty!");
  }
}

ApplicationDescriptor::~ApplicationDescriptor()
{
}

QString ApplicationDescriptor::getApplicationId() const
{
  return pid;
}

QHash<QString, QVariant> ApplicationDescriptor::getProperties(const QLocale& /*locale*/) const
{
  return this->getProperties();
}

QHash<QString, QVariant> berry::ApplicationDescriptor::getProperties() const
{
  // just use the service properties; for now we do not localize any properties
  return GetServiceProperties();
}

ctkApplicationHandle* ApplicationDescriptor::launch(const QHash<QString, QVariant>& arguments)
{
  CheckArgs(arguments);

  try
  {
    // if this application is locked throw an exception.
    /*
      if (GetLocked())
      {
        throw ctkIllegalStateException("Cannot launch a locked application.");
      }
      */
    // initialize the appHandle
    ApplicationHandle* appHandle = CreateAppHandle(arguments);
    try
    {
      // use the appContainer to launch the application on the main thread.
      appContainer->Launch(appHandle);
    }
    catch (const std::exception& t)
    {
      // be sure to destroy the appHandle if an error occurs
      try
      {
        appHandle->destroy();
      }
      catch (...)
      {
        // ignore and clean up
      }
      throw t;
    }
    return appHandle;
  }
  catch (const ctkIllegalStateException& ise)
  {
    throw ise;
  }
  catch (const ctkApplicationException& ae)
  {
    throw ae;
  }
  catch (const ctkException& e)
  {
    throw ctkApplicationException(ctkApplicationException::APPLICATION_INTERNAL_ERROR, e);
  }
  catch (const std::exception& e)
  {
    throw ctkApplicationException(ctkApplicationException::APPLICATION_INTERNAL_ERROR, QString(e.what()));
  }
  catch (...)
  {
    throw ctkApplicationException(ctkApplicationException::APPLICATION_INTERNAL_ERROR);
  }
}

void ApplicationDescriptor::CheckArgs(const QHash<QString, QVariant> arguments)
{
  foreach(QString key, arguments.keys())
  {
    if (key.isEmpty())
    {
      throw std::invalid_argument("Empty string is an invalid key");
    }
  }
}

ctkProperties ApplicationDescriptor::GetServiceProperties() const
{
  ctkProperties props;
  props[ctkApplicationDescriptor::APPLICATION_PID] = getApplicationId();
  if (!name.isEmpty())
  {
    props[ctkApplicationDescriptor::APPLICATION_NAME] = name;
  }
  props[ctkApplicationDescriptor::APPLICATION_CONTAINER] = IRuntimeConstants::PI_RUNTIME();
  props[ctkApplicationDescriptor::APPLICATION_LOCATION] = GetLocation();
  bool launchable = appContainer->IsLocked(this) == 0 ? true : false;
  props[ctkApplicationDescriptor::APPLICATION_LAUNCHABLE] = launchable;
  props[ctkApplicationDescriptor::APPLICATION_LOCKED] = GetLocked();
  bool visible = flags.testFlag(FLAG_VISIBLE);
  props[ctkApplicationDescriptor::APPLICATION_VISIBLE] = visible;
  props[APP_TYPE] = GetThreadTypeString();
  if (flags.testFlag(FLAG_DEFAULT_APP))
  {
    props[APP_DEFAULT] = true;
  }
  if (!iconPath.isEmpty())
  {
    props[ctkApplicationDescriptor::APPLICATION_ICON] = iconPath;
  }
  return props;
}

ApplicationHandle* ApplicationDescriptor::CreateAppHandle(const QHash<QString, QVariant>& arguments)
{
  ApplicationHandle* newAppHandle = new ApplicationHandle(GetInstanceID(), arguments, this);
  //appContainer.lock(newAppHandle);
  QStringList clazzes;
  clazzes.push_back(qobject_interface_iid<ctkApplicationHandle*>());
  clazzes.push_back(qobject_interface_iid<IApplicationContext*>());
  ctkServiceRegistration appHandleReg = appContainer->GetContext()->registerService(clazzes, newAppHandle,
                                                                                    newAppHandle->GetServiceProperties());
  newAppHandle->SetServiceRegistration(appHandleReg);
  return newAppHandle;
}

QString ApplicationDescriptor::GetInstanceID() const
{
  QMutexLocker l(&mutex);

  // make sure the instanceID has not reached the max
  if (instanceId == std::numeric_limits<long>::max())
  {
    instanceId = 0;
  }
  // create a unique instance id
  return getApplicationId() + "." + instanceId++;
}

QString ApplicationDescriptor::GetLocation() const
{
  if (!contributor)
  {
    return QString();
  }
  return contributor->getLocation();
}

bool ApplicationDescriptor::GetLocked() const
{
  QMutexLocker l(&mutex);
  return locked;
}

QString ApplicationDescriptor::GetThreadTypeString() const
{
  if (flags.testFlag(FLAG_TYPE_ANY_THREAD))
  {
    return APP_TYPE_ANY_THREAD;
  }
  return APP_TYPE_MAIN_THREAD;
}

ApplicationContainer* ApplicationDescriptor::GetContainerManager() const
{
  return appContainer;
}

void ApplicationDescriptor::RefreshProperties()
{
  ctkServiceRegistration reg = GetServiceRegistration();
  if (reg)
  {
    try
    {
      reg.setProperties(GetServiceProperties());
    }
    catch (const ctkIllegalStateException&)
    {
      // this must mean the service was unregistered
      // just ignore
    }
  }
}

void ApplicationDescriptor::SetServiceRegistration(const ctkServiceRegistration& sr)
{
  QMutexLocker l(&registrationMutex);
  this->sr = sr;
  registrationValidOrWaiting = sr;
  registrationMutexCondition.wakeAll();
}

ctkServiceRegistration ApplicationDescriptor::GetServiceRegistration() const
{
  QMutexLocker l(&registrationMutex);
  if (!sr && registrationValidOrWaiting)
  {
    registrationMutexCondition.wait(&registrationMutex, 1000); // timeout after 1 second
  }
  return sr;
}

void ApplicationDescriptor::Unregister()
{
  ctkServiceRegistration temp = GetServiceRegistration();
  if (temp)
  {
    SetServiceRegistration(ctkServiceRegistration());
    temp.unregister();
  }
}


int ApplicationDescriptor::GetThreadType() const
{
  return flags & (FLAG_TYPE_ANY_THREAD | FLAG_TYPE_MAIN_THREAD);
}

}
