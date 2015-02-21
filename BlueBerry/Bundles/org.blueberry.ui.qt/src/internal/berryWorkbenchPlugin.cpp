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

#include "berryLog.h"

#include "berryWorkbenchPlugin.h"

#include "berryWorkbenchRegistryConstants.h"
#include "berryWorkbench.h"
#include "berryPlatform.h"

#include "intro/berryEditorIntroAdapterPart.h"
#include "defaultpresentation/berryQtWorkbenchPresentationFactory.h"

#include "berryQtStyleManager.h"

#include "berryQtWorkbenchTweaklet.h"
#include "berryQtWorkbenchPageTweaklet.h"
#include "berryQtWidgetsTweaklet.h"
#include "berryQtStylePreferencePage.h"
#include "berryStatusUtil.h"
#include "berryHandlerServiceFactory.h"
#include "berryMenuServiceFactory.h"
#include "berryCommandServiceFactory.h"
#include "berryWorkbenchSourceProvider.h"
#include "berryObjectString.h"
#include "berryObjects.h"

#include "berryShowViewHandler.h"

#include "berryIQtStyleManager.h"
#include "berryIContributor.h"
#include "berryILog.h"

#include "berryIExtension.h"

#include <QDebug>

namespace berry
{

bool WorkbenchPlugin::DEBUG = false;

char WorkbenchPlugin::PREFERENCE_PAGE_CATEGORY_SEPARATOR = '/';

WorkbenchPlugin* WorkbenchPlugin::inst = 0;

WorkbenchPlugin::WorkbenchPlugin()
 : AbstractUICTKPlugin()
{
  inst = this;
  presentationFactory = 0;
  editorRegistry = 0;
  viewRegistry = 0;
  perspRegistry = 0;
  introRegistry = 0;
}

WorkbenchPlugin::~WorkbenchPlugin()
{
  delete presentationFactory;
  delete editorRegistry;
  delete viewRegistry;
  delete perspRegistry;
  delete introRegistry;

  inst = 0;
}

bool WorkbenchPlugin::HasExecutableExtension(
    const IConfigurationElement::Pointer& element, const QString& extensionName)
{
  if (!element->GetAttribute(extensionName).isNull()) return true;

  QString elementText = element->GetValue();
  if (!elementText.isEmpty()) return true;

  QList<IConfigurationElement::Pointer> children(element->GetChildren(extensionName));
  if (children.size() == 1)
  {
    if (!(children[0]->GetAttribute(WorkbenchRegistryConstants::ATT_CLASS).isNull()))
      return true;
  }
  return false;
}

bool WorkbenchPlugin::IsBundleLoadedForExecutableExtension(
    const IConfigurationElement::Pointer& element, const QString& extensionName)
{
  QSharedPointer<ctkPlugin> plugin = WorkbenchPlugin::GetBundleForExecutableExtension(element, extensionName);

  if (plugin.isNull())
    return true;
  return plugin->getState() == ctkPlugin::ACTIVE;
}

QSharedPointer<ctkPlugin> WorkbenchPlugin::GetBundleForExecutableExtension(
    const IConfigurationElement::Pointer& element, const QString& extensionName)
{
  // this code is derived heavily from
  // ConfigurationElement.createExecutableExtension.
  QString prop;
  QString executable;
  QString contributorName;
  int i = 0;

  if (!extensionName.isNull())
    prop = element->GetAttribute(extensionName);
  else
  {
    // property not specified, try as element value
    prop = element->GetValue();
    if (!prop.isNull())
    {
      prop = prop.trimmed();
      if (prop.isEmpty())
        prop = QString();
    }
  }

  if (prop.isNull())
  {
    // property not defined, try as a child element
    QList<IConfigurationElement::Pointer> exec(element->GetChildren(extensionName));
    if (!exec.isEmpty())
      contributorName = exec[0]->GetAttribute("plugin");
  }
  else
  {
    // simple property or element value, parse it into its components
    i = prop.indexOf(':');
    if (i != -1)
      executable = prop.left(i).trimmed();
    else
      executable = prop;

    i = executable.indexOf('/');
    if (i != -1)
      contributorName = executable.left(i).trimmed();
  }

  if (contributorName.isNull())
    contributorName = element->GetContributor()->GetName();

  return Platform::GetPlugin(contributorName);
}

WorkbenchPlugin* WorkbenchPlugin::GetDefault()
{
  return inst;
}

std::size_t WorkbenchPlugin::GetBundleCount()
{
  // TODO BundleContext GetBundles
  //return bundleContext->GetBundles().size();
  return 0;
}


//    ImageRegistry createImageRegistry() {
//        return WorkbenchImages.getImageRegistry();
//    }


IPerspectiveRegistry* WorkbenchPlugin::GetPerspectiveRegistry() {
  if (perspRegistry == 0) {
    perspRegistry = new PerspectiveRegistry();

  // the load methods can touch on WorkbenchImages if an image is
  // missing so we need to wrap the call in
  // a startup block for the case where a custom descriptor exists on
  // startup that does not have an image
  // associated with it. See bug 196352.
  //StartupThreading.runWithoutExceptions(new StartupRunnable() {
  //  public void runWithException() throws Throwable {
      perspRegistry->Load();
  //  }
  //});

  }
  return perspRegistry;
}


//    PreferenceManager getPreferenceManager() {
//        if (preferenceManager == null) {
//            preferenceManager = new WorkbenchPreferenceManager(
//                    PREFERENCE_PAGE_CATEGORY_SEPARATOR);
//
//            //Get the pages from the registry
//            PreferencePageRegistryReader registryReader = new PreferencePageRegistryReader(
//                    getWorkbench());
//            registryReader
//                    .loadFromRegistry(Platform.getExtensionRegistry());
//            preferenceManager.addPages(registryReader.getTopLevelNodes());
//
//        }
//        return preferenceManager;
//    }


//    ISharedImages getSharedImages() {
//        if (sharedImages == null) {
//      sharedImages = new SharedImages();
//    }
//        return sharedImages;
//    }

IIntroRegistry* WorkbenchPlugin::GetIntroRegistry()
{
  if (introRegistry == 0)
  {
    introRegistry = new IntroRegistry();
  }
  return introRegistry;
}

IViewRegistry* WorkbenchPlugin::GetViewRegistry()
{
  if (!viewRegistry)
    viewRegistry = new ViewRegistry();

  return viewRegistry;
}

IEditorRegistry* WorkbenchPlugin::GetEditorRegistry()
{
  if (!editorRegistry)
    editorRegistry = new EditorRegistry();

  return editorRegistry;
}

IPresentationFactory* WorkbenchPlugin::GetPresentationFactory() {
  if (presentationFactory != 0) return presentationFactory;

  QString targetID = Workbench::GetInstance()->GetPresentationId();
  presentationFactory = this->CreateExtension<IPresentationFactory>(
          WorkbenchRegistryConstants::PL_PRESENTATION_FACTORIES,
          "factory", targetID);
  if (presentationFactory == 0)
    WorkbenchPlugin::Log("Error creating presentation factory: " +
        targetID + " -- class is not an IPresentationFactory");

  return presentationFactory;
}

void WorkbenchPlugin::Log(const QString& message)
{
  BERRY_INFO << "LOG: " << message << std::endl;
  //inst->GetLog().log(message);
}

void WorkbenchPlugin::Log(const ctkException &exc)
{
  QString str;
  QDebug dbg(&str);
  dbg << exc.printStackTrace();
  BERRY_INFO << "LOG: " << str << std::endl;
  //inst->GetLog().log(exc);
}


void WorkbenchPlugin::Log(const QString& message, const ctkException &t)
{
  PlatformException exc(message, t);
  WorkbenchPlugin::Log(exc);
}

void WorkbenchPlugin::Log(const QString& clazz,
                          const QString& methodName, const ctkException &t)
{
  QString msg = QString("Exception in ") + clazz + "." + methodName + ": "
      + t.what();

  WorkbenchPlugin::Log(msg, t);
}

void WorkbenchPlugin::Log(const QString& message, const SmartPointer<IStatus>& status)
{
  //1FTUHE0: ITPCORE:ALL - API - Status & logging - loss of semantic info

  if (!message.isEmpty())
  {
    GetDefault()->GetLog()->Log(StatusUtil::NewStatus(IStatus::ERROR_TYPE, message, BERRY_STATUS_LOC));
  }

  GetDefault()->GetLog()->Log(status);
}

void WorkbenchPlugin::Log(const SmartPointer<IStatus>& status)
{
  GetDefault()->GetLog()->Log(status);
}

void WorkbenchPlugin::start(ctkPluginContext* context)
{
  //context.addBundleListener(getBundleListener());
  AbstractUICTKPlugin::start(context);
  bundleContext = context;

  BERRY_REGISTER_EXTENSION_CLASS(EditorIntroAdapterPart, context)

  BERRY_REGISTER_EXTENSION_CLASS(QtWidgetsTweaklet, context)
  BERRY_REGISTER_EXTENSION_CLASS(QtWorkbenchTweaklet, context)
  BERRY_REGISTER_EXTENSION_CLASS(QtWorkbenchPageTweaklet, context)
  BERRY_REGISTER_EXTENSION_CLASS(QtWorkbenchPresentationFactory, context)

  BERRY_REGISTER_EXTENSION_CLASS(QtStylePreferencePage, context)

  BERRY_REGISTER_EXTENSION_CLASS(HandlerServiceFactory, context)
  BERRY_REGISTER_EXTENSION_CLASS(MenuServiceFactory, context)
  BERRY_REGISTER_EXTENSION_CLASS(CommandServiceFactory, context)

  BERRY_REGISTER_EXTENSION_CLASS(WorkbenchSourceProvider, context)

  BERRY_REGISTER_EXTENSION_CLASS(ShowViewHandler, context)

  styleManager.reset(new QtStyleManager());
  context->registerService<berry::IQtStyleManager>(styleManager.data());

  // The UI plugin needs to be initialized so that it can install the callback in PrefUtil,
  // which needs to be done as early as possible, before the workbench
  // accesses any API preferences.
//  Bundle uiBundle = Platform.getBundle(PlatformUI.PLUGIN_ID);
//  try
//  {
//    // Attempt to load the activator of the ui bundle.  This will force lazy start
//    // of the ui bundle.  Using the bundle activator class here because it is a
//    // class that needs to be loaded anyway so it should not cause extra classes
//    // to be loaded.
//    if(uiBundle != null)
//    uiBundle.loadClass(UI_BUNDLE_ACTIVATOR);
//  }
//  catch (ClassNotFoundException e)
//  {
//    WorkbenchPlugin.log("Unable to load UI activator", e); //$NON-NLS-1$
//  }
  /*
   * DO NOT RUN ANY OTHER CODE AFTER THIS LINE.  If you do, then you are
   * likely to cause a deadlock in class loader code.  Please see Bug 86450
   * for more information.
   */

}

//const QList<IBundle::Pointer> WorkbenchPlugin::GetBundles()
//{
//  return bundleContext.IsNull() ? QList<IBundle::Pointer>() : bundleContext->GetBundles();
//}

ctkPluginContext* WorkbenchPlugin::GetPluginContext()
{
  return bundleContext;
}

void WorkbenchPlugin::stop(ctkPluginContext* context)
{
  AbstractUICTKPlugin::stop(context);

  styleManager.reset();

  delete perspRegistry;
  // avoid possible crash, see bug #18399
  perspRegistry = 0;
}

QString WorkbenchPlugin::GetDataLocation() const
{
  QFileInfo fileInfo = bundleContext->getDataFile("");
  if (!fileInfo.isWritable()) return QString();
  return fileInfo.absoluteFilePath();
}

}

#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
  Q_EXPORT_PLUGIN2(org_blueberry_ui_qt, berry::WorkbenchPlugin)
#endif
