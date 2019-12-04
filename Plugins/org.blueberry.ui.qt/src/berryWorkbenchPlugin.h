/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYWORKBENCHPLUGIN_H_
#define BERRYWORKBENCHPLUGIN_H_

#include <Poco/Path.h>

#include <berryIExtensionPoint.h>
#include <berryIExtensionRegistry.h>
#include <berryPlatform.h>
#include <berryCoreException.h>

#include "berryAbstractUICTKPlugin.h"
#include "berryPlatformUI.h"
#include "presentations/berryIPresentationFactory.h"


#include "internal/berryViewRegistry.h"
#include "internal/berryEditorRegistry.h"
#include "internal/berryPerspectiveRegistry.h"
#include "internal/intro/berryIntroRegistry.h"

namespace berry {

class QtStyleManager;

/**
 * \ingroup org_blueberry_ui_internal
 *
 * This class represents the TOP of the workbench UI world
 * A plugin class is effectively an application wrapper
 * for a plugin & its classes. This class should be thought
 * of as the workbench UI's application class.
 *
 * This class is responsible for tracking various registries
 * font, preference, graphics, dialog store.
 *
 * This class is explicitly referenced by the
 * workbench plugin's  "plugin.xml" and places it
 * into the UI start extension point of the main
 * overall application harness
 *
 * When is this class started?
 *      When the Application
 *      calls createExecutableExtension to create an executable
 *      instance of our workbench class.
 */
class BERRY_UI_QT WorkbenchPlugin : public AbstractUICTKPlugin
{

  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org_blueberry_ui_qt")
  Q_INTERFACES(ctkPluginActivator)

private:

  //static const QString UI_BUNDLE_ACTIVATOR = "org.blueberry.ui.internal.UIPlugin"; //$NON-NLS-1$

  // Default instance of the receiver
  static WorkbenchPlugin* inst;

  // The presentation factory
  IPresentationFactory* presentationFactory;

  // Manager that maps resources to descriptors of editors to use
  EditorRegistry* editorRegistry;

  // The context within which this plugin was started.
  ctkPluginContext* bundleContext;

  // Other data.
  //WorkbenchPreferenceManager preferenceManager;

  ViewRegistry* viewRegistry;

  PerspectiveRegistry* perspRegistry;

  IntroRegistry* introRegistry;

  //SharedImages sharedImages;

  QScopedPointer<QtStyleManager> styleManager;


public:

  /**
   * Global workbench ui plugin flag. Only workbench implementation is allowed to use this flag
   * All other plugins, examples, or test cases must *not* use this flag.
   */
  static bool DEBUG;

  /**
   * The character used to separate preference page category ids
   */
  static char PREFERENCE_PAGE_CATEGORY_SEPARATOR;


  /**
   * Create an instance of the WorkbenchPlugin. The workbench plugin is
   * effectively the "application" for the workbench UI. The entire UI
   * operates as a good plugin citizen.
   */
  WorkbenchPlugin();

  ~WorkbenchPlugin() override;


  /*
   * Creates an extension.  If the extension plugin has not
   * been loaded a busy cursor will be activated during the duration of
   * the load.
   *
   * @param element the config element defining the extension
   * @param classAttribute the name of the attribute carrying the class
   * @return the extension object
   * @throws CoreException if the extension cannot be created
   */
//    template<class E>
//    static E* CreateExtension(IConfigurationElement::ConstPointer element,
//            const QString& classAttribute) {
//        try {
//            // If plugin has been loaded create extension.
//            // Otherwise, show busy cursor then create extension.
//            if (BundleUtility.isActivated(element.getDeclaringExtension()
//                    .getNamespace())) {
//                return element.createExecutableExtension(classAttribute);
//            }
//            final Object[] ret = new Object[1];
//            final CoreException[] exc = new CoreException[1];
//            BusyIndicator.showWhile(null, new Runnable() {
//                public void run() {
//                    try {
//                        ret[0] = element
//                                .createExecutableExtension(classAttribute);
//                    } catch (CoreException e) {
//                        exc[0] = e;
//                    }
//                }
//            });
//            if (exc[0] != null) {
//        throw exc[0];
//      }
//            return ret[0];
//
//        } catch (CoreException core) {
//            throw core;
//        } catch (Exception e) {
//            throw new CoreException(new Status(IStatus.ERR, PI_WORKBENCH,
//                    IStatus.ERR, WorkbenchMessages.WorkbenchPlugin_extension,e));
//        }
//    }

