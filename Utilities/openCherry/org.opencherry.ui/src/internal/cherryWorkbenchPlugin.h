/*=========================================================================

Program:   openCherry Platform
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

#ifndef CHERRYWORKBENCHPLUGIN_H_
#define CHERRYWORKBENCHPLUGIN_H_

#include <Poco/Path.h>

#include <service/cherryIExtensionPoint.h>
#include <service/cherryIExtensionPointService.h>
#include <cherryPlatform.h>

#include "../cherryAbstractUIPlugin.h"
#include "../cherryPlatformUI.h"
#include "../presentations/cherryIPresentationFactory.h"


#include "cherryViewRegistry.h"
#include "cherryEditorRegistry.h"
#include "cherryPerspectiveRegistry.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui_internal
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
class WorkbenchPlugin : public AbstractUIPlugin {

private:

  //static const std::string UI_BUNDLE_ACTIVATOR = "org.opencherry.ui.internal.UIPlugin"; //$NON-NLS-1$

    // Default instance of the receiver
    static WorkbenchPlugin* inst;

    // The presentation factory
    IPresentationFactory* presentationFactory;

    // Manager that maps resources to descriptors of editors to use
    EditorRegistry editorRegistry;

    // The context within which this plugin was started.
    IBundleContext::Pointer bundleContext;

    // Other data.
    //WorkbenchPreferenceManager preferenceManager;

    ViewRegistry viewRegistry;

    PerspectiveRegistry* perspRegistry;

    //SharedImages sharedImages;


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

    ~WorkbenchPlugin();


    /**
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
//            const std::string& classAttribute) {
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
//            throw new CoreException(new Status(IStatus.ERROR, PI_WORKBENCH,
//                    IStatus.ERROR, WorkbenchMessages.WorkbenchPlugin_extension,e));
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
   * @since 3.3
   */
  static bool HasExecutableExtension(IConfigurationElement::Pointer element,
      const std::string& extensionName);

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
   * @since 3.3
   */
  static bool IsBundleLoadedForExecutableExtension(
      IConfigurationElement::Pointer element, const std::string& extensionName);

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
   * @since 3.3
   */
  static IBundle::Pointer GetBundleForExecutableExtension(IConfigurationElement::Pointer element, const std::string& extensionName);


  /**
     * Return the default instance of the receiver. This represents the runtime plugin.
     * @return WorkbenchPlugin
     * @see AbstractUIPlugin for the typical implementation pattern for plugin classes.
     */
    static WorkbenchPlugin* GetDefault();

    /**
     * Answer the manager that maps resource types to a the
     * description of the editor to use
     * @return IEditorRegistry the editor registry used
     * by this plug-in.
     */
    IEditorRegistry* GetEditorRegistry();

    /**
     * Returns the presentation factory with the given id, or <code>null</code> if not found.
     * @param targetID The id of the presentation factory to use.
     * @return IPresentationFactory or <code>null</code>
     * if not factory matches that id.
     */
    IPresentationFactory* GetPresentationFactory();


protected:

    /**
   * Returns the image registry for this plugin.
   *
   * Where are the images? The images (typically gifs) are found in the same
   * plugins directory.
   *
   * @see ImageRegistry
   *
   * Note: The workbench uses the standard JFace ImageRegistry to track its
   * images. In addition the class WorkbenchGraphicResources provides
   * convenience access to the graphics resources and fast field access for
   * some of the commonly used graphical images.
   */
//    ImageRegistry createImageRegistry();



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
    C* CreateExtension(const std::string extensionPointId, const std::string& elementName,
            const std::string& targetID) {
        const IExtensionPoint* extensionPoint = Platform::GetExtensionPointService()
                ->GetExtensionPoint("" + PlatformUI::PLUGIN_ID + "." + extensionPointId);
        if (extensionPoint == 0) {
            WorkbenchPlugin
                    ::Log("Unable to find extension. Extension point: " + extensionPointId + " not found"); //$NON-NLS-1$ //$NON-NLS-2$
            return 0;
        }

        // Loop through the config elements.
        IConfigurationElement::Pointer targetElement(0);
        IConfigurationElement::vector elements(Platform::GetExtensionPointService()
                                               ->GetConfigurationElementsFor("" + PlatformUI::PLUGIN_ID + "." + extensionPointId));
        for (unsigned int j = 0; j < elements.size(); j++) {
            if (elementName == "" || elementName == elements[j]->GetName()) {
                std::string strID;
                elements[j]->GetAttribute("id", strID);
                if (targetID == strID) {
                    targetElement = elements[j];
                    break;
                }
            }
        }
        if (targetElement.IsNull()) {
            // log it since we cannot safely display a dialog.
            WorkbenchPlugin::Log("Unable to find extension: " + targetID //$NON-NLS-1$
                    + " in extension point: " + extensionPointId); //$NON-NLS-1$
            return 0;
        }

        // Create the extension.
        try {
            return targetElement->CreateExecutableExtension<C>("class"); //$NON-NLS-1$
        } catch (CoreException e) {
            // log it since we cannot safely display a dialog.
            WorkbenchPlugin::Log("Unable to create extension: " + targetID //$NON-NLS-1$
                    + " in extension point: " + extensionPointId); //$NON-NLS-1$
        }
        return 0;
    }


public:

    /**
     * Return the perspective registry.
     * @return IPerspectiveRegistry. The registry for the receiver.
     */
    IPerspectiveRegistry* GetPerspectiveRegistry();


    /**
     * Get the preference manager.
     * @return PreferenceManager the preference manager for
     * the receiver.
     */
//    PreferenceManager getPreferenceManager();

    /**
     * Returns the shared images for the workbench.
     *
     * @return the shared image manager
     */
//    ISharedImages getSharedImages();



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
    static void Log(const std::string& message);

    /**
     * Log the throwable.
     * @param t
     */
    static void Log(const Poco::RuntimeException& exc);


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
    static void Log(const std::string& message, const Poco::RuntimeException& t);

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
    static void Log(const std::string& clazz, const std::string& methodName, const Poco::RuntimeException& t);


    /*
     *  (non-Javadoc)
     * @see org.osgi.framework.BundleActivator#start(org.osgi.framework.BundleContext)
     */
    void Start(IBundleContext::Pointer context);


  /**
     * Return an array of all bundles contained in this workbench.
     *
     * @return an array of bundles in the workbench or an empty array if none
     * @since 3.0
     */
    //const std::vector<IBundle::Pointer> GetBundles();

    /**
     * Returns the bundle context associated with the workbench plug-in.
     *
     * @return the bundle context
     * @since 3.1
     */
    IBundleContext::Pointer GetBundleContext();


    /* (non-Javadoc)
     * @see org.opencherry.ui.plugin.AbstractUIPlugin#stop(org.osgi.framework.BundleContext)
     */
    void Stop(IBundleContext::Pointer context);


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
     * @since 3.1
     */
    Poco::Path* GetDataPath();

};

}

#endif /*CHERRYWORKBENCHPLUGIN_H_*/
