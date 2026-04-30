/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBaseApplication_h
#define mitkBaseApplication_h

#include <MitkAppUtilExports.h>

#include <Poco/Util/Application.h>

#include <QString>
#include <QVariant>

class ctkPluginContext;
class ctkPluginFramework;

class QCoreApplication;
class QTranslator;

namespace mitk
{
  /**
   * A utility class for starting BlueBerry applications.
   *
   * In the simplest case, create an instance of this class and call run().
   * This will launch a CTK plugin framework instance and execute the
   * default application registered by a plug-in via the
   * org.blueberry.osgi.applications extension point.
   *
   * This class contains many convenience methods to:
   *  - Put the application in <em>safe mode</em> which catches unhandled
   *    exceptions thrown in the Qt event loop and displays an error
   *    message.
   *  - Put the application in <em>single mode</em> which by default
   *    sends the command line arguments to an already running instance
   *    of the same application instead of creating a second instance.
   *  - Add a list of library names which should be pre-loaded at
   *    application start-up, e.g. to speed up the initial launch during
   *    the caching process of the plug-in meta-data.
   *  - Set a custom provisioning file to start a specific set of CTK
   *    plug-ins during application start-up.
   *  - Set and get CTK plugin framework properties
   *
   * The behavior can further be customized by deriving from BaseApplication
   * and overriding specific methods, such as:
   *  - initializeLibraryPaths() to add specific library / plugin search paths
   *  - defineOptions(Poco::Util::OptionSet&) to define a custom set of
   *    command line options
   *  - getQApplication() to provide a custom QCoreApplication instance
   *
   * A simple but complete example:
   * \code
   * #include <mitkBaseApplication.h>
   *
   * int main(int argc, char* argv[])
   * {
   *   mitk::BaseApplication app(argc, argv);
   *   app.setApplicationName("MyApp");
   *   app.setOrganizationName("MyOrganization");
   *
   *   // Run the workbench
   *   return app.run();
   * }
   * \endcode
   */
  class MITKAPPUTIL_EXPORT BaseApplication : public Poco::Util::Application
  {
  public:
    /** \name Command Line Argument Constants
     *  String constants for recognized command line argument names.
     *  These are used as keys in the framework property map when the
     *  corresponding command line arguments are provided.
     */
    //\{
    static const QString ARG_APPLICATION;          ///< \brief Argument key for specifying the application extension id ("BlueBerry.application").
    static const QString ARG_CLEAN;                ///< \brief Argument key to clean the plugin cache on first init ("BlueBerry.clean").
    static const QString ARG_CONSOLELOG;           ///< \brief Argument key to enable console logging ("BlueBerry.consoleLog").
    static const QString ARG_DEBUG;                ///< \brief Argument key to enable debug mode ("BlueBerry.debug").
    static const QString ARG_FORCE_PLUGIN_INSTALL; ///< \brief Argument key to force installing plug-ins with the same symbolic name ("BlueBerry.forcePlugins").
    static const QString ARG_HOME;                 ///< \brief Argument key for the BlueBerry home directory ("BlueBerry.home").
    static const QString ARG_NEWINSTANCE;          ///< \brief Argument key to force a new instance in single mode ("BlueBerry.newInstance").
    static const QString ARG_NO_LAZY_REGISTRY_CACHE_LOADING; ///< \brief Argument key to disable lazy registry cache loading ("BlueBerry.noLazyRegistryCacheLoading").
    static const QString ARG_NO_REGISTRY_CACHE;    ///< \brief Argument key to disable the registry cache ("BlueBerry.noRegistryCache").
    static const QString ARG_PLUGIN_CACHE;         ///< \brief Argument key for the plugin cache directory ("BlueBerry.plugin_cache_dir").
    static const QString ARG_PLUGIN_DIRS;          ///< \brief Argument key for additional plugin directories ("BlueBerry.plugin_dirs").
    static const QString ARG_PRELOAD_LIBRARY;      ///< \brief Argument key for libraries to preload at startup ("BlueBerry.preloadLibrary").
    static const QString ARG_PRODUCT;              ///< \brief Argument key for the product id to launch ("BlueBerry.product").
    static const QString ARG_PROVISIONING;         ///< \brief Argument key for the provisioning file path ("BlueBerry.provisioning").
    static const QString ARG_REGISTRY_MULTI_LANGUAGE; ///< \brief Argument key to enable multi-language registry support ("BlueBerry.registryMultiLanguage").
    static const QString ARG_SPLASH_IMAGE;         ///< \brief Argument key for the splash screen image file path ("BlueBerry.splashscreen").
    static const QString ARG_STORAGE_DIR;          ///< \brief Argument key for the persistent storage directory ("BlueBerry.storageDir").
    static const QString ARG_XARGS;                ///< \brief Argument key for extended arguments ("xargs").
    static const QString ARG_LOG_QT_MESSAGES;      ///< \brief Argument key to enable logging of all Qt messages ("Qt.logMessages").
    static const QString ARG_SEGMENTATION_LABELSET_PRESET; ///< \brief Argument key for a label set preset file for new segmentations ("Segmentation.labelSetPreset").
    static const QString ARG_FULL_SCREEN_MODE;     ///< \brief Argument key to start the application in full screen mode ("MITK.fullscreen").
    static const QString ARG_PREFERENCES_OVERRIDE; ///< \brief Argument key for temporarily overriding preferences for this session ("MITK.preferences-override").
    static const QString ARG_PREFERENCES_PATCH;    ///< \brief Argument key for permanently patching preferences before session start ("MITK.preferences-patch").
    //\}

