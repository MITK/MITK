/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkBaseApplication.h"
#include "mitkLogMacros.h"
#include "mitkExceptionMacro.h"

#include "QmitkSafeApplication.h"
#include "QmitkSingleApplication.h"
#include "mitkProvisioningInfo.h"

#include <ctkPluginConstants.h>
#include <ctkPluginFramework.h>
#include <ctkPluginFrameworkLauncher.h>
#include <ctkPluginFramework_global.h>

#include <usModuleSettings.h>

#include <Poco/Util/HelpFormatter.h>

#include <QSplashScreen>
#include <QRunnable>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QStringList>
#include <QTime>

#include <iostream>
#include <string>

#include <QVTKOpenGLWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>

namespace mitk
{
  QString BaseApplication::ARG_NEWINSTANCE = "BlueBerry.newInstance";
  QString BaseApplication::ARG_CLEAN = "BlueBerry.clean";
  QString BaseApplication::ARG_APPLICATION = "BlueBerry.application";
  QString BaseApplication::ARG_PRODUCT = "BlueBerry.product";
  QString BaseApplication::ARG_HOME = "BlueBerry.home";
  QString BaseApplication::ARG_STORAGE_DIR = "BlueBerry.storageDir";
  QString BaseApplication::ARG_PLUGIN_CACHE = "BlueBerry.plugin_cache_dir";
  QString BaseApplication::ARG_PLUGIN_DIRS = "BlueBerry.plugin_dirs";
  QString BaseApplication::ARG_FORCE_PLUGIN_INSTALL = "BlueBerry.forcePlugins";
  QString BaseApplication::ARG_PRELOAD_LIBRARY = "BlueBerry.preloadLibrary";
  QString BaseApplication::ARG_PROVISIONING = "BlueBerry.provisioning";
  QString BaseApplication::ARG_DEBUG = "BlueBerry.debug";
  QString BaseApplication::ARG_CONSOLELOG = "BlueBerry.consoleLog";
  QString BaseApplication::ARG_TESTPLUGIN = "BlueBerry.testplugin";
  QString BaseApplication::ARG_TESTAPPLICATION = "BlueBerry.testapplication";

  QString BaseApplication::ARG_SPLASH_IMAGE = "BlueBerry.splashscreen";

  QString BaseApplication::ARG_NO_REGISTRY_CACHE = "BlueBerry.noRegistryCache";
  QString BaseApplication::ARG_NO_LAZY_REGISTRY_CACHE_LOADING = "BlueBerry.noLazyRegistryCacheLoading";
  QString BaseApplication::ARG_REGISTRY_MULTI_LANGUAGE = "BlueBerry.registryMultiLanguage";

  QString BaseApplication::ARG_XARGS = "xargs";

  QString BaseApplication::PROP_NEWINSTANCE = BaseApplication::ARG_NEWINSTANCE;
  QString BaseApplication::PROP_FORCE_PLUGIN_INSTALL = BaseApplication::ARG_FORCE_PLUGIN_INSTALL;
  QString BaseApplication::PROP_NO_REGISTRY_CACHE = BaseApplication::ARG_NO_REGISTRY_CACHE;
  QString BaseApplication::PROP_NO_LAZY_REGISTRY_CACHE_LOADING = BaseApplication::ARG_NO_LAZY_REGISTRY_CACHE_LOADING;
  QString BaseApplication::PROP_REGISTRY_MULTI_LANGUAGE = BaseApplication::ARG_REGISTRY_MULTI_LANGUAGE;

  QString BaseApplication::PROP_PRODUCT = "blueberry.product";
  QString BaseApplication::PROP_APPLICATION = "blueberry.application";
  QString BaseApplication::PROP_TESTPLUGIN = "BlueBerry.testplugin";
  QString BaseApplication::PROP_TESTAPPLICATION = "BlueBerry.testapplication";

