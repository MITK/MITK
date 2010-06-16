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

#include "berryInternalPlatform.h"
#include "berryLog.h"

#include <Poco/Exception.h>
#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/AutoPtr.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Util/HelpFormatter.h>

#include <iostream>

#include "../berryPlatform.h"
#include "osgi/framework/Exceptions.h"
#include "../berryDebugUtil.h"
#include "../event/berryPlatformEvents.h"
#include "berryPlatformLogChannel.h"
#include "osgi/framework/IBundle.h"
#include "berryCodeCache.h"
#include "../berryBundleLoader.h"
#include "berrySystemBundle.h"

namespace berry {

Poco::Mutex InternalPlatform::m_Mutex;

InternalPlatform::InternalPlatform() : m_Initialized(false), m_Running(false),
  m_ConsoleLog(false), m_ServiceRegistry(0),
  m_CodeCache(0), m_BundleLoader(0), m_SystemBundle(0), m_PlatformLogger(0),
  m_EventStarted(PlatformEvent::EV_PLATFORM_STARTED)
{
}

InternalPlatform::~InternalPlatform()
{

}

InternalPlatform* InternalPlatform::GetInstance()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  static InternalPlatform instance;
  return &instance;
}

bool InternalPlatform::ConsoleLog() const
{
  return m_ConsoleLog;
}

ServiceRegistry& InternalPlatform::GetServiceRegistry()
{
  AssertInitialized();
  return *m_ServiceRegistry;
}

void InternalPlatform::Initialize(int& argc, char** argv, Poco::Util::AbstractConfiguration* config)
{
  // initialization
  Poco::Mutex::ScopedLock lock(m_Mutex);

  m_Argc = &argc;
  m_Argv = argv;

  this->init(argc, argv);
  this->loadConfiguration();
  if (config)
  {
    this->config().add(config, 50, false);
  }

  m_ServiceRegistry = new ServiceRegistry();

  m_ConsoleLog = this->GetConfiguration().hasProperty(Platform::ARG_CONSOLELOG);

  m_ConfigPath.assign(this->GetConfiguration().getString("application.configDir"));
  m_InstancePath.assign(this->GetConfiguration().getString("application.dir"));
  try
  {
    m_InstallPath.assign(this->GetConfiguration().getString(Platform::ARG_HOME));
  }
  catch (Poco::NotFoundException& )
  {
    m_InstallPath.assign(m_InstancePath);
  }

  m_UserPath.assign(Poco::Path::home());
  m_UserPath.pushDirectory("." + this->commandName());
  Poco::File userFile(m_UserPath); 
  
  try
  {
    userFile.createDirectory();
    userFile.canWrite();
  }
  catch(const Poco::IOException& e)
  {
    BERRY_WARN << e.displayText();
    m_UserPath.assign(Poco::Path::temp());
    m_UserPath.pushDirectory("." + this->commandName());
    userFile = m_UserPath;
  }
   
  m_BaseStatePath = m_UserPath;
  m_BaseStatePath.pushDirectory(".metadata");
  m_BaseStatePath.pushDirectory(".plugins");

  Poco::Path logPath(m_UserPath);
  logPath.setFileName(this->commandName() + ".log");
  m_PlatformLogChannel = new PlatformLogChannel(logPath.toString());
  m_PlatformLogger = &Poco::Logger::create("PlatformLogger", m_PlatformLogChannel, Poco::Message::PRIO_TRACE);

  try
  {
    m_CodeCache = new CodeCache(this->GetConfiguration().getString(Platform::ARG_PLUGIN_CACHE));
  }
  catch (Poco::NotFoundException&)
  {
    Poco::Path cachePath(m_UserPath);
    cachePath.pushDirectory("plugin_cache");
    m_CodeCache = new CodeCache(cachePath.toString());
  }
  m_BundleLoader = new BundleLoader(m_CodeCache, *m_PlatformLogger);


  m_Initialized = true;

  // Clear the CodeCache
  if (this->GetConfiguration().hasProperty(Platform::ARG_CLEAN))
    m_CodeCache->Clear();

  try
  {
    // assemble a list of base plugin-directories (which contain
    // the real plugins as directories)
    std::vector<std::string> pluginBaseDirs;

    Poco::StringTokenizer tokenizer(this->GetConfiguration().getString(Platform::ARG_PLUGIN_DIRS), ";",
                                    Poco::StringTokenizer::TOK_IGNORE_EMPTY | Poco::StringTokenizer::TOK_TRIM);

    for (Poco::StringTokenizer::Iterator token = tokenizer.begin();
         token != tokenizer.end(); ++token)
    {
      pluginBaseDirs.push_back(*token);
    }

    std::vector<Poco::Path> pluginPaths;
    for (std::vector<std::string>::iterator pluginBaseDir = pluginBaseDirs.begin();
         pluginBaseDir != pluginBaseDirs.end(); ++pluginBaseDir)
    {
      BERRY_INFO(m_ConsoleLog) << "Plugin base directory: " << *pluginBaseDir;
      Poco::File pluginDir(*pluginBaseDir);

      if (!pluginDir.exists() || !pluginDir.isDirectory())
      {
        BERRY_WARN(m_ConsoleLog) << *pluginBaseDir << " is not a direcotry or does not exist. SKIPPED.\n";
        continue;
      }
    
      std::vector<std::string> pluginList;
      pluginDir.list(pluginList);

      std::vector<std::string>::iterator iter;
      for (iter = pluginList.begin(); iter != pluginList.end(); iter++)
      {
        Poco::Path pluginPath = Poco::Path::forDirectory(*pluginBaseDir);
        pluginPath.pushDirectory(*iter);

        Poco::File file(pluginPath);
        if (file.exists() && file.isDirectory())
        {
          pluginPaths.push_back(pluginPath);
        }
      }
    }

    std::vector<Poco::Path>::iterator pathIter;
    for (pathIter = pluginPaths.begin(); pathIter != pluginPaths.end(); pathIter++)
    {
      try
      {
      Bundle::Pointer bundle = m_BundleLoader->LoadBundle(*pathIter);
      BERRY_INFO(m_ConsoleLog) << "Bundle state (" << pathIter->toString() << "): " << bundle->GetStateString() << std::endl;
      }
      catch (const BundleStateException& exc)
      {
        BERRY_WARN << exc.displayText() << std::endl;
      }
    }

    // resolve plugins
    m_BundleLoader->ResolveAllBundles();
  }
  catch (Poco::Exception& exc)
  {
    this->logger().log(exc);
  }

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
  DebugUtil::RestoreState();
#endif

}

