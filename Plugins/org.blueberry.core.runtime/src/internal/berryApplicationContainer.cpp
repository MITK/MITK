/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryApplicationContainer.h"

#include <berryIConfigurationElement.h>
#include <berryIContributor.h>
#include <berryIExtensionRegistry.h>
#include <berryIExtension.h>
#include <berryIExtensionPoint.h>
#include <berryIProductProvider.h>
#include <berryIProduct.h>

#include "berryApplicationHandle.h"
#include "berryApplicationDescriptor.h"
#include "berryCoreException.h"
#include "berryErrorApplication.h"
#include "berryMainApplicationLauncher.h"
#include "berryProductExtensionBranding.h"
#include "berryProviderExtensionBranding.h"

#include <berryLog.h>

#include "berryCTKPluginActivator.h"

#include <service/application/ctkApplicationException.h>
#include <service/application/ctkApplicationLauncher.h>

namespace berry {

static const QString PI_OSGI = "org.blueberry.osgi";

const QString ApplicationContainer::PI_RUNTIME = "org.blueberry.core.runtime";
const QString ApplicationContainer::PT_APPLICATIONS = "applications";
const QString ApplicationContainer::PT_APP_VISIBLE = "visible";
const QString ApplicationContainer::PT_APP_THREAD = "thread";
const QString ApplicationContainer::PT_APP_THREAD_ANY = "any";
const QString ApplicationContainer::PT_APP_CARDINALITY = "cardinality";
const QString ApplicationContainer::PT_APP_CARDINALITY_SINGLETON_GLOBAL = "singleton-global";
const QString ApplicationContainer::PT_APP_CARDINALITY_SINGLETON_SCOPED = "singleton-scoped";
const QString ApplicationContainer::PT_APP_CARDINALITY_UNLIMITED = "*";
const QString ApplicationContainer::PT_APP_ICON = "icon";
const QString ApplicationContainer::PT_PRODUCTS = "products";
const QString ApplicationContainer::EXT_ERROR_APP = "org.blueberry.core.runtime.app.error";

const QString ApplicationContainer::PROP_PRODUCT = "blueberry.product";
const QString ApplicationContainer::PROP_BLUEBERRY_APPLICATION = "blueberry.application";
const QString ApplicationContainer::PROP_BLUEBERRY_APPLICATION_LAUNCH_DEFAULT = "blueberry.application.launchDefault";

const int ApplicationContainer::NOT_LOCKED = 0;
const int ApplicationContainer::LOCKED_SINGLETON_GLOBAL_RUNNING = 1;
const int ApplicationContainer::LOCKED_SINGLETON_GLOBAL_APPS_RUNNING = 2;
const int ApplicationContainer::LOCKED_SINGLETON_SCOPED_RUNNING = 3;
const int ApplicationContainer::LOCKED_SINGLETON_LIMITED_RUNNING = 4;
const int ApplicationContainer::LOCKED_MAIN_THREAD_RUNNING = 5;


ApplicationContainer::ApplicationContainer(ctkPluginContext* context, IExtensionRegistry* extensionRegistry)
  : context(context)
  , extensionRegistry(extensionRegistry)
  , launcherTracker(new ctkServiceTracker<ctkApplicationLauncher*>(context, this))
  , branding(nullptr)
  , missingProductReported(false)
  , defaultMainThreadAppHandle(nullptr)
  , missingApp(false)
{
}

ApplicationContainer::~ApplicationContainer()
{
}

void ApplicationContainer::Start()
{
  launcherTracker->open();
  extensionRegistry->AddListener(this, PI_OSGI + '.' + PT_APPLICATIONS);
  // need to listen for system bundle stopping
  context->connectPluginListener(this, SLOT(PluginChanged(ctkPluginEvent)), Qt::DirectConnection);
  // register all the descriptors
  RegisterAppDescriptors();
  QVariant startDefaultProp = context->getProperty(ApplicationContainer::PROP_BLUEBERRY_APPLICATION_LAUNCH_DEFAULT);
  if (startDefaultProp.isNull() || startDefaultProp.toBool())
  {
    // Start the default application
    try
    {
      StartDefaultApp(true);
    }
    catch (const ctkApplicationException& e)
    {
      BERRY_ERROR << "An error occurred while starting the application:" << e.message();
    }
  }
}

void ApplicationContainer::Stop()
{
  // stop all applications
  StopAllApps();
  context->disconnectPluginListener(this);
  extensionRegistry->RemoveListener(this);
  // flush the apps
  apps.clear();
  branding.reset();
  missingProductReported = false;
  launcherTracker->close();
}

IBranding* ApplicationContainer::GetBranding() const
{
  if (branding)
  {
    return branding.data();
  }

  // try pluginContext properties
  if (context == nullptr) return nullptr;
  QString productId = context->getProperty(PROP_PRODUCT).toString();
  if (productId.isEmpty()) return nullptr;

  QList<IConfigurationElement::Pointer> entries = extensionRegistry->GetConfigurationElementsFor(PI_RUNTIME, PT_PRODUCTS, productId);
  if (!entries.isEmpty())
  {
    // There should only be one product with the given id so just take the first element
    branding.reset(new ProductExtensionBranding(productId, entries[0]));
    return branding.data();
  }
  QList<IConfigurationElement::Pointer> elements = extensionRegistry->GetConfigurationElementsFor(PI_RUNTIME, PT_PRODUCTS);
  for (auto element : elements)
  {
    if (element->GetName().compare("provider", Qt::CaseInsensitive) == 0)
    {
      try
      {
        IProductProvider* provider = element->CreateExecutableExtension<IProductProvider>("run");
        QList<IProduct::Pointer> products = provider->GetProducts();
        for (auto product : products)
        {
          if (productId.compare(product->GetId(), Qt::CaseInsensitive) == 0)
          {
            branding.reset(new ProviderExtensionBranding(product));
            return branding.data();
          }
        }
      }
      catch (const CoreException&)
      {
        BERRY_WARN << "Problem creating the provider registered by " << element->GetParent()->ToString() << ".";
      }
    }
  }

  if (!missingProductReported)
  {
    BERRY_WARN << "Product " << productId << " could not be found.";
    missingProductReported = true;
  }
  return nullptr;
}

ctkPluginContext* ApplicationContainer::GetContext() const
{
  return context;
}

ApplicationDescriptor* ApplicationContainer::GetAppDescriptor(const QString& applicationId)
{
  ApplicationDescriptor* result = nullptr;
  {
    QMutexLocker l(&lock);
    auto iter = apps.find(applicationId);
    result = iter == apps.end() ? nullptr : iter.value();
  }

  if (result == nullptr)
  {
    RegisterAppDescriptor(applicationId); // try again just in case we are waiting for an event
    {
      QMutexLocker l(&lock);
      auto iter = apps.find(applicationId);
      result = iter == apps.end() ? nullptr : iter.value();
    }
  }
  return result;
}

ApplicationDescriptor* ApplicationContainer::CreateAppDescriptor(const SmartPointer<IExtension>& appExtension)
{
  if (org_blueberry_core_runtime_Activator::DEBUG)
  {
    BERRY_INFO << "Creating application descriptor: " << appExtension->GetUniqueIdentifier();
  }
  QString iconPath;
  {
    QMutexLocker l(&lock);
    auto iter = apps.find(appExtension->GetUniqueIdentifier());
    ApplicationDescriptor* appDescriptor = iter == apps.end() ? nullptr : iter.value();
    if (appDescriptor != nullptr)
    {
      return appDescriptor;
    }

    // the appDescriptor does not exist for the app ID; create it
    QList<IConfigurationElement::Pointer> configs = appExtension->GetConfigurationElements();
    ApplicationDescriptor::Flags flags = ApplicationDescriptor::FLAG_CARD_SINGLETON_GLOGAL | ApplicationDescriptor::FLAG_VISIBLE |
                                         ApplicationDescriptor::FLAG_TYPE_MAIN_THREAD;
    int cardinality = 0;
    if (!configs.isEmpty())
    {
      QString sVisible = configs[0]->GetAttribute(PT_APP_VISIBLE);
      if (!sVisible.isEmpty() && sVisible.compare("true", Qt::CaseInsensitive) != 0)
      {
        flags &= ~(ApplicationDescriptor::FLAG_VISIBLE);
      }
      QString sThread = configs[0]->GetAttribute(PT_APP_THREAD);
      if (PT_APP_THREAD_ANY == sThread)
      {
        flags |= ApplicationDescriptor::FLAG_TYPE_ANY_THREAD;
        flags &= ~(ApplicationDescriptor::FLAG_TYPE_MAIN_THREAD);
      }
      QString sCardinality = configs[0]->GetAttribute(PT_APP_CARDINALITY);
      if (!sCardinality.isEmpty())
      {
        flags &= ~(ApplicationDescriptor::FLAG_CARD_SINGLETON_GLOGAL); // clear the global bit
        if (PT_APP_CARDINALITY_SINGLETON_SCOPED == sCardinality)
        {
          flags |= ApplicationDescriptor::FLAG_CARD_SINGLETON_SCOPED;
        }
        else if (PT_APP_CARDINALITY_UNLIMITED == sCardinality)
        {
          flags |= ApplicationDescriptor::FLAG_CARD_UNLIMITED;
        }
        else if (PT_APP_CARDINALITY_SINGLETON_GLOBAL == sCardinality)
        {
          flags |= ApplicationDescriptor::FLAG_CARD_SINGLETON_GLOGAL;
        }
        else
        {
          bool okay = false;
          cardinality = sCardinality.toInt(&okay);
          if (okay)
          {
            flags |= ApplicationDescriptor::FLAG_CARD_LIMITED;
          }
          else
          {
            // TODO should we log this?
            // just fall back to the default
            flags |= ApplicationDescriptor::FLAG_CARD_SINGLETON_GLOGAL;
          }
        }
      }
      QString defaultApp = GetDefaultAppId();
      QString appId = appExtension->GetUniqueIdentifier();
      if (defaultApp == appId)
      {
        flags |= ApplicationDescriptor::FLAG_DEFAULT_APP;
      }
      iconPath = configs[0]->GetAttribute(PT_APP_ICON);
    }
    appDescriptor = new ApplicationDescriptor(org_blueberry_core_runtime_Activator::GetPlugin(appExtension->GetContributor()),
                                              appExtension->GetUniqueIdentifier(), appExtension->GetLabel(), iconPath, flags, cardinality, this);
    // register the appDescriptor as a service
    ctkServiceRegistration sr = context->registerService<ctkApplicationDescriptor>(appDescriptor, appDescriptor->GetServiceProperties());
    appDescriptor->SetServiceRegistration(sr);
    // save the app descriptor in the cache
    apps.insert(appExtension->GetUniqueIdentifier(), appDescriptor);
    return appDescriptor;
  }
}

ApplicationDescriptor*ApplicationContainer::RemoveAppDescriptor(const QString& applicationId)
{
  if (org_blueberry_core_runtime_Activator::DEBUG)
  {
    BERRY_INFO << "Removing application descriptor: " << applicationId;
  }

  {
    QMutexLocker l(&lock);
    ApplicationDescriptor* appDescriptor = apps.take(applicationId);
    if (appDescriptor == nullptr)
    {
      return nullptr;
    }
    appDescriptor->Unregister();
    return appDescriptor;
  }
}

void ApplicationContainer::StartDefaultApp(bool delayError)
{
  // find the default application
  QString applicationId = GetDefaultAppId();
  ApplicationDescriptor* defaultDesc = nullptr;
  QHash<QString, QVariant> args;
  args.insert(ApplicationDescriptor::APP_DEFAULT, true);
  if (applicationId.isEmpty() && !delayError)
  {
    // the application id is not set; use a descriptor that will throw an exception
    args.insert(ErrorApplication::ERROR_EXCEPTION, QString("No application id has been found."));
    defaultDesc = GetAppDescriptor(EXT_ERROR_APP);
  }
  else
  {
    defaultDesc = GetAppDescriptor(applicationId);
    if (defaultDesc == nullptr && !delayError)
    {
      // the application id is not available in the registry; use a descriptor that will throw an exception
      args.insert(ErrorApplication::ERROR_EXCEPTION, QString("Application \"%1\" could not be found in the registry. The applications available are: %2.")
                  .arg(applicationId)
                  .arg(GetAvailableAppsMsg()));
      defaultDesc = GetAppDescriptor(EXT_ERROR_APP);
    }
  }
  if (delayError && defaultDesc == nullptr)
  {
    // could not find the application; but we want to delay the error.
    // another bundle may get installed that provides the application
    // before we actually try to launch it.
    missingApp = true;
    return;
  }
  if (defaultDesc != nullptr)
  {
    defaultDesc->launch(args);
  }
  else
  {
    throw ctkApplicationException(ctkApplicationException::APPLICATION_INTERNAL_ERROR, "No application id has been found.");
  }
}

void ApplicationContainer::RegisterAppDescriptors()
{
  QList<IExtension::Pointer> availableApps = GetAvailableAppExtensions();
  for (int i = 0; i < availableApps.size(); i++)
  {
    CreateAppDescriptor(availableApps[i]);
  }
}

void ApplicationContainer::RegisterAppDescriptor(const QString& applicationId)
{
  IExtension::Pointer appExtension = GetAppExtension(applicationId);
  if (appExtension.IsNotNull())
  {
    CreateAppDescriptor(appExtension);
  }
}

QList<SmartPointer<IExtension> > ApplicationContainer::GetAvailableAppExtensions() const
{
  IExtensionPoint::Pointer point = extensionRegistry->GetExtensionPoint(PI_OSGI + '.' + PT_APPLICATIONS);
  if (point.IsNull())
  {
    return QList<IExtension::Pointer>();
  }
  return point->GetExtensions();
}

QString ApplicationContainer::GetAvailableAppsMsg() const
{
  QList<IExtension::Pointer> availableApps = GetAvailableAppExtensions();
  QString availableAppsMsg = "<NONE>";
  if (!availableApps.isEmpty())
  {
    availableAppsMsg = availableApps.front()->GetUniqueIdentifier();
    for (auto availableApp : availableApps)
    {
      availableAppsMsg = availableAppsMsg + ", " + availableApp->GetUniqueIdentifier();
    }
  }
  return availableAppsMsg;
}

SmartPointer<IExtension> ApplicationContainer::GetAppExtension(const QString& applicationId) const
{
  return extensionRegistry->GetExtension(PI_OSGI, PT_APPLICATIONS, applicationId);
}

void ApplicationContainer::Launch(ApplicationHandle* appHandle)
{
  bool isDefault = appHandle->IsDefault();
  if (appHandle->getApplicationDescriptor()->GetThreadType() == ApplicationDescriptor::FLAG_TYPE_MAIN_THREAD)
  {
    // use the ApplicationLauncher provided by the framework to ensure it is launched on the main thread
    //DefaultApplicationListener curDefaultApplicationListener = null;
    MainApplicationLauncher* curMissingAppLauncher = nullptr;
    ctkApplicationLauncher* appLauncher = nullptr;
    {
      QMutexLocker l(this);
      appLauncher = launcherTracker->getService();
      if (appLauncher == nullptr)
      {
        if (isDefault)
        {
          // we need to wait to allow the ApplicationLauncher to get registered;
          // save the handle to be launched as soon as the ApplicationLauncher is available
          defaultMainThreadAppHandle = appHandle;
          return;
        }
        throw ctkApplicationException(ctkApplicationException::APPLICATION_INTERNAL_ERROR,
                                      QString("The main thread is not available to launch the application: %1").arg(appHandle->getInstanceId()));
      }
      //curDefaultApplicationListener = defaultAppListener;
      curMissingAppLauncher = missingAppLauncher.data();
    }
    //if (curDefaultApplicationListener != nullptr)
    //{
    //  curDefaultApplicationListener->launch(appHandle);
    //}
    //else
    if (curMissingAppLauncher != nullptr)
    {
      curMissingAppLauncher->Launch(appHandle);
    }
    else
    {
      appLauncher->launch(appHandle, appHandle->GetArguments()[IApplicationContext::APPLICATION_ARGS]);
    }
  }
  else
  {
    if (isDefault)
    {
      //DefaultApplicationListener curDefaultApplicationListener = null;
      MainApplicationLauncher* curMissingAppLauncher = nullptr;
      ctkApplicationLauncher* appLauncher = nullptr;
      {
        QMutexLocker l(this);
        appLauncher = launcherTracker->getService();
        //if (defaultAppListener == nullptr)
        //{
        //  defaultAppListener = new DefaultApplicationListener(appHandle);
        //}
        //curDefaultApplicationListener = defaultAppListener;
        if (appLauncher == nullptr)
        {
          // we need to wait to allow the ApplicationLauncher to get registered;
          // save the default app listener to be launched as soon as the ApplicationLauncher is available
          //defaultMainThreadAppHandle = curDefaultApplicationListener;
          return;
        }
        curMissingAppLauncher = missingAppLauncher.data();
      }
      if (curMissingAppLauncher != nullptr)
      {
        //curMissingAppLauncher->Launch(curDefaultApplicationListener);
      }
      else
      {
        //appLauncher->Launch(curDefaultApplicationListener, QVariant());
      }
    }
    else
    {
      //AnyThreadAppLauncher.launchEclipseApplication(appHandle);
    }
  }
}

void ApplicationContainer::PluginChanged(const ctkPluginEvent& event)
{
  // if this is not the system bundle stopping then ignore the event
  if ((ctkPluginEvent::STOPPING & event.getType()) == 0 || event.getPlugin()->getPluginId() != 0)
  {
    return;
  }
  // The system bundle is stopping; better stop all applications and containers now
  StopAllApps();
}

void ApplicationContainer::StopAllApps()
{
  // get a stapshot of running applications
  QList<ctkServiceReference> runningRefs = context->getServiceReferences<ctkApplicationHandle>("(!(application.state=STOPPING))");
  for (auto runningRef : runningRefs)
  {
    ctkApplicationHandle* handle = context->getService<ctkApplicationHandle>(runningRef);
    try
    {
      if (handle != nullptr)
      {
        handle->destroy();
      }
    }
    catch (const std::exception&)
    {
      BERRY_WARN << QString("An error occurred while stopping the application: %1").arg(handle->getInstanceId());
    }
    if (handle != nullptr)
    {
      context->ungetService(runningRef);
    }
  }
}

QString ApplicationContainer::GetDefaultAppId() const
{
  if (!defaultAppId.isNull())
  {
    return defaultAppId;
  }

  // try plugin context properties
  defaultAppId = context->getProperty(ApplicationContainer::PROP_BLUEBERRY_APPLICATION).toString();
  if (!defaultAppId.isEmpty())
  {
    return defaultAppId;
  }

  //Derive the application from the product information
  defaultAppId = GetBranding() == nullptr ? QString("") : GetBranding()->GetApplication();
  return defaultAppId;
}

void ApplicationContainer::Lock(ApplicationHandle* /*appHandle*/)
{
  /*
  ApplicationDescriptor eclipseApp = (ApplicationDescriptor) appHandle.getApplicationDescriptor();
  {
    QMutexLocker l(&lock);
    switch (isLocked(eclipseApp))
    {
    case NOT_LOCKED :
      break;
    case LOCKED_SINGLETON_GLOBAL_RUNNING :
      throw new ApplicationException(ApplicationException.APPLICATION_NOT_LAUNCHABLE, NLS.bind(Messages.singleton_running, activeGlobalSingleton.getInstanceId()));
    case LOCKED_SINGLETON_GLOBAL_APPS_RUNNING :
      throw new ApplicationException(ApplicationException.APPLICATION_NOT_LAUNCHABLE, Messages.apps_running);
    case LOCKED_SINGLETON_SCOPED_RUNNING :
      throw new ApplicationException(ApplicationException.APPLICATION_NOT_LAUNCHABLE, NLS.bind(Messages.singleton_running, activeScopedSingleton.getInstanceId()));
    case LOCKED_SINGLETON_LIMITED_RUNNING :
      throw new ApplicationException(ApplicationException.APPLICATION_NOT_LAUNCHABLE, NLS.bind(Messages.max_running, eclipseApp.getApplicationId()));
    case LOCKED_MAIN_THREAD_RUNNING :
      throw new ApplicationException(ApplicationException.APPLICATION_NOT_LAUNCHABLE, NLS.bind(Messages.main_running, activeMain.getInstanceId()));
    default :
      break;
    }

    // ok we can now successfully lock the container
    switch (eclipseApp.getCardinalityType())
    {
    case ApplicationDescriptor::FLAG_CARD_SINGLETON_GLOGAL :
      activeGlobalSingleton = appHandle;
      break;
    case ApplicationDescriptor::FLAG_CARD_SINGLETON_SCOPED :
      activeScopedSingleton = appHandle;
      break;
    case ApplicationDescriptor::FLAG_CARD_LIMITED :
      if (activeLimited == null)
        activeLimited = new HashMap(3);
      ArrayList limited = (ArrayList) activeLimited.get(eclipseApp.getApplicationId());
      if (limited == null)
      {
        limited = new ArrayList(eclipseApp.getCardinality());
        activeLimited.put(eclipseApp.getApplicationId(), limited);
      }
      limited.add(appHandle);
      break;
    case ApplicationDescriptor::FLAG_CARD_UNLIMITED :
      break;
    default :
      break;
    }
    if (eclipseApp.getThreadType() == ApplicationDescriptor::FLAG_TYPE_MAIN_THREAD)
    {
      activeMain = appHandle;
    }
    activeHandles.add(appHandle);
    refreshAppDescriptors();
  }
  */
}

void ApplicationContainer::Unlock(ApplicationHandle* /*appHandle*/)
{
  /*
  QMutexLocker l(&lock);
  if (activeGlobalSingleton == appHandle)
    activeGlobalSingleton = null;
  else if (activeScopedSingleton == appHandle)
    activeScopedSingleton = null;
  else if (((ApplicationDescriptor) appHandle.getApplicationDescriptor()).getCardinalityType() == ApplicationDescriptor::FLAG_CARD_LIMITED) {
    if (activeLimited != null) {
      ArrayList limited = (ArrayList) activeLimited.get(((ApplicationDescriptor) appHandle.getApplicationDescriptor()).getApplicationId());
      if (limited != null)
        limited.remove(appHandle);
    }
  }
  if (activeMain == appHandle)
    activeMain = null;
  if (activeHandles.remove(appHandle))
    refreshAppDescriptors(); // only refresh descriptors if we really unlocked something
*/
}

int ApplicationContainer::IsLocked(const ApplicationDescriptor* /*eclipseApp*/) const
{
  /*
  {
    QMutexLocker l(&lock);
    if (activeGlobalSingleton != null)
      return LOCKED_SINGLETON_GLOBAL_RUNNING;
    switch (eclipseApp.getCardinalityType()) {
    case ApplicationDescriptor::FLAG_CARD_SINGLETON_GLOGAL :
      if (activeHandles.size() > 0)
        return LOCKED_SINGLETON_GLOBAL_APPS_RUNNING;
      break;
    case ApplicationDescriptor::FLAG_CARD_SINGLETON_SCOPED :
      if (activeScopedSingleton != null)
        return LOCKED_SINGLETON_SCOPED_RUNNING;
      break;
    case ApplicationDescriptor::FLAG_CARD_LIMITED :
      if (activeLimited != null) {
        ArrayList limited = (ArrayList) activeLimited.get(eclipseApp.getApplicationId());
        if (limited != null && limited.size() >= eclipseApp.getCardinality())
          return LOCKED_SINGLETON_LIMITED_RUNNING;
      }
      break;
    case ApplicationDescriptor::FLAG_CARD_UNLIMITED :
      break;
    default :
      break;
    }
    if (eclipseApp.getThreadType() == ApplicationDescriptor::FLAG_TYPE_MAIN_THREAD && activeMain != null)
      return LOCKED_MAIN_THREAD_RUNNING;
    return NOT_LOCKED;
  }
  */
  return NOT_LOCKED;
}

ctkApplicationLauncher* ApplicationContainer::addingService(const ctkServiceReference& reference)
{
  ctkApplicationLauncher* appLauncher = nullptr;
  ctkApplicationRunnable* appRunnable = nullptr;
  {
    QMutexLocker l(this);
    appLauncher = context->getService<ctkApplicationLauncher>(reference);
    // see if there is a default main threaded application waiting to run
    appRunnable = defaultMainThreadAppHandle;
    // null out so we do not attempt to start this handle again
    defaultMainThreadAppHandle = nullptr;
    if (appRunnable == nullptr && missingApp)
    {
      missingAppLauncher.reset(new MainApplicationLauncher(this));
      appRunnable = missingAppLauncher.data();
      missingApp = false;
    }
  }
  if (appRunnable != nullptr)
  {
    // found a main threaded app; start it now that the app launcher is available
    ApplicationHandle* handle = dynamic_cast<ApplicationHandle*>(appRunnable);
    appLauncher->launch(appRunnable, handle ? handle->GetArguments()[IApplicationContext::APPLICATION_ARGS] : QVariant());
  }
  return appLauncher;
}

void ApplicationContainer::modifiedService(const ctkServiceReference& /*reference*/, ctkApplicationLauncher* /*service*/)
{
  // Do nothing
}

void ApplicationContainer::removedService(const ctkServiceReference& /*reference*/, ctkApplicationLauncher* /*service*/)
{
  // Do nothing
}

void ApplicationContainer::Added(const QList<SmartPointer<IExtension> >& extensions)
{
  for (IExtension::Pointer extension : extensions)
  {
    CreateAppDescriptor(extension);
  }
}

void ApplicationContainer::Added(const QList<SmartPointer<IExtensionPoint> >& /*extensionPoints*/)
{
  // nothing
}

void ApplicationContainer::Removed(const QList<SmartPointer<IExtension> >& extensions)
{
  for (IExtension::Pointer extension : extensions)
  {
    RemoveAppDescriptor(extension->GetUniqueIdentifier());
  }
}

void ApplicationContainer::Removed(const QList<SmartPointer<IExtensionPoint> >& /*extensionPoints*/)
{
  // nothing
}

void ApplicationContainer::RefreshAppDescriptors()
{
  QMutexLocker l(&lock);
  for (ApplicationDescriptor* app : apps.values())
  {
    app->RefreshProperties();
  }
}



}