  static void outputQtMessage(QtMsgType type, const QMessageLogContext &, const QString &msg)
  {
    auto message = msg.toStdString();

    switch (type)
    {
      case QtDebugMsg:
        MITK_DEBUG << message;
        break;

      case QtInfoMsg:
        MITK_INFO << message;
        break;

      case QtWarningMsg:
        MITK_WARN << message;
        break;

      case QtCriticalMsg:
        MITK_ERROR << message;
        break;

      case QtFatalMsg:
        MITK_ERROR << message;
        abort();

      default:
        MITK_INFO << message;
        break;
    }
  }

  class SplashCloserCallback : public QRunnable
  {
  public:
    SplashCloserCallback(QSplashScreen* splashscreen)
    {
      this->m_Splashscreen = splashscreen;
    }

    void run() override
    {
      this->m_Splashscreen->close();
    }

  private:
    QSplashScreen* m_Splashscreen;
  };

  struct BaseApplication::Impl
  {
    ctkProperties m_FWProps;

    QScopedPointer<QCoreApplication> m_QApp;

    int m_Argc;
    char **m_Argv;

    QString m_AppName;
    QString m_OrgaName;
    QString m_OrgaDomain;

    bool m_SingleMode;
    bool m_SafeMode;

    QSplashScreen* m_Splashscreen;
    SplashCloserCallback* m_SplashscreenClosingCallback;

    QStringList m_PreloadLibs;
    QString m_ProvFile;

    Impl(int argc, char **argv)
      : m_Argc(argc), m_Argv(argv), m_SingleMode(false), m_SafeMode(true),
        m_Splashscreen(nullptr), m_SplashscreenClosingCallback(nullptr)
    {
#ifdef Q_OS_MAC
      /*
       * This is a workaround for bug 19080:
       * On macOS the prosess serial number is passed as an commandline argument (-psn_<NUMBER>)
       * if the application is started via the.app bundle.
       * This option is unknown, which causes a Poco exception.
       * Since this is done by the system we have to manually remove the argument here.
       */

      int newArgc = m_Argc - 1;
      char **newArgs = new char *[newArgc];
      bool argFound(false);
      for (int i = 0; i < m_Argc; ++i)
      {
        if (QString::fromLatin1(m_Argv[i]).contains("-psn"))
        {
          argFound = true;
        }
        else
        {
          newArgs[i] = m_Argv[i];
        }
      }
      if (argFound)
      {
        m_Argc = newArgc;
        m_Argv = newArgs;
      }
#endif
    }

    QVariant getProperty(const QString &property) const
    {
      auto iter = m_FWProps.find(property);
      return iter == m_FWProps.end() ? QVariant() : iter.value();
    }

    void handleBooleanOption(const std::string &name, const std::string & /*value*/)
    {
      QString fwKey = QString::fromStdString(name);

      // translate some keys to proper framework properties
      if (fwKey == ARG_CONSOLELOG)
      {
        fwKey = ctkPluginFrameworkLauncher::PROP_CONSOLE_LOG;
      }

      // For all other options we use the command line option name as the
      // framework property key.
      m_FWProps[fwKey] = true;
    }

    void handlePreloadLibraryOption(const std::string & /*name*/, const std::string &value)
    {
      m_PreloadLibs.push_back(QString::fromStdString(value));
    }

    void handleClean(const std::string & /*name*/, const std::string & /*value*/)
    {
      m_FWProps[ctkPluginConstants::FRAMEWORK_STORAGE_CLEAN] = ctkPluginConstants::FRAMEWORK_STORAGE_CLEAN_ONFIRSTINIT;
    }

