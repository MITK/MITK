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

#ifndef BERRYABSTRACTUICTKPLUGIN_H_
#define BERRYABSTRACTUICTKPLUGIN_H_

#include <org_blueberry_ui_qt_Export.h>

#include <berryPlugin.h>

namespace berry {

template<class T> class SmartPointer;

struct IPreferences;
struct IPreferencesService;
struct IWorkbench;

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Abstract base class for plug-ins that integrate with the BlueBerry platform UI.
 * <p>
 * Subclasses obtain the following capabilities:
 * </p>
 * <p>
 * Preferences
 * <ul>
 * <li> The platform core runtime contains general support for plug-in
 *      preferences (<code>org.blueberry.core.runtime.Preferences</code>).
 *      This class provides appropriate conversion to the older JFace preference
 *      API (<code>org.blueberry.jface.preference.IPreferenceStore</code>).</li>
 * <li> The method <code>getPreferenceStore</code> returns the JFace preference
 *      store (cf. <code>Plugin.getPluginPreferences</code> which returns
 *      a core runtime preferences object.</li>
 * <li> Subclasses may reimplement <code>initializeDefaultPreferences</code>
 *      to set up any default values for preferences using JFace API. In this
 *      case, <code>initializeDefaultPluginPreferences</code> should not be
 *      overridden.</li>
 * <li> Subclasses may reimplement
 *      <code>initializeDefaultPluginPreferences</code> to set up any default
 *      values for preferences using core runtime API. In this
 *      case, <code>initializeDefaultPreferences</code> should not be
 *      overridden.</li>
 * <li> Preferences are also saved automatically on plug-in shutdown.
 *      However, saving preferences immediately after changing them is
 *      strongly recommended, since that ensures that preference settings
 *      are not lost even in the event of a platform crash.</li>
 * </ul>
 * Dialogs
 * <ul>
 * <li> The dialog store is read the first time <code>getDialogSettings</code>
 *      is called.</li>
 * <li> The dialog store allows the plug-in to "record" important choices made
 *      by the user in a wizard or dialog, so that the next time the
 *      wizard/dialog is used the widgets can be defaulted to better values. A
 *      wizard could also use it to record the last 5 values a user entered into
 *      an editable combo - to show "recent values". </li>
 * <li> The dialog store is found in the file whose name is given by the
 *      constant <code>FN_DIALOG_STORE</code>. A dialog store file is first
 *      looked for in the plug-in's read/write state area; if not found there,
 *      the plug-in's install directory is checked.
 *      This allows a plug-in to ship with a read-only copy of a dialog store
 *      file containing initial values for certain settings.</li>
 * <li> Plug-in code can call <code>saveDialogSettings</code> to cause settings to
 *      be saved in the plug-in's read/write state area. A plug-in may opt to do
 *      this each time a wizard or dialog is closed to ensure the latest
 *      information is always safe on disk. </li>
 * <li> Dialog settings are also saved automatically on plug-in shutdown.</li>
 * </ul>
 * <p>
 * For easy access to your plug-in object, use the singleton pattern. Declare a
 * static variable in your plug-in class for the singleton. Store the first
 * (and only) instance of the plug-in class in the singleton when it is created.
 * Then access the singleton when needed through a static <code>getDefault</code>
 * method.
 * </p>
 * <p>
 * See the description on {@link Plugin}.
 * </p>
 */
class BERRY_UI_QT AbstractUICTKPlugin : public Plugin
{
  Q_OBJECT

private:

    /**
     * The name of the dialog settings file (value
     * <code>"dialog_settings.xml"</code>).
     */
   static const QString FN_DIALOG_SETTINGS;

    /**
     * Storage for dialog and wizard data; <code>null</code> if not yet
     * initialized.
     */
    //DialogSettings dialogSettings = null;

    /**
     * Storage for preferences.
     */
    mutable IPreferencesService* preferencesService;

    /**
     * The bundle listener used for kicking off refreshPluginActions().
     */
    //BundleListener bundleListener;

public:

    /**
     * Creates an abstract UI plug-in runtime object.
     * <p>
     * Plug-in runtime classes are <code>ctkPluginActivator</code>s and so must
     * have an default constructor.  This method is called by the runtime when
     * the associated bundle is being activated.
     */
   AbstractUICTKPlugin();

    /**
     * Returns the dialog settings for this UI plug-in.
     * The dialog settings is used to hold persistent state data for the various
     * wizards and dialogs of this plug-in in the context of a workbench.
     * <p>
     * If an error occurs reading the dialog store, an empty one is quietly created
     * and returned.
     * </p>
     * <p>
     * Subclasses may override this method but are not expected to.
     * </p>
     *
     * @return the dialog settings
     */
//    IDialogSettings getDialogSettings();

    /**
     * Returns the preferences service for this UI plug-in.
     * This preferences service is used to hold persistent settings for this plug-in in
     * the context of a workbench. Some of these settings will be user controlled,
     * whereas others may be internal setting that are never exposed to the user.
     * <p>
     * If an error occurs reading the preferences service, an empty preference service is
     * quietly created, initialized with defaults, and returned.
     * </p>
     *
     * @return the preferences service
     */
    IPreferencesService* GetPreferencesService() const;

    SmartPointer<IPreferences> GetPreferences() const;

    /**
     * Returns the Platform UI workbench.
     * <p>
     * This method exists as a convenience for plugin implementors.  The
     * workbench can also be accessed by invoking <code>PlatformUI.getWorkbench()</code>.
     * </p>
     * @return IWorkbench the workbench for this plug-in
     */
    IWorkbench* GetWorkbench();


protected:

    /**
     * Returns a new image registry for this plugin-in.  The registry will be
     * used to manage images which are frequently used by the plugin-in.
     * <p>
     * The default implementation of this method creates an empty registry.
     * Subclasses may override this method if needed.
     * </p>
     *
     * @return ImageRegistry the resulting registry.
     * @see #getImageRegistry
     */
//    ImageRegistry createImageRegistry();

    /**
     * Initializes an image registry with images which are frequently used by the
     * plugin.
     * <p>
     * The image registry contains the images used by this plug-in that are very
     * frequently used and so need to be globally shared within the plug-in. Since
     * many OSs have a severe limit on the number of images that can be in memory
     * at any given time, each plug-in should only keep a small number of images in
     * its registry.
     * </p><p>
     * Implementors should create a JFace image descriptor for each frequently used
     * image.  The descriptors describe how to create/find the image should it be needed.
     * The image described by the descriptor is not actually allocated until someone
     * retrieves it.
     * </p><p>
     * Subclasses may override this method to fill the image registry.
     * </p>
     * @param reg the registry to initalize
     *
     * @see #getImageRegistry
     */
//    void initializeImageRegistry(ImageRegistry reg);

    /**
     * Loads the dialog settings for this plug-in.
     * The default implementation first looks for a standard named file in the
     * plug-in's read/write state area; if no such file exists, the plug-in's
     * install directory is checked to see if one was installed with some default
     * settings; if no file is found in either place, a new empty dialog settings
     * is created. If a problem occurs, an empty settings is silently used.
     * <p>
     * This framework method may be overridden, although this is typically
     * unnecessary.
     * </p>
     */
//    void loadDialogSettings();


    /**
     * Refreshes the actions for the plugin.
     * This method is called from <code>startup</code>.
     * <p>
     * This framework method may be overridden, although this is typically
     * unnecessary.
     * </p>
     */
//    void refreshPluginActions();

    /**
     * Saves this plug-in's dialog settings.
     * Any problems which arise are silently ignored.
     */
//    void saveDialogSettings();


public:

    /**
     * The <code>AbstractUIPlugin</code> implementation of this <code>Plugin</code>
     * method refreshes the plug-in actions.  Subclasses may extend this method,
     * but must send super <b>first</b>.
     */
    void start(ctkPluginContext* context);

    /**
     * The <code>AbstractUIPlugin</code> implementation of this <code>Plugin</code>
     * method saves this plug-in's preference and dialog stores and shuts down
     * its image registry (if they are in use). Subclasses may extend this
     * method, but must send super <b>last</b>. A try-finally statement should
     * be used where necessary to ensure that <code>super.shutdown()</code> is
     * always done.
     */
    void stop(ctkPluginContext* context);

    /**
     * Creates and returns a new image descriptor for an image file located
     * within the specified plug-in.
     * <p>
     * This is a convenience method that simply locates the image file in
     * within the plug-in (no image registries are involved). The path is
     * relative to the root of the plug-in, and takes into account files
     * coming from plug-in fragments. The path may include $arg$ elements.
     * However, the path must not have a leading "." or path separator.
     * Clients should use a path like "icons/mysample.gif" rather than
     * "./icons/mysample.gif" or "/icons/mysample.gif".
     * </p>
     *
     * @param pluginId the id of the plug-in containing the image file;
     * <code>null</code> is returned if the plug-in does not exist
     * @param imageFilePath the relative path of the image file, relative to the
     * root of the plug-in; the path must be legal
     * @return an image descriptor, or <code>null</code> if no image
     * could be found
     */
    static QIcon ImageDescriptorFromPlugin(
        const QString& pluginId, const QString& imageFilePath);

    static QIcon GetMissingIcon();
};

}  // namespace berry

#endif /*BERRYABSTRACTUICTKPLUGIN_H_*/
