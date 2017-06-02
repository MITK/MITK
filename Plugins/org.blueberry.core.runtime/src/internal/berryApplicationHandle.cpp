/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryApplicationHandle.h"

#include <berryIConfigurationElement.h>
#include <berryIExtension.h>
#include <berryIApplication.h>

#include "berryApplicationDescriptor.h"
#include "berryApplicationContainer.h"

#include "berryCTKPluginActivator.h"

#include <service/application/ctkApplicationException.h>
#include <ctkServiceRegistration.h>

#include <QTime>
#include <QMutexLocker>
#include <QCoreApplication>

namespace berry {

static const QString STARTING = "org.blueberry.app.starting";
static const QString STOPPED = "org.blueberry.app.stopped";


ApplicationHandle::ApplicationHandle(const QString& instanceId, const QHash<QString, QVariant>& arguments,
                                     ApplicationDescriptor* descriptor)
  : instanceId(instanceId)
  , descriptor(descriptor)
  , defaultAppInstance(false)
  , arguments(arguments)
  , status(FLAG_STARTING)
  , result(0)
  , setResult(false)
  , application(nullptr)
{
  if (instanceId.isEmpty() || descriptor == nullptr)
  {
    throw std::invalid_argument("Parameters must not be null!");
  }

  defaultAppInstance = arguments.isEmpty() ||
                       this->arguments.remove(ApplicationDescriptor::APP_DEFAULT);
}

ApplicationDescriptor* ApplicationHandle::getApplicationDescriptor() const
{
  return descriptor;
}

QString ApplicationHandle::getState_unlocked() const
{
  switch (status)
  {
  case FLAG_STARTING :
    return STARTING;
  case FLAG_ACTIVE :
    return ApplicationHandle::RUNNING;
  case FLAG_STOPPING :
    return ApplicationHandle::STOPPING;
  case FLAG_STOPPED :
  default :
    // must only check this if the status is STOPPED; otherwise we throw exceptions before we have set the registration.
    if (!GetServiceRegistration())
    {
      throw ctkIllegalStateException("This instance of the application has been stopped: " + getInstanceId());
    }
    return STOPPED;
  }
}

QString ApplicationHandle::getState() const
{
  QMutexLocker l(&mutex);
  return getState_unlocked();
}

QVariant ApplicationHandle::getExitValue(long timeout) const
{
  QMutexLocker l(&mutex);

  if (!handleRegistration && application == nullptr) return result;

  QTime stopTime = QTime::currentTime().addMSecs(timeout);
  while(!setResult && (QTime::currentTime() < stopTime || timeout == 0))
  {
    QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
  }

  if (!result.isValid())
  {
    throw ctkApplicationException(ctkApplicationException::APPLICATION_EXITVALUE_NOT_AVAILABLE);
  }
  return result;
}

QString ApplicationHandle::getInstanceId() const
{
  return instanceId;
}

void ApplicationHandle::destroy()
{
  if (STOPPING == getState()) return;

  // when this method is called we must force the application to exit.
  // first set the status to stopping
  SetAppStatus(FLAG_STOPPING);
  // now force the application to stop
  IApplication* app = GetApplication();
  if (app != nullptr)
  {
    app->Stop();
  }
  // make sure the app status is stopped
  SetAppStatus(FLAG_STOPPED);
}

QHash<QString, QVariant> ApplicationHandle::GetArguments() const
{
  return arguments;
}

void ApplicationHandle::ApplicationRunning()
{
  // first set the application handle status to running
  SetAppStatus(FLAG_ACTIVE);
  // now run the splash handler
  /*
  final ServiceReference[] monitors = getStartupMonitors();
  if (monitors == null)
    return;
      SafeRunner.run(new ISafeRunnable() {
        public void handleException(Throwable e) {
          // just continue ... the exception has already been logged by
          // handleException(ISafeRunnable)
        }

        public void run() throws Exception {
          for (int i = 0; i < monitors.length; i++) {
            StartupMonitor monitor = (StartupMonitor) Activator.getContext().getService(monitors[i]);
            if (monitor != null) {
              monitor.applicationRunning();
              Activator.getContext().ungetService(monitors[i]);
            }
          }
        }
      });
  */
}

QVariant ApplicationHandle::run(const QVariant& context_)
{
  QVariant context = context_;
  if (context.isValid())
  {
    // always force the use of the context if it is not null
    arguments[IApplicationContext::APPLICATION_ARGS] = context;
  }
  else
  {
    // get the context from the arguments
      context = arguments[IApplicationContext::APPLICATION_ARGS];
  }

  QVariant tempResult;
  try
  {
    {
      QMutexLocker l(&mutex);
      if (!(status.testFlag(FLAG_STARTING) || status.testFlag(FLAG_STOPPING)))
      {
        throw ctkApplicationException(ctkApplicationException::APPLICATION_INTERNAL_ERROR,
                                      QString("The application instance has been stopped before it could be started: ") + getInstanceId());
      }
      application = GetConfiguration()->CreateExecutableExtension<IApplication>("run");
      waitCondition.wakeAll();
    }
    tempResult = application->Start(this);
    if (!tempResult.isValid()) tempResult = QVariant(QVariant::Int);

    tempResult = SetInternalResult(tempResult, nullptr);
  }
  catch (...)
  {
    tempResult = SetInternalResult(tempResult, nullptr);
    throw;
  }

  if (org_blueberry_core_runtime_Activator::DEBUG)
  {
    qDebug() << QString("The application \"%1\" returned with code: %2.").arg(
                  getApplicationDescriptor()->getApplicationId()).arg(tempResult.toString());
  }
  return tempResult;
}

void ApplicationHandle::stop()
{
  try
  {
    destroy();
  }
  catch (const ctkIllegalStateException&)
  {
    // Do nothing; we don't care that the application was already stopped
    // return with no error
  }
}

void ApplicationHandle::SetAppStatus_unlocked(Status status)
{
  if (this->status == status)
  {
    return;
  }
  if (status & FLAG_STARTING)
  {
    throw std::invalid_argument("Cannot set app status to starting");
  }

  // if status is stopping and the context is already stopping then return
  if (status & FLAG_STOPPING)
  {
    if (this->status & (FLAG_STOPPING | FLAG_STOPPED))
    {
      return;
    }
  }
  // change the service properties to reflect the state change.
  this->status = status;

  ctkServiceRegistration handleReg = GetServiceRegistration();
  if (!handleReg) return;
  handleReg.setProperties(GetServiceProperties_unlocked());
  // if the status is stopped then unregister the service
  if (this->status & FLAG_STOPPED)
  {
    // GetApplicationDescriptor().getContainerManager().unlock(this);
    handleReg.unregister();
    SetServiceRegistration(ctkServiceRegistration());
  }
}

void ApplicationHandle::SetAppStatus(Status status)
{
  QMutexLocker l(&mutex);
  SetAppStatus_unlocked(status);
}

void ApplicationHandle::SetServiceRegistration(const ctkServiceRegistration& sr)
{
  this->handleRegistration = sr;
}

ctkServiceRegistration ApplicationHandle::GetServiceRegistration() const
{
  return handleRegistration;
}

ctkDictionary ApplicationHandle::GetServiceProperties_unlocked() const
{
  ctkDictionary props;
  props[APPLICATION_PID] = getInstanceId();
  props[APPLICATION_STATE] = getState_unlocked();
  props[APPLICATION_DESCRIPTOR] = getApplicationDescriptor()->getApplicationId();
  props[ApplicationDescriptor::APP_TYPE] = getApplicationDescriptor()->GetThreadTypeString();
  props[APPLICATION_SUPPORTS_EXITVALUE] = true;
  if (defaultAppInstance)
  {
    props[ApplicationDescriptor::APP_DEFAULT] = defaultAppInstance;
  }
  return props;
}

ctkDictionary ApplicationHandle::GetServiceProperties() const
{
  ctkDictionary props;
  props[APPLICATION_PID] = getInstanceId();
  props[APPLICATION_STATE] = getState();
  props[APPLICATION_DESCRIPTOR] = getApplicationDescriptor()->getApplicationId();
  props[ApplicationDescriptor::APP_TYPE] = getApplicationDescriptor()->GetThreadTypeString();
  props[APPLICATION_SUPPORTS_EXITVALUE] = true;
  if (defaultAppInstance)
  {
    props[ApplicationDescriptor::APP_DEFAULT] = defaultAppInstance;
  }
  return props;
}

QVariant ApplicationHandle::SetInternalResult(const QVariant& result, IApplication* /*tokenApp*/)
{
  QMutexLocker l(&mutex);

  if (setResult)
  {
    throw ctkIllegalStateException("The result of the application is already set.");
  }
  /*
  if (isAsync) {
    if (!setAsyncResult)
      throw new IllegalStateException("The application must return IApplicationContext.EXIT_ASYNC_RESULT to set asynchronous results."); //$NON-NLS-1$
    if (application != tokenApp)
      throw new IllegalArgumentException("The application is not the correct instance for this application context."); //$NON-NLS-1$
  } else {
    if (result == IApplicationContext.EXIT_ASYNC_RESULT) {
      setAsyncResult = true;
      return nullptr_RESULT; // the result well be set with setResult
    }
  }
  */
  this->result = result;
  setResult = true;
  application = nullptr;
  waitCondition.wakeAll();

  // The application exited itself; notify the app context
  SetAppStatus_unlocked(FLAG_STOPPING); // must ensure the STOPPING event is fired
  SetAppStatus_unlocked(FLAG_STOPPED);
  // only set the exit code property if this is the default application
  // (bug 321386) only set the exit code if the result != null; when result == null we assume an exception was thrown

  if (IsDefault() && !result.isNull())
  {
    //FrameworkProperties::SetProperty(FrameworkProperties::PROP_EXITCODE, result.toInt());
  }

  return result;
}

IApplication* ApplicationHandle::GetApplication() const
{
  QMutexLocker l(&mutex);

  if (handleRegistration && application == nullptr)
  {
    // the handle has been initialized by the container but the launcher has not
    // gotten around to creating the application object and starting it yet.
    waitCondition.wait(&mutex, 5000); // timeout after a while in case there was an internal error and there will be no application created
  }
  return application;
}

bool ApplicationHandle::IsDefault() const
{
  return defaultAppInstance;
}

IConfigurationElement::Pointer ApplicationHandle::GetConfiguration() const
{
  IExtension::Pointer applicationExtension = getApplicationDescriptor()->GetContainerManager()->
                                             GetAppExtension(getApplicationDescriptor()->getApplicationId());
  if (applicationExtension.IsNull())
  {
    throw ctkRuntimeException(QString("Application \"%1\" could not be found in the registry. The applications available are: %2.")
                              .arg(getApplicationDescriptor()->getApplicationId())
                              .arg(getApplicationDescriptor()->GetContainerManager()->GetAvailableAppsMsg()));
  }
  QList<IConfigurationElement::Pointer> configs = applicationExtension->GetConfigurationElements();
  if (configs.isEmpty())
  {
    throw ctkRuntimeException(QString("Invalid (empty) application extension \"%1\".")
                              .arg(getApplicationDescriptor()->getApplicationId()));
  }
  return configs.front();
}


}