    void initializeCTKPluginFrameworkProperties(Poco::Util::LayeredConfiguration &configuration)
    {
      // add all configuration key / value pairs as framework properties
      Poco::Util::LayeredConfiguration::Keys keys;
      Poco::Util::LayeredConfiguration::Keys keyStack;
      configuration.keys(keyStack);
      std::vector<std::string> keyChain;
      while (!keyStack.empty())
      {
        std::string currSubKey = keyStack.back();
        if (!keyChain.empty() && keyChain.back() == currSubKey)
        {
          keyChain.pop_back();
          keyStack.pop_back();
          continue;
        }
        Poco::Util::LayeredConfiguration::Keys subKeys;
        configuration.keys(currSubKey, subKeys);
        if (subKeys.empty())
        {
          keyStack.pop_back();
          std::string finalKey;
          for (auto k = keyChain.begin(); k != keyChain.end(); ++k)
          {
            finalKey += *k + ".";
          }
          finalKey += currSubKey;
          keys.push_back(finalKey);
        }
        else
        {
          keyChain.push_back(currSubKey);
          for (auto s : subKeys)
          {
            keyStack.push_back(s);
          }
        }
      }

      for (auto key : keys)
      {
        QString qKey = QString::fromStdString(key);
        if (configuration.hasProperty(key))
        {
          // ini and command line options overwrite already inserted keys
          m_FWProps[qKey] = QString::fromStdString(configuration.getString(key));
        }
      }
    }

    void parseProvisioningFile(const QString &filePath)
    {
      // Skip parsing if the file path is empty
      if (filePath.isEmpty())
        return;

      bool consoleLog = this->getProperty(ctkPluginFrameworkLauncher::PROP_CONSOLE_LOG).toBool();

      // read initial plugins from a provisioning file
      QStringList pluginsToStart;

      QFileInfo provFile(filePath);

      if (provFile.exists())
      {
        MITK_INFO(consoleLog) << "Using provisioning file: " << qPrintable(provFile.absoluteFilePath());
        ProvisioningInfo provInfo(provFile.absoluteFilePath());
        // it can still happen, that the encoding is not compatible with the fromUtf8 function ( i.e. when manipulating
        // the LANG variable
        // in such case, the QStringList in provInfo is empty which we can easily check for
        if (provInfo.getPluginDirs().empty())
        {
          MITK_ERROR << "Cannot search for provisioning file, the retrieved directory list is empty.\n"
                     << "This can occur if there are some special (non-ascii) characters in the install path.";
        }
        else
        {
          foreach (QString pluginPath, provInfo.getPluginDirs())
          {
            ctkPluginFrameworkLauncher::addSearchPath(pluginPath);
          }

          // bool forcePluginOverwrite = this->getProperty(ARG_FORCE_PLUGIN_INSTALL).toBool();
          QList<QUrl> pluginUrlsToStart = provInfo.getPluginsToStart();
          for (auto url : pluginUrlsToStart)
          {
            pluginsToStart.push_back(url.toString());
          }
          // foreach(QUrl pluginUrl, provInfo.getPluginsToInstall())
          //{
          // TODO for "uninstall", we need a proper configuration agent, e.g. a dedicated
          // plug-in for provisioning of the platform
          /*
          if (forcePluginOverwrite)
          {
            uninstallPugin(pluginUrl, context);
          }
          */
          // try
          //{
          // MITK_INFO(consoleLog) << "Installing CTK plug-in from: " << pluginUrl.toString().toStdString();
          /*
          QSharedPointer<ctkPlugin> plugin = context->installPlugin(pluginUrl);
          if (pluginsToStart.contains(pluginUrl))
          {
            m_CTKPluginsToStart << plugin->getPluginId();
          }
          */
          /*
        }
        catch (const ctkPluginException& e)
        {
          QString errorMsg;
          QDebug dbg(&errorMsg);
          dbg << e.printStackTrace();
          BERRY_ERROR << qPrintable(errorMsg);
        }
        */
          //}
        }
      }
      else
      {
        MITK_INFO(consoleLog) << "No provisioning file set.";
      }

      if (!pluginsToStart.isEmpty())
      {
        m_FWProps[ctkPluginFrameworkLauncher::PROP_PLUGINS] = pluginsToStart;

        // Use transient start with declared activation policy (this helps when
        // the provisioning file changes and some plug-ins should not be installed
        // in the application any more).
        ctkPlugin::StartOptions startOptions(ctkPlugin::START_TRANSIENT | ctkPlugin::START_ACTIVATION_POLICY);
        m_FWProps[ctkPluginFrameworkLauncher::PROP_PLUGINS_START_OPTIONS] = static_cast<int>(startOptions);
      }
    }
  };