    /** \name BlueBerry Plugin Framework Property Constants
     *  String constants for CTK plugin framework property keys.
     */
    //\{
    static const QString PROP_APPLICATION;         ///< \brief Framework property key for the application extension id ("blueberry.application").
    static const QString PROP_FORCE_PLUGIN_INSTALL;///< \brief Framework property key for forcing plugin installation.
    static const QString PROP_NEWINSTANCE;         ///< \brief Framework property key for forcing a new instance.
    static const QString PROP_NO_LAZY_REGISTRY_CACHE_LOADING; ///< \brief Framework property key for disabling lazy registry cache loading.
    static const QString PROP_NO_REGISTRY_CACHE;   ///< \brief Framework property key for disabling the registry cache.
    static const QString PROP_PRODUCT;             ///< \brief Framework property key for the product id ("blueberry.product").
    static const QString PROP_REGISTRY_MULTI_LANGUAGE; ///< \brief Framework property key for multi-language registry support.
    //\}

    /**
     * \brief Construct a BaseApplication.
     * \param[in] argc The number of command line arguments.
     * \param[in] argv Array of command line argument strings.
     */
    BaseApplication(int argc, char **argv);

    /**
     * \brief Destructor. Releases internal resources.
     */
    ~BaseApplication() override;

    /**
     * Initialize the Qt library such that a QCoreApplication
     * instance is available and e.g. Qt widgets can be created.
     *
     * This is usually not called directly by the user.
     */
    void initializeQt();

    /**
     * \brief Launch the BlueBerry framework and run the default application.
     *
     * Initializes Poco, parses command line arguments, sets up the CTK plugin framework,
     * and runs the application specified by the PROP_APPLICATION framework property
     * (or the default application registered via the org.blueberry.osgi.applications
     * extension point).
     *
     * \return The return code of the application after it was shut down.
     * \throw Poco::Util::OptionException If an invalid command line option is encountered.
     * \throw mitk::Exception If a MITK-specific initialization error occurs.
     */
    int run() override;

    /**
     * \brief Print the command line help text and exit the application.
     *
     * This is used as a Poco option callback for the \c --help command line argument.
     *
     * \param[in] name The name of the option that triggered this callback (unused).
     * \param[in] value The value associated with the option (unused).
     */
    void printHelp(const std::string &name, const std::string &value);

    /**
     * \brief Set the application name.
     *
     * Equivalent to QCoreApplication::setApplicationName(). If a QCoreApplication instance
     * already exists, the name is forwarded to it immediately. The name is also stored
     * internally so it can be set on the QCoreApplication when it is created later.
     *
     * \param[in] name The application name.
     */
    void setApplicationName(const QString &name);

    /**
     * \brief Get the application name.
     * \return The current application name. Returns the QCoreApplication name if available,
     *         otherwise the internally stored name.
     */
    QString getApplicationName() const;

    /**
     * \brief Set the organization name.
     *
     * Equivalent to QCoreApplication::setOrganizationName(). If a QCoreApplication instance
     * already exists, the name is forwarded to it immediately.
     *
     * \param[in] name The organization name.
     */
    void setOrganizationName(const QString &name);

    /**
     * \brief Get the organization name.
     * \return The current organization name.
     */
    QString getOrganizationName() const;

    /**
     * \brief Set the organization domain.
     *
     * Equivalent to QCoreApplication::setOrganizationDomain(). If a QCoreApplication instance
     * already exists, the domain is forwarded to it immediately.
     *
     * \param[in] name The organization domain.
     */
    void setOrganizationDomain(const QString &name);