  /**
   * Answers whether the provided element either has an attribute with the
   * given name or a child element with the given name with an attribute
   * called class.
   *
   * @param element
   *            the element to test
   * @param extensionName
   *            the name of the extension to test for
   * @return whether or not the extension is declared
   */
  static bool HasExecutableExtension(const IConfigurationElement::Pointer& element,
                                     const QString& extensionName);

  /**
   * Checks to see if the provided element has the syntax for an executable
   * extension with a given name that resides in a bundle that is already
   * active. Determining the bundle happens in one of two ways:<br/>
   * <ul>
   * <li>The element has an attribute with the specified name or element text
   * in the form <code>bundle.id/class.name[:optional attributes]</code></li>
   * <li>The element has a child element with the specified name that has a
   * <code>plugin</code> attribute</li>
   * </ul>
   *
   * @param element
   *            the element to test
   * @param extensionName
   *            the name of the extension to test for
   * @return whether or not the bundle expressed by the above criteria is
   *         active. If the bundle cannot be determined then the state of the
   *         bundle that declared the element is returned.
   */
  static bool IsBundleLoadedForExecutableExtension(
      const IConfigurationElement::Pointer& element, const QString& extensionName);

  /**
   * Returns the bundle that contains the class referenced by an executable
   * extension. Determining the bundle happens in one of two ways:<br/>
   * <ul>
   * <li>The element has an attribute with the specified name or element text
   * in the form <code>bundle.id/class.name[:optional attributes]</code></li>
   * <li>The element has a child element with the specified name that has a
   * <code>plugin</code> attribute</li>
   * </ul>
   *
   * @param element
   *            the element to test
   * @param extensionName
   *            the name of the extension to test for
   * @return the bundle referenced by the extension. If that bundle cannot be
   *         determined the bundle that declared the element is returned. Note
   *         that this may be <code>null</code>.
   */
  static QSharedPointer<ctkPlugin> GetBundleForExecutableExtension(
      const IConfigurationElement::Pointer& element, const QString& extensionName);


  /**
   * Return the default instance of the receiver. This represents the runtime plugin.
   * @return WorkbenchPlugin
   * @see AbstractUICTKPlugin for the typical implementation pattern for plugin classes.
   */
  static WorkbenchPlugin* GetDefault();

  std::size_t GetBundleCount();

  /**
   * Answer the manager that maps resource types to a the
   * description of the editor to use
   * @return IEditorRegistry the editor registry used
   * by this plug-in.
   */
  IEditorRegistry* GetEditorRegistry();

  /**
   * Answer the element factory for an id, or <code>null</code. if not found.
   * @param targetID
   * @return IElementFactory
   */
  IElementFactory* GetElementFactory(const QString& targetID) const;

  /**
   * Returns the presentation factory with the given id, or <code>null</code> if not found.
   * @param targetID The id of the presentation factory to use.
   * @return IPresentationFactory or <code>null</code>
   * if not factory matches that id.
   */
  IPresentationFactory* GetPresentationFactory();


private:

  /**
   * Looks up the configuration element with the given id on the given extension point
   * and instantiates the class specified by the class attributes.
   *
   * @param extensionPointId the extension point id (simple id)
   * @param elementName the name of the configuration element, or <code>null</code>
   *   to match any element
   * @param targetID the target id
   * @return the instantiated extension object, or <code>null</code> if not found
   */
  template<class C>
  C* CreateExtension(const QString& extensionPointId, const QString& elementName,
                     const QString& targetID)
  {
    IExtensionPoint::Pointer extensionPoint = Platform::GetExtensionRegistry()
        ->GetExtensionPoint(PlatformUI::PLUGIN_ID() + "." + extensionPointId);
    if (extensionPoint == 0)
    {
      WorkbenchPlugin::Log("Unable to find extension. Extension point: " +
                           extensionPointId + " not found");
      return nullptr;
    }

    // Loop through the config elements.
    IConfigurationElement::Pointer targetElement(nullptr);
    QList<IConfigurationElement::Pointer> elements(
          Platform::GetExtensionRegistry()->GetConfigurationElementsFor(PlatformUI::PLUGIN_ID() + "." + extensionPointId));
    for (int j = 0; j < elements.size(); j++)
    {
      if (elementName == "" || elementName == elements[j]->GetName())
      {
        QString strID = elements[j]->GetAttribute("id");
        if (targetID == strID)
        {
            targetElement = elements[j];
            break;
        }
      }
    }
    if (targetElement.IsNull())
    {
      // log it since we cannot safely display a dialog.
      WorkbenchPlugin::Log("Unable to find extension: " + targetID
                           + " in extension point: " + extensionPointId);
      return nullptr;
    }

    // Create the extension.
    try
    {
      return targetElement->CreateExecutableExtension<C>("class");
    }
    catch (const CoreException& /*e*/)
    {
      // log it since we cannot safely display a dialog.
      WorkbenchPlugin::Log("Unable to create extension: " + targetID
                           + " in extension point: " + extensionPointId);
    }
    return nullptr;
  }


public:

