/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "berryLog.h"

#include "berryWorkbenchPlugin.h"

#include "berryWorkbenchRegistryConstants.h"
#include "berryWorkbench.h"
#include "berryPlatform.h"

#include "../berryImageDescriptor.h"

#include <Poco/String.h>

namespace berry
{

bool WorkbenchPlugin::DEBUG = false;

char WorkbenchPlugin::PREFERENCE_PAGE_CATEGORY_SEPARATOR = '/';

WorkbenchPlugin* WorkbenchPlugin::inst = 0;

WorkbenchPlugin::WorkbenchPlugin()
 : AbstractUIPlugin()
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
  delete editorRegistry;
  delete viewRegistry;
  delete perspRegistry;
  delete introRegistry;
}

bool WorkbenchPlugin::HasExecutableExtension(
    IConfigurationElement::Pointer element, const std::string& extensionName)
{

  std::string attr;
  if (element->GetAttribute(extensionName, attr))
    return true;
  std::string elementText(element->GetValue());
  if (elementText != "")
    return true;

  IConfigurationElement::vector children(element->GetChildren(extensionName));
  if (children.size() == 1)
  {
    if (children[0]->GetAttribute(WorkbenchRegistryConstants::ATT_CLASS, attr))
      return true;
  }
  return false;
}

bool WorkbenchPlugin::IsBundleLoadedForExecutableExtension(
    IConfigurationElement::Pointer element, const std::string& extensionName)
{
  IBundle::Pointer bundle(WorkbenchPlugin::GetBundleForExecutableExtension(element, extensionName));

  if (bundle.IsNull())
    return true;
  return bundle->GetState() == IBundle::BUNDLE_ACTIVE;
}

IBundle::Pointer WorkbenchPlugin::GetBundleForExecutableExtension(
    IConfigurationElement::Pointer element, const std::string& extensionName)
{
  // this code is derived heavily from
  // ConfigurationElement.createExecutableExtension.
  std::string prop;
  std::string executable;
  std::string contributorName;
  std::string::size_type i;

  if (extensionName != "")
    element->GetAttribute(extensionName, prop);
  else
  {
    // property not specified, try as element value
    prop = element->GetValue();
    if (prop != "")
    {
      Poco::trimInPlace(prop);
    }
  }

  if (prop == "")
  {
    // property not defined, try as a child element
    IConfigurationElement::vector exec(element->GetChildren(extensionName));
    if (exec.size() != 0)
      exec[0]->GetAttribute("plugin", contributorName); //$NON-NLS-1$
  }
  else
  {
    // simple property or element value, parse it into its components
    i = prop.find_first_of(':');
    if (i != std::string::npos)
      executable = Poco::trim(prop.substr(0, i));
    else
      executable = prop;

    i = executable.find_first_of('/');
    if (i != std::string::npos)
      contributorName = Poco::trim(executable.substr(0, i));

  }

  if (contributorName == "")
    contributorName = element->GetContributor();

  return Platform::GetBundle(contributorName);
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

  std::string targetID = Workbench::GetInstance()->GetPresentationId();
  presentationFactory = this->CreateExtension<IPresentationFactory>(
          WorkbenchRegistryConstants::PL_PRESENTATION_FACTORIES,
          "factory", targetID);
  if (presentationFactory == 0)
    WorkbenchPlugin::Log("Error creating presentation factory: " +
        targetID + " -- class is not an IPresentationFactory");

  return presentationFactory;
}

void WorkbenchPlugin::Log(const std::string& message)
{
  BERRY_INFO << "LOG: " << message << std::endl;
  //inst->GetLog().log(message);
}

void WorkbenchPlugin::Log(const Poco::RuntimeException& exc)
{
  BERRY_INFO << "LOG: " << exc.message() << std::endl;
  //inst->GetLog().log(exc);
}


void WorkbenchPlugin::Log(const std::string& message, const Poco::RuntimeException& t)
{
  PlatformException exc(message, t);
  WorkbenchPlugin::Log(exc);
}

void WorkbenchPlugin::Log(const std::string& clazz,
    const std::string& methodName, const Poco::RuntimeException& t)
{
  std::string msg = "Exception in " + clazz + "." + methodName + ": "
      + t.what();

  WorkbenchPlugin::Log(msg, t);
}

void WorkbenchPlugin::Log(IStatus::Pointer status)
{
  //getDefault().getLog().log(status);
  std::cout << status->ToString() << std::endl;
}

void WorkbenchPlugin::Start(IBundleContext::Pointer context)
{
  //context.addBundleListener(getBundleListener());
  AbstractUIPlugin::Start(context);
  bundleContext = context;


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

//const std::vector<IBundle::Pointer> WorkbenchPlugin::GetBundles()
//{
//  return bundleContext.IsNull() ? std::vector<IBundle::Pointer>() : bundleContext->GetBundles();
//}

IBundleContext::Pointer WorkbenchPlugin::GetBundleContext()
{
  return bundleContext;
}

void WorkbenchPlugin::Stop(IBundleContext::Pointer context)
{
  AbstractUIPlugin::Stop(context);

  delete perspRegistry;
}

bool WorkbenchPlugin::GetDataPath(Poco::Path& path)
{
  return this->GetStatePath(path);
}

}
