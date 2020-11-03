/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryPlatform.h"

#include <berryIProduct.h>

#include "internal/berryInternalPlatform.h"
#include "internal/berryIRuntimeConstants.h"

#include <service/datalocation/ctkLocation.h>

namespace berry {

const QString Platform::PI_RUNTIME = IRuntimeConstants::PI_RUNTIME();

const int Platform::OS_FREE_BSD = BERRY_OS_FREE_BSD;
const int Platform::OS_AIX = BERRY_OS_AIX;
const int Platform::OS_HPUX = BERRY_OS_HPUX;
const int Platform::OS_TRU64 = BERRY_OS_TRU64;
const int Platform::OS_LINUX = BERRY_OS_LINUX;
const int Platform::OS_MAC_OS_X = BERRY_OS_MAC_OS_X;
const int Platform::OS_NET_BSD = BERRY_OS_NET_BSD;
const int Platform::OS_OPEN_BSD = BERRY_OS_OPEN_BSD;
const int Platform::OS_IRIX = BERRY_OS_IRIX;
const int Platform::OS_SOLARIS = BERRY_OS_SOLARIS;
const int Platform::OS_QNX = BERRY_OS_QNX;
const int Platform::OS_VXWORKS = BERRY_OS_VXWORKS;
const int Platform::OS_CYGWIN = BERRY_OS_CYGWIN;
const int Platform::OS_UNKNOWN_UNIX = BERRY_OS_UNKNOWN_UNIX;
const int Platform::OS_WINDOWS_NT = BERRY_OS_WINDOWS_NT;
const int Platform::OS_WINDOWS_CE = BERRY_OS_WINDOWS_CE;
const int Platform::OS_VMS = BERRY_OS_VMS;

const int Platform::ARCH_ALPHA = BERRY_ARCH_ALPHA;
const int Platform::ARCH_IA32 = BERRY_ARCH_IA32;
const int Platform::ARCH_IA64 = BERRY_ARCH_IA64;
const int Platform::ARCH_MIPS = BERRY_ARCH_MIPS;
const int Platform::ARCH_HPPA = BERRY_ARCH_HPPA;
const int Platform::ARCH_PPC = BERRY_ARCH_PPC;
const int Platform::ARCH_POWER = BERRY_ARCH_POWER;
const int Platform::ARCH_SPARC = BERRY_ARCH_SPARC;
const int Platform::ARCH_AMD64 = BERRY_ARCH_AMD64;
const int Platform::ARCH_ARM = BERRY_ARCH_ARM;

const QString Platform::PROP_QTPLUGIN_PATH = "BlueBerry.qtplugin_path";

const QString Platform::PROP_NEWINSTANCE = "BlueBerry.newInstance";
const QString Platform::PROP_APPLICATION = "BlueBerry.application";
const QString Platform::PROP_PLUGIN_DIRS = "BlueBerry.plugin_dirs";
const QString Platform::PROP_FORCE_PLUGIN_INSTALL = "BlueBerry.forcePlugins";

const QString Platform::PROP_IGNOREAPP = "BlueBerry.ignoreApp";

const QString Platform::PROP_XARGS = "xargs";


QDir Platform::GetConfigurationPath()
{
  ctkLocation* location = GetConfigurationLocation();
  if (location)
  {
    return location->getUrl().toLocalFile();
  }
  return QDir();
}

ctkLocation* Platform::GetConfigurationLocation()
{
  return InternalPlatform::GetInstance()->GetConfigurationLocation();
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
  ctkLocation* location = GetInstallLocation();
  if (location)
  {
    return location->getUrl().toLocalFile();
  }
  return QDir();
}

ctkLocation* Platform::GetInstallLocation()
{
  return InternalPlatform::GetInstance()->GetInstallLocation();
}

QDir Platform::GetInstancePath()
{
  ctkLocation* location = GetInstanceLocation();
  if (location)
  {
    return location->getUrl().toLocalFile();
  }
  return QDir();

}

ctkLocation* Platform::GetInstanceLocation()
{
  return InternalPlatform::GetInstance()->GetInstanceLocation();
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

bool Platform::GetStatePath(QDir& statePath, const QSharedPointer<ctkPlugin>& plugin, bool /*create*/)
{
  statePath = GetStateLocation(plugin);
  return true;
}

QDir Platform::GetStateLocation(const QSharedPointer<ctkPlugin>& plugin)
{
  return InternalPlatform::GetInstance()->GetStateLocation(plugin);
}

QDir Platform::GetUserPath()
{
  ctkLocation* location = GetUserLocation();
  if (location)
  {
    return location->getUrl().toLocalFile();
  }
  return QDir();

}

ctkLocation* Platform::GetUserLocation()
{
  return InternalPlatform::GetInstance()->GetUserLocation();
}

bool Platform::IsRunning()
{
  return InternalPlatform::GetInstance()->IsRunning();
}

QStringList Platform::GetApplicationArgs()
{
  return InternalPlatform::GetInstance()->GetApplicationArgs();
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

QVariant berry::Platform::GetDebugOption(const QString& option)
{
  return InternalPlatform::GetInstance()->GetOption(option);
}

IProduct::Pointer berry::Platform::GetProduct()
{
  return InternalPlatform::GetInstance()->GetProduct();
}

}