  BaseApplication::BaseApplication(int argc, char **argv) : Application(), d(new Impl(argc, argv))
  {
  }

  BaseApplication::~BaseApplication()
  {
    if (d->m_Splashscreen != nullptr)
    {
      delete(d->m_Splashscreen);
    }
    if (d->m_SplashscreenClosingCallback != nullptr)
    {
      delete(d->m_SplashscreenClosingCallback);
    }
  }

  void BaseApplication::printHelp(const std::string & /*name*/, const std::string & /*value*/)
  {
    Poco::Util::HelpFormatter help(this->options());
    help.setAutoIndent();
    help.setCommand(this->commandName());
    help.format(std::cout);

    exit(EXIT_OK);
  }

  void BaseApplication::setApplicationName(const QString &name)
  {
    if (qApp)
    {
      qApp->setApplicationName(name);
    }
    d->m_AppName = name;
  }

  QString BaseApplication::getApplicationName() const
  {
    if (qApp)
    {
      return qApp->applicationName();
    }
    return d->m_AppName;
  }

  void BaseApplication::setOrganizationName(const QString &name)
  {
    if (qApp)
    {
      qApp->setOrganizationName(name);
    }
    d->m_OrgaName = name;
  }

  QString BaseApplication::getOrganizationName() const
  {
    if (qApp)
      return qApp->organizationName();
    return d->m_OrgaName;
  }

  void BaseApplication::setOrganizationDomain(const QString &domain)
  {
    if (qApp)
    {
      qApp->setOrganizationDomain(domain);
    }
    d->m_OrgaDomain = domain;
  }

  QString BaseApplication::getOrganizationDomain() const
  {
    if (qApp)
      return qApp->organizationDomain();
    return d->m_OrgaDomain;
  }

  void BaseApplication::setSingleMode(bool singleMode)
  {
    if (qApp)
      return;
    d->m_SingleMode = singleMode;
  }

  bool BaseApplication::getSingleMode() const { return d->m_SingleMode; }
  void BaseApplication::setSafeMode(bool safeMode)
  {
    if (qApp && !d->m_QApp)
      return;
    d->m_SafeMode = safeMode;
    if (d->m_QApp)
    {
      if (getSingleMode())
      {
        static_cast<QmitkSingleApplication *>(d->m_QApp.data())->setSafeMode(safeMode);
      }
      else
      {
        static_cast<QmitkSafeApplication *>(d->m_QApp.data())->setSafeMode(safeMode);
      }
    }
  }

  bool BaseApplication::getSafeMode() const { return d->m_SafeMode; }
  void BaseApplication::setPreloadLibraries(const QStringList &libraryBaseNames)
  {
    d->m_PreloadLibs = libraryBaseNames;
  }

