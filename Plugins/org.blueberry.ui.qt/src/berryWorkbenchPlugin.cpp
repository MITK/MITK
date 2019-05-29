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

#include "internal/berryWorkbenchRegistryConstants.h"
#include "internal/berryWorkbench.h"
#include "berryPlatform.h"

#include "internal/intro/berryEditorIntroAdapterPart.h"
#include "internal/defaultpresentation/berryQtWorkbenchPresentationFactory.h"

#include "berryQtStyleManager.h"

#include "berryExtensionFactory.h"
#include "internal/berryQtWorkbenchTweaklet.h"
#include "internal/berryQtWorkbenchPageTweaklet.h"
#include "internal/berryQtWidgetsTweaklet.h"
#include "internal/dialogs/berryPerspectivesPreferencePage.h"
#include "internal/berryQtStylePreferencePage.h"
#include "internal/berryStatusUtil.h"
#include "internal/berryHandlerServiceFactory.h"
#include "internal/berryMenuServiceFactory.h"
#include "internal/berryCommandServiceFactory.h"
#include "internal/berryWorkbenchSourceProvider.h"
#include "berryObjectString.h"
#include "berryObjects.h"
#include "internal/berryPolicy.h"
#include "internal/berryHandlerAuthority.h"

#include "internal/berryOpenPerspectivePropertyTester.h"
#include "internal/berryPerspectiveParameterValues.h"

#include "internal/handlers/berryCloseAllPerspectivesHandler.h"
#include "internal/handlers/berryClosePerspectiveHandler.h"
#include "internal/handlers/berryDynamicHelpHandler.h"
#include "internal/handlers/berryHelpContentsHandler.h"
#include "internal/handlers/berryIntroHandler.h"
#include "internal/handlers/berryOpenInNewWindowHandler.h"
#include "internal/handlers/berryNewEditorHandler.h"
#include "internal/handlers/berryQuitHandler.h"
#include "internal/handlers/berryResetPerspectiveHandler.h"
#include "internal/handlers/berrySavePerspectiveHandler.h"
#include "internal/handlers/berryShowPerspectiveHandler.h"
#include "internal/handlers/berryShowViewHandler.h"

#include "berryIQtStyleManager.h"
#include "berryIContributor.h"
#include "berryILog.h"
#include "berryIElementFactory.h"

#include "berryIExtension.h"

#include <QDebug>
#include <QPrinterInfo>
#include <QSvgGenerator>

