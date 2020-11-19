/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
     * Storage for preferences.
     */
    mutable IPreferencesService* preferencesService;

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

public:

    /**
     * The <code>AbstractUIPlugin</code> implementation of this <code>Plugin</code>
     * method refreshes the plug-in actions.  Subclasses may extend this method,
     * but must send super <b>first</b>.
     * 
     * \param context
     */
    void start(ctkPluginContext* context) override;

    /**
     * The <code>AbstractUIPlugin</code> implementation of this <code>Plugin</code>
     * method saves this plug-in's preference and dialog stores and shuts down
     * its image registry (if they are in use). Subclasses may extend this
     * method, but must send super <b>last</b>. A try-finally statement should
     * be used where necessary to ensure that <code>super.shutdown()</code> is
     * always done.
     */
    void stop(ctkPluginContext* context) override;

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
