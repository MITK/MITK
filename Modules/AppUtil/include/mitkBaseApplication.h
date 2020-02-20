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
   *  - Put the application in \emph{safe mode} which catches unhandled
   *    exceptions thrown in the Qt event loop and displays an error
   *    message.
   *  - Put the application in \emph{single mode} which by default
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
   * <code>
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
   * </code>
   */
  class MITKAPPUTIL_EXPORT BaseApplication : public Poco::Util::Application
  {
  public:
    // Command line arguments

    static const QString ARG_APPLICATION;
    static const QString ARG_CLEAN;
    static const QString ARG_CONSOLELOG;
    static const QString ARG_DEBUG;
    static const QString ARG_FORCE_PLUGIN_INSTALL;
    static const QString ARG_HOME;
    static const QString ARG_NEWINSTANCE;
    static const QString ARG_NO_LAZY_REGISTRY_CACHE_LOADING;
    static const QString ARG_NO_REGISTRY_CACHE;
    static const QString ARG_PLUGIN_CACHE;
    static const QString ARG_PLUGIN_DIRS;
    static const QString ARG_PRELOAD_LIBRARY;
    static const QString ARG_PRODUCT;
    static const QString ARG_PROVISIONING;
    static const QString ARG_REGISTRY_MULTI_LANGUAGE;
    static const QString ARG_SPLASH_IMAGE;
    static const QString ARG_STORAGE_DIR;
    static const QString ARG_XARGS;

    // BlueBerry specific plugin framework properties

    static const QString PROP_APPLICATION;
    static const QString PROP_FORCE_PLUGIN_INSTALL;
    static const QString PROP_NEWINSTANCE;
    static const QString PROP_NO_LAZY_REGISTRY_CACHE_LOADING;
    static const QString PROP_NO_REGISTRY_CACHE;
    static const QString PROP_PRODUCT;
    static const QString PROP_REGISTRY_MULTI_LANGUAGE;

    BaseApplication(int argc, char **argv);
    ~BaseApplication() override;

    /**
     * Initialize the Qt library such that a QCoreApplication
     * instance is available and e.g. Qt widgets can be created.
     *
     * This is usually not called directly by the user.
     */
    void initializeQt();

    /**
     * Launches the BlueBerry framework and runs the default application
     * or the one specified in the PROP_APPLICATION framework property.
     *
     * @return The return code of the application after it was shut down.
     */
    int run() override;

    void printHelp(const std::string &name, const std::string &value);

    /**
     * Set the application name. Same as QCoreApplication::setApplicationName.
     * @param name The application name.
     */
    void setApplicationName(const QString &name);
    QString getApplicationName() const;

    /**
     * Set the organization name. Same as QCoreApplication::setOrganizationName.
     * @param name The organization name.
     */
    void setOrganizationName(const QString &name);
    QString getOrganizationName() const;

    /**
     * Set the organization domain. Same as QCoreApplication::setOrganizationDomain.
     * @param name The organization domain.
     */
    void setOrganizationDomain(const QString &name);
    QString getOrganizationDomain() const;

    /**
     * Put the application in single mode, which by default only allows
     * a single instance of the application to be created.
     *
     * Calling this method after run() has been called has no effect.
     *
     * @param singleMode
     */
    void setSingleMode(bool singleMode);
    bool getSingleMode() const;

    /**
     * Put the application in safe mode, catching exceptions from the
     * Qt event loop.
     *
     * @param safeMode
     */
    void setSafeMode(bool safeMode);
    bool getSafeMode() const;

    /**
     * Set a list of library names or absoulte file paths
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
     * as the executable and named <executable>.provisioning
     * is loaded if it exists. To disable parsing of provisioning
     * files, use an empty string as the argument. Use a
     * null QString (<code>QString::null</code>) to reset to the
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

    void setProperty(const QString &property, const QVariant &value);
    QVariant getProperty(const QString &property) const;

    void installTranslator(QTranslator*);

    bool isRunning();

    void sendMessage(const QByteArray);

  protected:
    void initialize(Poco::Util::Application &self) override;

    void uninitialize() override;

    int getArgc() const;
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
     * Runs the application for which the platform was started. The platform
     * must be running.
     * <p>
     * The given argument is passed to the application being run.  If it is an invalid QVariant
     * then the command line arguments used in starting the platform, and not consumed
     * by the platform code, are passed to the application as a <code>QStringList</code>.
     * </p>
     * @param argument the argument passed to the application. May be <code>invalid</code>
     * @return the result of running the application
     * @throws std::exception if anything goes wrong
     */
    int main(const std::vector<std::string> &args) override;

    /**
     * Define command line arguments
     * @param options
     */
    void defineOptions(Poco::Util::OptionSet &options) override;

    QSharedPointer<ctkPluginFramework> getFramework() const;
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

    /*
    * Initialize and display the splash screen if an image filename is given
    *
    */
    void initializeSplashScreen(QCoreApplication * application) const;

private:
    struct Impl;
    Impl* d;
  };
}

#endif // MITKBASEAPPLICATION_H