    /**
     * \brief Get the organization domain.
     * \return The current organization domain.
     */
    QString getOrganizationDomain() const;

    /**
     * \brief Enable or disable single-instance mode.
     *
     * In single mode, only one instance of the application is allowed to run.
     * If a second instance is started, the command line arguments are sent to the
     * already running instance instead of creating a new one (unless ARG_NEWINSTANCE
     * is specified).
     *
     * \pre Must be called before run(). Calling this method after run() has no effect.
     *
     * \param[in] singleMode If \c true, only a single application instance is allowed.
     */
    void setSingleMode(bool singleMode);

    /**
     * \brief Query whether single-instance mode is enabled.
     * \return \c true if single mode is enabled; \c false otherwise.
     */
    bool getSingleMode() const;

    /**
     * \brief Enable or disable safe mode for exception-catching in the Qt event loop.
     *
     * When safe mode is enabled, unhandled exceptions thrown during Qt event processing
     * are caught and displayed to the user instead of crashing the application.
     *
     * \param[in] safeMode If \c true, exception-safe event dispatching is enabled.
     * \note Has no effect if the QCoreApplication was already created externally
     *       (i.e., not by BaseApplication itself).
     */
    void setSafeMode(bool safeMode);

    /**
     * \brief Query whether safe mode is currently enabled.
     * \return \c true if safe mode is enabled; \c false otherwise.
     */
    bool getSafeMode() const;

    /**
     * \brief Enable or disable full screen mode.
     *
     * In full screen mode, the application window covers the entire screen without
     * a window frame.
     *
     * \pre Must be called before run(). Calling this method after run() has no effect.
     *
     * \param[in] fullScreenMode If \c true, the application starts in full screen mode.
     */
    void setFullScreenMode(bool fullScreenMode);

    /**
     * \brief Query whether full screen mode is enabled.
     * \return \c true if full screen mode is enabled; \c false otherwise.
     */
    bool getFullScreenMode() const;

    /**
     * Set a list of library names or absolute file paths
     * which should be loaded at application start-up. The name
     * and file path may contain a library version appended at the
     * end and separated by a '$' charactger.
     *
     * For example <code>liborg_mitk_gui_qt_common$1.0</code>.
     * Platform specific suffixes are appended automatically.
     *
     * @param libraryBaseNames A list of library base names.
     */
    void setPreloadLibraries(const QStringList &libraryBaseNames);

    /**
     * Get the list of library base names which should be pre-loaded.
     *
     * @return A list of pre-loaded libraries.
     */
    QStringList getPreloadLibraries() const;

    /**
     * Set the path to the provisioning file.
     *
     * By default a provisioning file located in the same directory
     * as the executable and named \<executable\>.provisioning
     * is loaded if it exists. To disable parsing of provisioning
     * files, use an empty string as the argument. Use a
     * null QString (\c QString::null ) to reset to the
     * default behaviour.
     *
     * @param filePath An absolute file path to the provisioning file.
     */
    void setProvisioningFilePath(const QString &filePath);

    /**
     * Get the file path to the provisioning file.
     * @return The provisioning file path.
     */
    QString getProvisioningFilePath() const;

    /**
     * \brief Set a CTK plugin framework property.
     *
     * Properties set through this method are passed to the CTK plugin framework
     * during initialization. They can also be used to store arbitrary application-level
     * configuration data.
     *
     * \param[in] property The property key.
     * \param[in] value The property value.
     */
    void setProperty(const QString &property, const QVariant &value);

    /**
     * \brief Get a CTK plugin framework property.
     * \param[in] property The property key.
     * \return The property value, or an invalid QVariant if the property is not set.
     */
    QVariant getProperty(const QString &property) const;

    /**
     * \brief Install a QTranslator on the underlying QCoreApplication.
     *
     * This is a convenience method that forwards to QCoreApplication::installTranslator().
     *
     * \param[in] translator The translator to install. Ownership is not transferred.
     * \pre A QCoreApplication instance must exist (i.e., initializeQt() must have been called
     *      or a QCoreApplication must have been created externally).
     */
    void installTranslator(QTranslator* translator);

    /**
     * \brief Check whether another instance of this application is already running.
     *
     * This method is only meaningful when the application is in single mode.
     *
     * \return \c true if another instance is already running.
     * \throw mitk::Exception If the application is not in single mode.
     * \pre Single mode must be enabled via setSingleMode().
     */
    bool isRunning();

    /**
     * \brief Send a message to the already running application instance.
     *
     * This method is only meaningful when the application is in single mode
     * and another instance is already running.
     *
     * \param[in] msg The message to send (e.g., serialized command line arguments).
     * \throw mitk::Exception If the application is not in single mode.
     * \pre Single mode must be enabled via setSingleMode().
     */
    void sendMessage(const QByteArray msg);