  /**
   * Return the perspective registry.
   * @return IPerspectiveRegistry. The registry for the receiver.
   */
  IPerspectiveRegistry* GetPerspectiveRegistry();


  /**
   * Returns the introduction registry.
   *
   * @return the introduction registry.
   */
  IIntroRegistry* GetIntroRegistry();

  /*
   * Get the preference manager.
   * @return PreferenceManager the preference manager for
   * the receiver.
   */
  //PreferenceManager getPreferenceManager();

  /**
   * Answer the view registry.
   * @return IViewRegistry the view registry for the
   * receiver.
   */
  IViewRegistry* GetViewRegistry();


  /**
   * Logs the given message to the platform log.
   *
   * If you have an exception in hand, call log(String, Throwable) instead.
   *
   * If you have a status object in hand call log(String, IStatus) instead.
   *
   * This convenience method is for internal use by the Workbench only and
   * must not be called outside the Workbench.
   *
   * @param message
   *            A high level UI message describing when the problem happened.
   */
  static void Log(const QString &message);

  /**
   * Log the throwable.
   * @param t
   */
  static void Log(const ctkException& exc);


  /**
   * Logs the given message and throwable to the platform log.
   *
   * If you have a status object in hand call log(String, IStatus) instead.
   *
   * This convenience method is for internal use by the Workbench only and
   * must not be called outside the Workbench.
   *
   * @param message
   *            A high level UI message describing when the problem happened.
   * @param t
   *            The throwable from where the problem actually occurred.
   */
  static void Log(const QString &message, const ctkException& t);

  /**
   * Logs the given throwable to the platform log, indicating the class and
   * method from where it is being logged (this is not necessarily where it
   * occurred).
   *
   * This convenience method is for internal use by the Workbench only and
   * must not be called outside the Workbench.
   *
   * @param clazz
   *            The calling class.
   * @param methodName
   *            The calling method name.
   * @param t
   *            The throwable from where the problem actually occurred.
   */
  static void Log(const QString &clazz, const QString &methodName, const ctkException& t);

  /**
   * Logs the given message and status to the platform log.
   *
   * This convenience method is for internal use by the Workbench only and
   * must not be called outside the Workbench.
   *
   * @param message
   *            A high level UI message describing when the problem happened.
   *            May be <code>null</code>.
   * @param status
   *            The status describing the problem. Must not be null.
   */
  static void Log(const QString& message, const SmartPointer<IStatus>& status);

  /**
   * Log the status to the default log.
   * @param status
   */
  static void Log(const SmartPointer<IStatus>& status);

  /*
   *  (non-Javadoc)
   * @see org.osgi.framework.BundleActivator#start(org.osgi.framework.BundleContext)
   */
  void start(ctkPluginContext* context) override;


  /*
   * Return an array of all bundles contained in this workbench.
   *
   * @return an array of bundles in the workbench or an empty array if none
   */
  //const QList<IBundle::Pointer> GetBundles();

  /**
   * Returns the bundle context associated with the workbench plug-in.
   *
   * @return the bundle context
   */
  ctkPluginContext* GetPluginContext();


  /* (non-Javadoc)
   * @see org.blueberry.ui.plugin.AbstractUICTKPlugin#stop(org.osgi.framework.BundleContext)
   */
  void stop(ctkPluginContext* context) override;


  /**
   * FOR INTERNAL WORKBENCH USE ONLY.
   *
   * Returns the path to a location in the file system that can be used
   * to persist/restore state between workbench invocations.
   * If the location did not exist prior to this call it will  be created.
   * Returns <code>null</code> if no such location is available.
   *
   * @return path to a location in the file system where this plug-in can
   * persist data between sessions, or <code>null</code> if no such
   * location is available.
   */
  QString GetDataLocation() const;

};

}

#endif /*BERRYWORKBENCHPLUGIN_H_*/