namespace berry
{

bool WorkbenchPlugin::DEBUG = false;

char WorkbenchPlugin::PREFERENCE_PAGE_CATEGORY_SEPARATOR = '/';

WorkbenchPlugin* WorkbenchPlugin::inst = nullptr;

WorkbenchPlugin::WorkbenchPlugin()
 : AbstractUICTKPlugin()
{
  inst = this;
  presentationFactory = nullptr;
  editorRegistry = nullptr;
  viewRegistry = nullptr;
  perspRegistry = nullptr;
  introRegistry = nullptr;
}

WorkbenchPlugin::~WorkbenchPlugin()
{
  delete presentationFactory;
  delete editorRegistry;
  delete viewRegistry;
  delete perspRegistry;
  delete introRegistry;

  inst = nullptr;
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

IPerspectiveRegistry* WorkbenchPlugin::GetPerspectiveRegistry()
{
  if (perspRegistry == nullptr)
  {
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

IIntroRegistry* WorkbenchPlugin::GetIntroRegistry()
{
  if (introRegistry == nullptr)
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

IElementFactory* WorkbenchPlugin::GetElementFactory(const QString& targetID) const
{
  // Get the extension point registry.
  IExtensionPoint::Pointer extensionPoint = Platform::GetExtensionRegistry()->GetExtensionPoint(
                                              PlatformUI::PLUGIN_ID(),
                                              WorkbenchRegistryConstants::PL_ELEMENT_FACTORY);

  IElementFactory* factory = nullptr;
  if (!extensionPoint)
  {
    WorkbenchPlugin::Log("Unable to find element factory. Extension point: " +
                         WorkbenchRegistryConstants::PL_ELEMENT_FACTORY + " not found");
    return factory;
  }

  // Loop through the config elements.
  IConfigurationElement::Pointer targetElement;
  QList<IConfigurationElement::Pointer> configElements =
      extensionPoint->GetConfigurationElements();
  for (int j = 0; j < configElements.size(); j++)
  {
    QString strID = configElements[j]->GetAttribute("id");
    if (targetID == strID)
    {
      targetElement = configElements[j];
      break;
    }
  }
  if (!targetElement)
  {
    // log it since we cannot safely display a dialog.
    WorkbenchPlugin::Log("Unable to find element factory: " + targetID);
    return factory;
  }

  // Create the extension.
  try
  {
    factory = targetElement->CreateExecutableExtension<IElementFactory>("class");
  }
  catch (const CoreException& e)
  {
    // log it since we cannot safely display a dialog.
    WorkbenchPlugin::Log("Unable to create element factory.", e.GetStatus());
    factory = nullptr;
  }
  return factory;
}

IPresentationFactory* WorkbenchPlugin::GetPresentationFactory() {
  if (presentationFactory != nullptr) return presentationFactory;

  QString targetID = Workbench::GetInstance()->GetPresentationId();
  presentationFactory = this->CreateExtension<IPresentationFactory>(
                          WorkbenchRegistryConstants::PL_PRESENTATION_FACTORIES,
                          "factory", targetID);
  if (presentationFactory == nullptr)
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
  // Dummy code to force linkage to Qt5PrintSupport (issue with GCC 7.3)
  QPrinterInfo forceQt5PrintSupportLinkage;
  forceQt5PrintSupportLinkage.isNull();

  // Same for Qt5Svg
  QSvgGenerator forceQt5SvgLinkage;
  forceQt5SvgLinkage.title();

  //context.addBundleListener(getBundleListener());
  AbstractUICTKPlugin::start(context);
  bundleContext = context;

  AbstractSourceProvider::DEBUG = Policy::DEBUG_SOURCES();

  HandlerAuthority::DEBUG = Policy::DEBUG_HANDLERS();
  HandlerAuthority::DEBUG_PERFORMANCE = Policy::DEBUG_HANDLERS_PERFORMANCE();
  HandlerAuthority::DEBUG_VERBOSE = Policy::DEBUG_HANDLERS_VERBOSE();
  HandlerAuthority::DEBUG_VERBOSE_COMMAND_ID = Policy::DEBUG_HANDLERS_VERBOSE_COMMAND_ID();

  BERRY_REGISTER_EXTENSION_CLASS(EditorIntroAdapterPart, context)

  BERRY_REGISTER_EXTENSION_CLASS(ExtensionFactory, context)

  BERRY_REGISTER_EXTENSION_CLASS(QtWidgetsTweaklet, context)
  BERRY_REGISTER_EXTENSION_CLASS(QtWorkbenchTweaklet, context)
  BERRY_REGISTER_EXTENSION_CLASS(QtWorkbenchPageTweaklet, context)
  BERRY_REGISTER_EXTENSION_CLASS(QtWorkbenchPresentationFactory, context)

  BERRY_REGISTER_EXTENSION_CLASS(PerspectivesPreferencePage, context)
  BERRY_REGISTER_EXTENSION_CLASS(QtStylePreferencePage, context)

  BERRY_REGISTER_EXTENSION_CLASS(HandlerServiceFactory, context)
  BERRY_REGISTER_EXTENSION_CLASS(MenuServiceFactory, context)
  BERRY_REGISTER_EXTENSION_CLASS(CommandServiceFactory, context)

  BERRY_REGISTER_EXTENSION_CLASS(WorkbenchSourceProvider, context)

  BERRY_REGISTER_EXTENSION_CLASS(OpenPerspectivePropertyTester, context)
  BERRY_REGISTER_EXTENSION_CLASS(PerspectiveParameterValues, context)

  BERRY_REGISTER_EXTENSION_CLASS(HelpContentsHandler, context)
  BERRY_REGISTER_EXTENSION_CLASS(DynamicHelpHandler, context)
  BERRY_REGISTER_EXTENSION_CLASS(IntroHandler, context)
  BERRY_REGISTER_EXTENSION_CLASS(OpenInNewWindowHandler, context)
  BERRY_REGISTER_EXTENSION_CLASS(NewEditorHandler, context)
  BERRY_REGISTER_EXTENSION_CLASS(QuitHandler, context)
  BERRY_REGISTER_EXTENSION_CLASS(ShowPerspectiveHandler, context)
  BERRY_REGISTER_EXTENSION_CLASS(ShowViewHandler, context)
  BERRY_REGISTER_EXTENSION_CLASS(SavePerspectiveHandler, context)
  BERRY_REGISTER_EXTENSION_CLASS(ClosePerspectiveHandler, context)
  BERRY_REGISTER_EXTENSION_CLASS(CloseAllPerspectivesHandler, context)
  BERRY_REGISTER_EXTENSION_CLASS(ResetPerspectiveHandler, context)

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
  perspRegistry = nullptr;
}

QString WorkbenchPlugin::GetDataLocation() const
{
  QFileInfo fileInfo = bundleContext->getDataFile("");
  if (!fileInfo.isWritable()) return QString();
  return fileInfo.absoluteFilePath();
}

}