  QStringList BaseApplication::getPreloadLibraries() const { return d->m_PreloadLibs; }
  void BaseApplication::setProvisioningFilePath(const QString &filePath) { d->m_ProvFile = filePath; }
  QString BaseApplication::getProvisioningFilePath() const
  {
    QString provFilePath = d->m_ProvFile;

    // A null QString means look up a default provisioning file
    if (provFilePath.isNull() && qApp)
    {
      QFileInfo appFilePath(QCoreApplication::applicationFilePath());
      QDir basePath(QCoreApplication::applicationDirPath());

      QString provFileName = appFilePath.baseName() + ".provisioning";

      QFileInfo provFile(basePath.absoluteFilePath(provFileName));

#ifdef Q_OS_MAC
      /*
       * On Mac, if started from the build directory the .provisioning file is located at:
       * <MITK-build/bin/MitkWorkbench.provisioning>
       * but the executable path is:
       * <MITK-build/bin/MitkWorkbench.app/Contents/MacOS/MitkWorkbench>
       * In this case we have to cdUp threetimes.
       *
       * During packaging however the MitkWorkbench.provisioning file is placed at the same
       * level like the executable, hence nothing has to be done.
       */

      if (!provFile.exists())
      {
        basePath.cdUp();
        basePath.cdUp();
        basePath.cdUp();
        provFile = basePath.absoluteFilePath(provFileName);
      }
#endif

      if (provFile.exists())
      {
        provFilePath = provFile.absoluteFilePath();
      }
#ifdef CMAKE_INTDIR
      else
      {
        basePath.cdUp();
        provFile.setFile(basePath.absoluteFilePath(provFileName));
        if (provFile.exists())
        {
          provFilePath = provFile.absoluteFilePath();
        }
      }
#endif
    }
    return provFilePath;
  }

  void BaseApplication::initializeQt()
  {
    if (qApp)
      return;

    // If previously parameters have been set we have to store them
    // to hand them through to the application
    QString appName = this->getApplicationName();
    QString orgName = this->getOrganizationName();
    QString orgDomain = this->getOrganizationDomain();

    // Create a QCoreApplication instance
    this->getQApplication();

    // provide parameters to QCoreApplication
    this->setApplicationName(appName);
    this->setOrganizationName(orgName);
    this->setOrganizationDomain(orgDomain);

    qInstallMessageHandler(outputQtMessage);
  }

  void BaseApplication::initialize(Poco::Util::Application &self)
  {
    // 1. Call the super-class method
    Poco::Util::Application::initialize(self);

    // 2. Initialize the Qt framework (by creating a QCoreApplication)
    this->initializeQt();

    // 3. Seed the random number generator, once at startup.
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    // 4. Load the "default" configuration, which involves parsing
    //    an optional <executable-name>.ini file and parsing any
    //    command line arguments
    this->loadConfiguration();

    // 5. Add configuration data from the command line and the
    //    optional <executable-name>.ini file as CTK plugin
    //    framework properties.
    d->initializeCTKPluginFrameworkProperties(this->config());

  // 6. Initialize splash screen if an image path is provided
  //    in the .ini file
  this->initializeSplashScreen(qApp);

  // 7. Set the custom CTK Plugin Framework storage directory
    QString storageDir = this->getCTKFrameworkStorageDir();
    if (!storageDir.isEmpty())
    {
      d->m_FWProps[ctkPluginConstants::FRAMEWORK_STORAGE] = storageDir;
    }

  // 8. Set the library search paths and the pre-load library property
    this->initializeLibraryPaths();
    QStringList preloadLibs = this->getPreloadLibraries();
    if (!preloadLibs.isEmpty())
    {
      d->m_FWProps[ctkPluginConstants::FRAMEWORK_PRELOAD_LIBRARIES] = preloadLibs;
    }

  // 9. Initialize the CppMicroServices library.
    //    The initializeCppMicroServices() method reuses the
    //    FRAMEWORK_STORAGE property, so we call it after the
    //    getCTKFrameworkStorageDir method.
    this->initializeCppMicroServices();

  // 10. Parse the (optional) provisioning file and set the
    //    correct framework properties.
    d->parseProvisioningFile(this->getProvisioningFilePath());

    // Finally, set the CTK Plugin Framework properties
    ctkPluginFrameworkLauncher::setFrameworkProperties(d->m_FWProps);
  }

  void BaseApplication::uninitialize()
  {
    QSharedPointer<ctkPluginFramework> pfw = this->getFramework();
    if (pfw)
    {
      pfw->stop();

      // wait 10 seconds for the CTK plugin framework to stop
      pfw->waitForStop(10000);
    }

    Poco::Util::Application::uninitialize();
  }