  protected:
    /**
     * \brief Initialize the application.
     *
     * Called by the Poco framework. This method performs the complete initialization
     * sequence: initializes Qt, loads configuration, sets up the CTK plugin framework
     * storage, initializes CppMicroServices, parses the provisioning file, and installs
     * library search paths.
     *
     * \param[in,out] self Reference to this application instance (required by Poco).
     */
    void initialize(Poco::Util::Application &self) override;

    /**
     * \brief Uninitialize the application.
     *
     * Stops the CTK plugin framework (waiting up to 10 seconds) and calls
     * Poco::Util::Application::uninitialize().
     */
    void uninitialize() override;

    /**
     * \brief Get the argument count.
     * \return The number of command line arguments.
     */
    int getArgc() const;

    /**
     * \brief Get the argument vector.
     * \return Pointer to the array of command line argument strings.
     */
    char **getArgv() const;

    /**
     * Get the framework storage directory for the CTK plugin
     * framework. This method is called in the initialize(Poco::Util::Application&)
     * method. It must not be called without a QCoreApplications instance.
     *
     * @return The CTK Plugin Framework storage directory.
     */
    virtual QString getCTKFrameworkStorageDir() const;

    /**
     * Initialize the CppMicroServices library.
     *
     * The default implementation set the CppMicroServices storage
     * path to the current ctkPluginConstants::FRAMEWORK_STORAGE property
     * value.
     *
     * This method is called in the initialize(Poco::Util::Application&)
     * after the CTK Plugin Framework storage directory property
     * was set.
     */
    virtual void initializeCppMicroServices();

    /**
     * Get the QCoreApplication object.
     *
     * This method is called in the initialize(Poco::Util::Application&)
     * method and must create a QCoreApplication instance if the
     * global qApp variable is not initialized yet.
     *
     * @return The current QCoreApplication instance. This method
     * never returns null.
     */
    virtual QCoreApplication *getQApplication() const;

    /**
     * Add plugin library search paths to the CTK Plugin Framework.
     *
     * This method is called in the nitialize(Poco::Util::Application&)
     * method after getQApplication() was called.
     */
    virtual void initializeLibraryPaths();

    /**
     * \brief Run the BlueBerry application via the CTK plugin framework.
     *
     * Called by the Poco framework after initialization is complete. This method
     * starts the CTK plugin framework launcher, which in turn starts all installed
     * plug-ins according to their auto-start setting and then runs the registered
     * application.
     *
     * \param[in] args The unconsumed command line arguments passed from Poco.
     * \return The exit code of the application.
     * \throw std::exception If the application encounters an error during execution.
     */
    int main(const std::vector<std::string> &args) override;

    /**
     * \brief Define the recognized command line options.
     *
     * Registers all MITK/BlueBerry-specific command line options (such as \c --help,
     * \c --BlueBerry.clean, \c --BlueBerry.application, etc.) as well as Qt-specific
     * options with the Poco option set.
     *
     * Override this method in subclasses to add custom command line options.
     *
     * \param[in,out] options The Poco option set to populate with recognized options.
     */
    void defineOptions(Poco::Util::OptionSet &options) override;

    /**
     * \brief Get the CTK plugin framework instance.
     * \return A shared pointer to the CTK plugin framework, or a null pointer if
     *         the framework has not been started.
     */
    QSharedPointer<ctkPluginFramework> getFramework() const;

    /**
     * \brief Get the CTK plugin context of the framework.
     * \return The plugin context of the running framework, or \c nullptr if the
     *         framework is not available.
     */
    ctkPluginContext *getFrameworkContext() const;

    /**
     * Get the initial properties for the CTK plugin framework.
     *
     * The returned map contains the initial framework properties for
     * initializing the CTK plugin framework. The value of specific
     * properties may change at runtime and differ from the initial
     * value.
     *
     * @return The initial CTK Plugin Framework properties.
     */
    QHash<QString, QVariant> getFrameworkProperties() const;

    /**
     * \brief Initialize and display the splash screen if configured.
     *
     * If the ARG_SPLASH_IMAGE property contains a valid path to an image file,
     * a QSplashScreen is created and displayed. The splash screen is automatically
     * closed when the application's main window becomes visible.
     *
     * \param[in] application The QCoreApplication instance used to process events
     *            while the splash screen is displayed.
     */
    void initializeSplashScreen(QCoreApplication * application) const;

private:
    struct Impl;
    Impl* d;
  };
}

#endif