void InternalPlatform::Launch()
{
  AssertInitialized();

  if (m_Running) return;

  m_Running = true;

  this->run();
}

void InternalPlatform::Shutdown()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);

  AssertInitialized();

  DebugUtil::SaveState();

  m_Initialized = false;

  this->uninitialize();

  delete m_ServiceRegistry;
  delete m_BundleLoader;
  delete m_CodeCache;
}


void InternalPlatform::AssertInitialized()
{
  if (!m_Initialized)
    throw Poco::SystemException("The Platform has not been initialized yet!");
}

IExtensionPointService::Pointer InternalPlatform::GetExtensionPointService()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  this->AssertInitialized();

  return m_ServiceRegistry->GetServiceById<IExtensionPointService>(IExtensionPointService::SERVICE_ID);
}

const Poco::Path& InternalPlatform::GetConfigurationPath()
{
  return m_ConfigPath;
}

const Poco::Path& InternalPlatform::GetInstallPath()
{
  return m_InstallPath;
}

const Poco::Path& InternalPlatform::GetInstancePath()
{
  return m_InstancePath;
}

bool InternalPlatform::GetStatePath(Poco::Path& statePath, osgi::framework::IBundle::Pointer bundle, bool create)
{
  statePath = m_BaseStatePath;
  statePath.pushDirectory(bundle->GetSymbolicName());
  try
  {
  Poco::File stateFile(statePath);
  if (!stateFile.exists() && create)
    stateFile.createDirectories();
  }
  catch (Poco::FileException&)
  {
    return false;
  }

  return true;
}

PlatformEvents& InternalPlatform::GetEvents()
{
  return m_Events;
}

const Poco::Path& InternalPlatform::GetUserPath()
{
  return m_UserPath;
}

bool InternalPlatform::IsRunning() const
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  return (m_Initialized && m_Running);
}

osgi::framework::IBundle::Pointer InternalPlatform::GetBundle(const std::string& id)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);

  AssertInitialized();

  return m_BundleLoader->FindBundle(id);
}

Poco::Logger* InternalPlatform::GetLogger()
{
  return m_PlatformLogger;
}

Poco::Util::LayeredConfiguration& InternalPlatform::GetConfiguration() const
{
  return this->config();
}

std::vector<std::string> InternalPlatform::GetApplicationArgs() const
{
  return m_FilteredArgs;
}

int& InternalPlatform::GetRawApplicationArgs(char**& argv)
{
  argv = m_Argv;
  return *m_Argc;
}

void InternalPlatform::defineOptions(Poco::Util::OptionSet& options)
{
  Poco::Util::Option helpOption("help", "h", "print this help text");
  helpOption.callback(Poco::Util::OptionCallback<InternalPlatform>(this, &InternalPlatform::PrintHelp));
  options.addOption(helpOption);

  Poco::Util::Option cleanOption(Platform::ARG_CLEAN, "", "cleans the plugin cache");
  cleanOption.binding(Platform::ARG_CLEAN);
  options.addOption(cleanOption);

  Poco::Util::Option appOption(Platform::ARG_APPLICATION, "", "the id of the application extension to be executed");
  appOption.argument("<id>").binding(Platform::ARG_APPLICATION);
  options.addOption(appOption);

  Poco::Util::Option consoleLogOption(Platform::ARG_CONSOLELOG, "", "log messages to the console");
  consoleLogOption.binding(Platform::ARG_CONSOLELOG);
  options.addOption(consoleLogOption);

  Poco::Util::Option testPluginOption(Platform::ARG_TESTPLUGIN, "", "the plug-in to be tested");
  testPluginOption.argument("<id>").binding(Platform::ARG_TESTPLUGIN);
  options.addOption(testPluginOption);

  Poco::Util::Option testAppOption(Platform::ARG_TESTAPPLICATION, "", "the application to be tested");
  testAppOption.argument("<id>").binding(Platform::ARG_TESTAPPLICATION);
  options.addOption(testAppOption);

  Poco::Util::Application::defineOptions(options);
}

int InternalPlatform::main(const std::vector<std::string>& args)
{
  m_FilteredArgs = args;
  m_FilteredArgs.insert(m_FilteredArgs.begin(), this->config().getString("application.argv[0]"));

  SystemBundle::Pointer systemBundle = m_BundleLoader->FindBundle("system.bundle").Cast<SystemBundle>();
  if (systemBundle == 0)
    throw PlatformException("Could not find the system bundle");
  m_BundleLoader->StartSystemBundle(systemBundle);

  systemBundle->Resume();

  return EXIT_OK;
}

void InternalPlatform::PrintHelp(const std::string&, const std::string&)
{
  Poco::Util::HelpFormatter help(this->options());
  help.setAutoIndent();
  help.setCommand(this->commandName());
  help.format(std::cout);

  exit(EXIT_OK);
}

}