  int BaseApplication::getArgc() const { return d->m_Argc; }
  char **BaseApplication::getArgv() const { return d->m_Argv; }
  QString BaseApplication::getCTKFrameworkStorageDir() const
  {
    QString storageDir;
    if (this->getSingleMode())
    {
      // This function checks if an instance is already running
      // and either sends a message to it (containing the command
      // line arguments) or checks if a new instance was forced by
      // providing the BlueBerry.newInstance command line argument.
      // In the latter case, a path to a temporary directory for
      // the new application's storage directory is returned.
      storageDir = handleNewAppInstance(
        static_cast<QtSingleApplication *>(d->m_QApp.data()), d->m_Argc, d->m_Argv, ARG_NEWINSTANCE);
    }

    if (storageDir.isEmpty())
    {
      // This is a new instance and no other instance is already running. We specify
      // the storage directory here (this is the same code as in berryInternalPlatform.cpp
      // so that we can re-use the location for the persistent data location of the
      // the CppMicroServices library.

      // Append a hash value of the absolute path of the executable to the data location.
      // This allows to start the same application from different build or install trees.
      storageDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/" +
                   this->getOrganizationName() + "/" + this->getApplicationName() + '_';
      storageDir += QString::number(qHash(QCoreApplication::applicationDirPath())) + "/";
    }
    return storageDir;
  }

  void BaseApplication::initializeCppMicroServices()
  {
    QString storageDir = this->getProperty(ctkPluginConstants::FRAMEWORK_STORAGE).toString();

    if (!storageDir.isEmpty())
    {
      us::ModuleSettings::SetStoragePath((storageDir + QString("us") + QDir::separator()).toStdString());
    }
  }

  QCoreApplication *BaseApplication::getQApplication() const
  {
    QCoreApplication *qCoreApp = qApp;

    if (nullptr == qCoreApp)
    {
      vtkOpenGLRenderWindow::SetGlobalMaximumNumberOfMultiSamples(0);

      auto defaultFormat = QVTKOpenGLWidget::defaultFormat();
      defaultFormat.setSamples(0);
      QSurfaceFormat::setDefaultFormat(defaultFormat);

#ifdef Q_OS_OSX
      QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
#endif

      QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

      if (this->getSingleMode())
      {
        qCoreApp = new QmitkSingleApplication(d->m_Argc, d->m_Argv, getSafeMode());
      }
      else
      {
        auto safeApp = new QmitkSafeApplication(d->m_Argc, d->m_Argv);
        safeApp->setSafeMode(d->m_SafeMode);
        qCoreApp = safeApp;
      }
      d->m_QApp.reset(qCoreApp);
    }

    return qCoreApp;
  }

  void BaseApplication::initializeLibraryPaths()
  {
    QStringList suffixes;
    suffixes << "plugins";
#ifdef Q_OS_WINDOWS
    suffixes << "bin/plugins";
#ifdef CMAKE_INTDIR
    suffixes << "bin/" CMAKE_INTDIR "/plugins";
#endif
#else
    suffixes << "lib/plugins";
#ifdef CMAKE_INTDIR
    suffixes << "lib/" CMAKE_INTDIR "/plugins";
#endif
#endif

#ifdef Q_OS_MAC
    suffixes << "../../plugins";
#endif

    // we add a couple of standard library search paths for plug-ins
    QDir appDir(QCoreApplication::applicationDirPath());

    // walk one directory up and add bin and lib sub-dirs; this
    // might be redundant
    appDir.cdUp();

    foreach (QString suffix, suffixes)
    {
      ctkPluginFrameworkLauncher::addSearchPath(appDir.absoluteFilePath(suffix));
    }
  }

