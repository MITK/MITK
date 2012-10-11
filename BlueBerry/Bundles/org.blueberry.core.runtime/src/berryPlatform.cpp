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

#include <Poco/Path.h>


#include "berryPlatform.h"
#include "internal/berryInternalPlatform.h"

namespace berry {

int Platform::OS_FREE_BSD = BERRY_OS_FREE_BSD;
int Platform::OS_AIX = BERRY_OS_AIX;
int Platform::OS_HPUX = BERRY_OS_HPUX;
int Platform::OS_TRU64 = BERRY_OS_TRU64;
int Platform::OS_LINUX = BERRY_OS_LINUX;
int Platform::OS_MAC_OS_X = BERRY_OS_MAC_OS_X;
int Platform::OS_NET_BSD = BERRY_OS_NET_BSD;
int Platform::OS_OPEN_BSD = BERRY_OS_OPEN_BSD;
int Platform::OS_IRIX = BERRY_OS_IRIX;
int Platform::OS_SOLARIS = BERRY_OS_SOLARIS;
int Platform::OS_QNX = BERRY_OS_QNX;
int Platform::OS_VXWORKS = BERRY_OS_VXWORKS;
int Platform::OS_CYGWIN = BERRY_OS_CYGWIN;
int Platform::OS_UNKNOWN_UNIX = BERRY_OS_UNKNOWN_UNIX;
int Platform::OS_WINDOWS_NT = BERRY_OS_WINDOWS_NT;
int Platform::OS_WINDOWS_CE = BERRY_OS_WINDOWS_CE;
int Platform::OS_VMS = BERRY_OS_VMS;

int Platform::ARCH_ALPHA = BERRY_ARCH_ALPHA;
int Platform::ARCH_IA32 = BERRY_ARCH_IA32;
int Platform::ARCH_IA64 = BERRY_ARCH_IA64;
int Platform::ARCH_MIPS = BERRY_ARCH_MIPS;
int Platform::ARCH_HPPA = BERRY_ARCH_HPPA;
int Platform::ARCH_PPC = BERRY_ARCH_PPC;
int Platform::ARCH_POWER = BERRY_ARCH_POWER;
int Platform::ARCH_SPARC = BERRY_ARCH_SPARC;
int Platform::ARCH_AMD64 = BERRY_ARCH_AMD64;
int Platform::ARCH_ARM = BERRY_ARCH_ARM;

QString Platform::PROP_QTPLUGIN_PATH = "BlueBerry.qtplugin_path";

QString Platform::ARG_NEWINSTANCE = "BlueBerry.newInstance";
QString Platform::ARG_CLEAN = "BlueBerry.clean";
QString Platform::ARG_APPLICATION = "BlueBerry.application";
QString Platform::ARG_HOME = "BlueBerry.home";
QString Platform::ARG_STORAGE_DIR = "BlueBerry.storageDir";
QString Platform::ARG_PLUGIN_CACHE = "BlueBerry.plugin_cache_dir";
QString Platform::ARG_PLUGIN_DIRS = "BlueBerry.plugin_dirs";
QString Platform::ARG_FORCE_PLUGIN_INSTALL = "BlueBerry.forcePlugins";
QString Platform::ARG_PRELOAD_LIBRARY = "BlueBerry.preloadLibrary";
QString Platform::ARG_PROVISIONING = "BlueBerry.provisioning";
QString Platform::ARG_CONSOLELOG = "BlueBerry.consoleLog";
QString Platform::ARG_TESTPLUGIN = "BlueBerry.testplugin";
QString Platform::ARG_TESTAPPLICATION = "BlueBerry.testapplication";

QString Platform::ARG_NO_REGISTRY_CACHE = "BlueBerry.noRegistryCache";
QString Platform::ARG_NO_LAZY_REGISTRY_CACHE_LOADING = "BlueBerry.noLazyRegistryCacheLoading";
QString Platform::ARG_REGISTRY_MULTI_LANGUAGE = "BlueBerry.registryMultiLanguage";

QString Platform::ARG_XARGS = "xargs";

QDir Platform::GetConfigurationPath()
{
  return InternalPlatform::GetInstance()->GetConfigurationPath();
}

IAdapterManager* Platform::GetAdapterManager()
{
  return InternalPlatform::GetInstance()->GetAdapterManager();
}

IExtensionRegistry *Platform::GetExtensionRegistry()
{
  return InternalPlatform::GetInstance()->GetExtensionRegistry();
}

IPreferencesService *Platform::GetPreferencesService()
{
  return InternalPlatform::GetInstance()->GetPreferencesService();
}

//PlatformEvents& Platform::GetEvents()
//{
//  return InternalPlatform::GetInstance()->GetEvents();
//}

QDir Platform::GetInstallPath()
{
  return InternalPlatform::GetInstance()->GetInstallPath();
}

QDir Platform::GetInstancePath()
{
  return InternalPlatform::GetInstance()->GetInstancePath();
}

int Platform::GetOS()
{
  return BERRY_OS;
}

int Platform::GetOSArch()
{
  return BERRY_ARCH;
}

bool Platform::IsUnix()
{
#ifdef BERRY_OS_FAMILY_UNIX
  return true;
#else
  return false;
#endif
}

bool Platform::IsWindows()
{
#ifdef BERRY_OS_FAMILY_WINDOWS
  return true;
#else
  return false;
#endif
}

bool Platform::IsBSD()
{
#ifdef BERRY_OS_FAMILY_BSD
  return true;
#else
  return false;
#endif
}

bool Platform::IsLinux()
{
#ifdef BERRY_OS_FAMILY_LINUX
  return true;
#else
  return false;
#endif
}

bool Platform::IsVMS()
{
#ifdef BERRY_OS_FAMILY_VMS
  return true;
#else
  return false;
#endif
}

bool Platform::GetStatePath(QDir& statePath, const QSharedPointer<ctkPlugin>& plugin, bool create)
{
  return InternalPlatform::GetInstance()->GetStatePath(statePath, plugin, create);
}

QDir Platform::GetUserPath()
{
  return InternalPlatform::GetInstance()->GetUserPath();
}

QString Platform::GetProperty(const QString& key)
{
  return QString::fromStdString(GetConfiguration().getString(key.toStdString(), ""));
}

bool Platform::IsRunning()
{
  return InternalPlatform::GetInstance()->IsRunning();
}

int& Platform::GetRawApplicationArgs(char**& argv)
{
  return InternalPlatform::GetInstance()->GetRawApplicationArgs(argv);
}

QStringList Platform::GetApplicationArgs()
{
  return InternalPlatform::GetInstance()->GetApplicationArgs();
}

QString Platform::GetExtendedApplicationArgs()
{
  return QString::fromStdString(InternalPlatform::GetInstance()->GetConfiguration().getString(ARG_XARGS.toStdString(), ""));
}

void Platform::GetOptionSet(Poco::Util::OptionSet& os)
{
  InternalPlatform::GetInstance()->defineOptions(os);
}

Poco::Util::LayeredConfiguration& Platform::GetConfiguration()
{
  return InternalPlatform::GetInstance()->GetConfiguration();
}

QSharedPointer<ctkPlugin> Platform::GetCTKPlugin(const QString& symbolicName)
{
  QList<QSharedPointer<ctkPlugin> > plugins =
      InternalPlatform::GetInstance()->GetCTKPluginFrameworkContext()->getPlugins();

  foreach(QSharedPointer<ctkPlugin> plugin, plugins)
  {
    if (plugin->getSymbolicName() == symbolicName)
      return plugin;
  }
  return QSharedPointer<ctkPlugin>(0);
}

QSharedPointer<ctkPlugin> Platform::GetCTKPlugin(long id)
{
  return InternalPlatform::GetInstance()->GetCTKPluginFrameworkContext()->getPlugin(id);
}

QSharedPointer<ctkPlugin> Platform::GetPlugin(const QString& symbolicName)
{
  return InternalPlatform::GetInstance()->GetPlugin(symbolicName);
}

QList<QSharedPointer<ctkPlugin> > Platform::GetPlugins(const QString& symbolicName,
                                                       const QString& version)
{
  return InternalPlatform::GetInstance()->GetPlugins(symbolicName, version);
}

}
