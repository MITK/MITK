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

#ifndef CHERRYINTERNALPLATFORM_H_
#define CHERRYINTERNALPLATFORM_H_

#include "Poco/Path.h"
#include "Poco/Mutex.h"
#include "Poco/AutoPtr.h"
#include "Poco/Logger.h"

#include "../event/cherryPlatformEvents.h"
#include "../service/cherryServiceRegistry.h"
#include "cherryExtensionPointService.h"

#include "cherrySystemBundle.h"

#include <map>

namespace cherry {

using namespace Poco;

struct IBundle;
class CodeCache;
class BundleLoader;
class PlatformLogChannel;

class CHERRY_OSGI InternalPlatform
{
private:
  
  static Mutex m_Mutex;
  Path m_InstanceLocation;
  
  bool m_Initialized;
  bool m_Running;
  
  ServiceRegistry m_ServiceRegistry;
  
  Path* m_StatePath;
  Path* m_InstallPath;
  Path* m_InstancePath;
  Path* m_UserPath;
  
  CodeCache* m_CodeCache;
  BundleLoader* m_BundleLoader;
  
  SystemBundle* m_SystemBundle;
  
  Poco::AutoPtr<PlatformLogChannel> m_PlatformLogChannel;
  Poco::Logger* m_PlatformLogger;
  
  PlatformEvents m_Events;
  PlatformEvent m_EventStarted;
  
  int m_Argc;
  char** m_Argv;
  
  std::map<std::string, std::string> m_ArgMap;
  
  InternalPlatform();
  //InternalPlatform(const InternalPlatform&) : m_EventStarted(PlatformEvent::EV_PLATFORM_STARTED) {};
  
  void AssertInitialized();
  
  void ParseArguments();
  void ParseConfigFile();
  
public:
  virtual ~InternalPlatform();
  
  static InternalPlatform* GetInstance();
  
  void Initialize(int& argc, char** argv);
  void Launch();
  void Shutdown();
  
  /// Returns a ServiceRegistry object for registering
  /// and accessing services from different plugins
  ServiceRegistry& GetServiceRegistry();
  
  /// Convenience method to quickly get the extension
  /// point service, which is automatically started
  /// by the platform
  IExtensionPointService::Pointer GetExtensionPointService();
  
  /// Returns the location of the configuration information
  /// used to run this instance of the CHERRY Platform. The 
  /// configuration area typically contains the list of plug-ins
  /// available for use, various settings (those shared across 
  /// different instances of the same configuration) and any 
  /// other such data needed by plug-ins. 
  /// 0 is returned if the platform is running without a 
  /// configuration location.
  const Path* GetConfigurationPath();
  
  const Path* GetInstallPath();
  
  const Path* GetInstancePath();
  
  const Path* GetStatePath(IBundle* bundle);
  
  const Path* GetUserPath();
  
  PlatformEvents& GetEvents();
  
  bool IsRunning() const;
  
  void GetRawArguments(int& argc, char**& argv);
  
  bool HasArgument(const std::string& arg) const;
  const std::string& GetArgValue(const std::string& arg);
  
  IBundle::Pointer GetBundle(const std::string& id);
};

}  // namespace cherry

#endif /*CHERRYINTERNALPLATFORM_H_*/