  int BaseApplication::main(const std::vector<std::string> &args)
  {
    // Start the plugin framework and all installed plug-ins according with
    // their auto-start setting.
    QStringList arguments;

    for (auto const &arg : args)
    {
      arguments.push_back(QString::fromStdString(arg));
    }

    if (d->m_Splashscreen != nullptr)
    {
      // a splash screen is displayed,
      // creating the closing callback
      d->m_SplashscreenClosingCallback = new SplashCloserCallback(d->m_Splashscreen);
    }

    return ctkPluginFrameworkLauncher::run(d->m_SplashscreenClosingCallback, QVariant::fromValue(arguments)).toInt();
  }

  void BaseApplication::defineOptions(Poco::Util::OptionSet &options)
  {
    Poco::Util::Option helpOption("help", "h", "print this help text");
    helpOption.callback(Poco::Util::OptionCallback<BaseApplication>(this, &BaseApplication::printHelp));
    options.addOption(helpOption);

    Poco::Util::Option newInstanceOption(
      ARG_NEWINSTANCE.toStdString(), "", "forces a new instance of this application");
    newInstanceOption.callback(Poco::Util::OptionCallback<Impl>(d.data(), &Impl::handleBooleanOption));
    options.addOption(newInstanceOption);

    Poco::Util::Option cleanOption(ARG_CLEAN.toStdString(), "", "cleans the plugin cache");
    cleanOption.callback(Poco::Util::OptionCallback<Impl>(d.data(), &Impl::handleClean));
    options.addOption(cleanOption);

    Poco::Util::Option productOption(ARG_PRODUCT.toStdString(), "", "the id of the product to be launched");
    productOption.argument("<id>").binding(PROP_PRODUCT.toStdString());
    options.addOption(productOption);

    Poco::Util::Option appOption(
      ARG_APPLICATION.toStdString(), "", "the id of the application extension to be executed");
    appOption.argument("<id>").binding(PROP_APPLICATION.toStdString());
    options.addOption(appOption);

    Poco::Util::Option provOption(ARG_PROVISIONING.toStdString(), "", "the location of a provisioning file");
    provOption.argument("<prov file>").binding(ARG_PROVISIONING.toStdString());
    options.addOption(provOption);

    Poco::Util::Option storageDirOption(
      ARG_STORAGE_DIR.toStdString(), "", "the location for storing persistent application data");
    storageDirOption.argument("<dir>").binding(ctkPluginConstants::FRAMEWORK_STORAGE.toStdString());
    options.addOption(storageDirOption);

    Poco::Util::Option consoleLogOption(ARG_CONSOLELOG.toStdString(), "", "log messages to the console");
    consoleLogOption.callback(Poco::Util::OptionCallback<Impl>(d.data(), &Impl::handleBooleanOption));
    options.addOption(consoleLogOption);

    Poco::Util::Option debugOption(ARG_DEBUG.toStdString(), "", "enable debug mode");
    debugOption.argument("<options file>", false).binding(ctkPluginFrameworkLauncher::PROP_DEBUG.toStdString());
    options.addOption(debugOption);

    Poco::Util::Option forcePluginOption(
      ARG_FORCE_PLUGIN_INSTALL.toStdString(), "", "force installing plug-ins with same symbolic name");
    forcePluginOption.callback(Poco::Util::OptionCallback<Impl>(d.data(), &Impl::handleBooleanOption));
    options.addOption(forcePluginOption);

    Poco::Util::Option preloadLibsOption(ARG_PRELOAD_LIBRARY.toStdString(), "", "preload a library");
    preloadLibsOption.argument("<library>")
      .repeatable(true)
      .callback(Poco::Util::OptionCallback<Impl>(d.data(), &Impl::handlePreloadLibraryOption));
    options.addOption(preloadLibsOption);

    Poco::Util::Option testPluginOption(ARG_TESTPLUGIN.toStdString(), "", "the plug-in to be tested");
    testPluginOption.argument("<id>").binding(PROP_TESTPLUGIN.toStdString());
    options.addOption(testPluginOption);

    Poco::Util::Option testAppOption(ARG_TESTAPPLICATION.toStdString(), "", "the application to be tested");
    testAppOption.argument("<id>").binding(PROP_TESTAPPLICATION.toStdString());
    options.addOption(testAppOption);

    Poco::Util::Option noRegistryCacheOption(
      ARG_NO_REGISTRY_CACHE.toStdString(), "", "do not use a cache for the registry");
    noRegistryCacheOption.callback(Poco::Util::OptionCallback<Impl>(d.data(), &Impl::handleBooleanOption));
    options.addOption(noRegistryCacheOption);

    Poco::Util::Option noLazyRegistryCacheLoadingOption(
      ARG_NO_LAZY_REGISTRY_CACHE_LOADING.toStdString(), "", "do not use lazy cache loading for the registry");
    noLazyRegistryCacheLoadingOption.callback(Poco::Util::OptionCallback<Impl>(d.data(), &Impl::handleBooleanOption));
    options.addOption(noLazyRegistryCacheLoadingOption);

    Poco::Util::Option registryMultiLanguageOption(
      ARG_REGISTRY_MULTI_LANGUAGE.toStdString(), "", "enable multi-language support for the registry");
    registryMultiLanguageOption.callback(Poco::Util::OptionCallback<Impl>(d.data(), &Impl::handleBooleanOption));
    options.addOption(registryMultiLanguageOption);

  Poco::Util::Option splashScreenOption(ARG_SPLASH_IMAGE.toStdString(), "", "optional picture to use as a splash screen");
  splashScreenOption.argument("<filename>").binding(ARG_SPLASH_IMAGE.toStdString());
  options.addOption(splashScreenOption);

    Poco::Util::Option xargsOption(ARG_XARGS.toStdString(), "", "Extended argument list");
    xargsOption.argument("<args>").binding(ARG_XARGS.toStdString());
    options.addOption(xargsOption);

    Poco::Util::Application::defineOptions(options);
  }

