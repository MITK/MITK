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

#include "cherryInternalPlatform.h"

#include "Poco/Exception.h"
#include "Poco/File.h"

#include <iostream>

#include "../cherryPlatform.h"
#include "../cherryPlatformException.h"
#include "../event/cherryPlatformEvents.h"
#include "cherryPlatformLogChannel.h"
#include "../cherryIBundle.h"
#include "cherryCodeCache.h"
#include "../cherryBundleLoader.h"

namespace cherry {

Poco::Mutex InternalPlatform::m_Mutex;

InternalPlatform::InternalPlatform() : m_Initialized(false), m_Running(false),
  m_StatePath(0), m_InstallPath(0), m_InstancePath(0), m_UserPath(0),
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

void InternalPlatform::GetRawArguments(int& argc, char**& argv)
{
  argc = m_Argc;
  argv = m_Argv;
}

bool InternalPlatform::HasArgument(const std::string& arg) const
{
  return m_ArgMap.find(arg) != m_ArgMap.end();
}
  
const std::string& 
InternalPlatform::GetArgValue(const std::string& arg)
{
  return m_ArgMap[arg];
}

ServiceRegistry& InternalPlatform::GetServiceRegistry()
{
  AssertInitialized();
  return m_ServiceRegistry;
}

void InternalPlatform::ParseArguments()
{
  std::string arg;
  for (int i = 1; i < m_Argc; ++i)
  {
    arg = m_Argv[i];
    if (arg == Platform::ARG_CLEAN)
      m_ArgMap[Platform::ARG_CLEAN] = "";
    
    if (arg == Platform::ARG_APPLICATION)
      m_ArgMap[Platform::ARG_APPLICATION] = m_Argv[++i];
  }
}

void InternalPlatform::Initialize(int& argc, char** argv)
{
  // initialization
  Poco::Mutex::ScopedLock lock(m_Mutex);
  
  m_Argc = argc;
  m_Argv = argv;
  
  this->ParseArguments();
  
  m_PlatformLogChannel = new PlatformLogChannel("/tmp/.cherryLog");
  m_PlatformLogger = &Poco::Logger::create("", m_PlatformLogChannel, Poco::Message::PRIO_TRACE);
  
  Poco::Path cachePath(*(this->GetInstallPath()));
  cachePath.pushDirectory("bin");
  cachePath.pushDirectory("Plugins");
  m_CodeCache = new CodeCache(cachePath.toString());
  m_BundleLoader = new BundleLoader(m_CodeCache, *m_PlatformLogger);
  
 
  m_Initialized = true;
  
  // Clear the CodeCache
  if (this->HasArgument(Platform::ARG_CLEAN))
    m_CodeCache->Clear();
  
  // go through the plugins
  Poco::Path pluginPath(*(this->GetInstallPath()));
  pluginPath.pushDirectory("Utilities/openCherry");
  
  std::cout << "Plugin dir. is: " << pluginPath.toString() << std::endl;
  Poco::File pluginDir(pluginPath);
  
  std::vector<std::string> pluginList;
  pluginDir.list(pluginList);
  
  std::vector<Poco::Path> pluginPaths;
  std::vector<std::string>::iterator iter;
  for (iter = pluginList.begin(); iter != pluginList.end(); iter++)
  {
    pluginPath.pushDirectory(*iter);
    
    Poco::File file(pluginPath);
    if (file.exists() && file.isDirectory())
    {
      pluginPaths.push_back(pluginPath);
    }
    
    pluginPath.popDirectory();
  }
  
  std::vector<Poco::Path>::iterator pathIter;
  for (pathIter = pluginPaths.begin(); pathIter != pluginPaths.end(); pathIter++)
  {
    try
    {
    Bundle::Pointer bundle = m_BundleLoader->LoadBundle(*pathIter);
    std::cout << "Bundle state (" << pathIter->toString() << "): " << bundle->GetStateString() << std::endl;
    }
    catch (BundleStateException exc)
    {
      std::cout << exc.displayText() << std::endl;
    }
  }
  
  // resolve plugins
  m_BundleLoader->ResolveAllBundles();
  
}

void InternalPlatform::Launch()
{
  AssertInitialized();
  
  if (m_Running) return;
  
  m_Running = true;
  
  SystemBundle* systemBundle = m_BundleLoader->FindBundle("system.bundle").Cast<SystemBundle>();
  if (systemBundle == 0)
    throw PlatformException("Could not find the system bundle");
  m_BundleLoader->StartSystemBundle(systemBundle);
  
  systemBundle->Resume();
}

void InternalPlatform::Shutdown()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  
  AssertInitialized();
  
  m_Initialized = false;
  
  if (m_StatePath != 0) delete m_StatePath;
  if (m_InstallPath != 0) delete m_InstallPath;
  if (m_InstancePath != 0) delete m_InstancePath;
  if (m_UserPath != 0) delete m_UserPath;
  
  if (m_BundleLoader != 0) delete m_BundleLoader;
  if (m_CodeCache != 0) delete m_CodeCache;
  
}


void InternalPlatform::AssertInitialized()
{
  if (!m_Initialized)
    throw SystemException("The Platform has not been initialized yet!");
}

IExtensionPointService::Pointer InternalPlatform::GetExtensionPointService()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  this->AssertInitialized();
  
  return m_ServiceRegistry.GetServiceById<IExtensionPointService>(IExtensionPointService::SERVICE_ID);
}

const Path* InternalPlatform::GetConfigurationPath()
{
  return 0;
}

const Path* InternalPlatform::GetInstallPath()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  if (m_InstallPath == 0)
  {
    m_InstallPath = new Path(true);
    m_InstallPath->assign(Path("../").absolute());
  }
  
  return m_InstallPath;
}

const Path* InternalPlatform::GetInstancePath()
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  if (m_InstancePath == 0)
    {
      m_InstancePath = new Path(true);
      m_InstancePath->assign(Path().absolute());
    }
    
  return m_InstancePath;
}

const Path* InternalPlatform::GetStatePath(IBundle* /*bundle*/)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  if (m_StatePath == 0)
  {
    m_StatePath = new Path(true);
    m_StatePath->assign(Path::temp());
  }

  return m_StatePath;
}

PlatformEvents& InternalPlatform::GetEvents()
{
  return m_Events;
}

const Path* InternalPlatform::GetUserPath()
{
  return 0;
}

bool InternalPlatform::IsRunning() const
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  return (m_Initialized && m_Running);
}

IBundle::Pointer InternalPlatform::GetBundle(const std::string& id)
{
  Poco::Mutex::ScopedLock lock(m_Mutex);
  
  AssertInitialized();
  
  return m_BundleLoader->FindBundle(id);
}

}