  QSharedPointer<ctkPluginFramework> BaseApplication::getFramework() const
  {
    return ctkPluginFrameworkLauncher::getPluginFramework();
  }

  ctkPluginContext *BaseApplication::getFrameworkContext() const
  {
    QSharedPointer<ctkPluginFramework> framework = getFramework();
    if (framework)
      return framework->getPluginContext();
    return nullptr;
  }

  void BaseApplication::initializeSplashScreen(QCoreApplication * application) const
  {
    QVariant pixmapFileNameProp = d->getProperty(ARG_SPLASH_IMAGE);
    if (!pixmapFileNameProp.isNull()) {
      QString pixmapFileName = pixmapFileNameProp.toString();
      QFileInfo checkFile(pixmapFileName);
      if (checkFile.exists() && checkFile.isFile()) {
        QPixmap pixmap(checkFile.absoluteFilePath());
        d->m_Splashscreen = new QSplashScreen(pixmap, Qt::WindowStaysOnTopHint);
        d->m_Splashscreen->show();
        application->processEvents();
      }
    }
  }

  QHash<QString, QVariant> BaseApplication::getFrameworkProperties() const { return d->m_FWProps; }

  int BaseApplication::run()
  {
    this->init(d->m_Argc, d->m_Argv);
    return Application::run();
  }

  void BaseApplication::setProperty(const QString &property, const QVariant &value) { d->m_FWProps[property] = value; }
  QVariant BaseApplication::getProperty(const QString &property) const { return d->getProperty(property); }

  void BaseApplication::installTranslator(QTranslator* translator)
  {
    this->getQApplication()->installTranslator(translator);
  }

  bool BaseApplication::isRunning()
  {
    auto app = dynamic_cast<QtSingleApplication*>(this->getQApplication());

    if (nullptr != app)
      app->isRunning();

    mitkThrow() << "Method not implemented.";
  }

  void BaseApplication::sendMessage(const QByteArray msg)
  {
    auto app = dynamic_cast<QtSingleApplication*>(this->getQApplication());

    if (nullptr != app)
      app->sendMessage(msg);

    mitkThrow() << "Method not implemented.";
  }

}
