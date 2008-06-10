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

#include "Poco/Path.h"


#include "cherryPlatform.h"
#include "service/cherryIExtensionPointService.h"
#include "internal/cherryInternalPlatform.h"

namespace cherry {

using namespace Poco;

int Platform::OS_FREE_BSD = CHERRY_OS_FREE_BSD;
int Platform::OS_AIX = CHERRY_OS_AIX;
int Platform::OS_HPUX = CHERRY_OS_HPUX;
int Platform::OS_TRU64 = CHERRY_OS_TRU64;
int Platform::OS_LINUX = CHERRY_OS_LINUX;
int Platform::OS_MAC_OS_X = CHERRY_OS_MAC_OS_X;
int Platform::OS_NET_BSD = CHERRY_OS_NET_BSD;
int Platform::OS_OPEN_BSD = CHERRY_OS_OPEN_BSD;
int Platform::OS_IRIX = CHERRY_OS_IRIX;
int Platform::OS_SOLARIS = CHERRY_OS_SOLARIS;
int Platform::OS_QNX = CHERRY_OS_QNX;
int Platform::OS_VXWORKS = CHERRY_OS_VXWORKS;
int Platform::OS_CYGWIN = CHERRY_OS_CYGWIN;
int Platform::OS_UNKNOWN_UNIX = CHERRY_OS_UNKNOWN_UNIX;
int Platform::OS_WINDOWS_NT = CHERRY_OS_WINDOWS_NT;
int Platform::OS_WINDOWS_CE = CHERRY_OS_WINDOWS_CE;
int Platform::OS_VMS = CHERRY_OS_VMS;

int Platform::ARCH_ALPHA = CHERRY_ARCH_ALPHA;
int Platform::ARCH_IA32 = CHERRY_ARCH_IA32;
int Platform::ARCH_IA64 = CHERRY_ARCH_IA64;
int Platform::ARCH_MIPS = CHERRY_ARCH_MIPS;
int Platform::ARCH_HPPA = CHERRY_ARCH_HPPA;
int Platform::ARCH_PPC = CHERRY_ARCH_PPC;
int Platform::ARCH_POWER = CHERRY_ARCH_POWER;
int Platform::ARCH_SPARC = CHERRY_ARCH_SPARC;
int Platform::ARCH_AMD64 = CHERRY_ARCH_AMD64;
int Platform::ARCH_ARM = CHERRY_ARCH_ARM;

std::string Platform::ARG_CLEAN = "openCherry.clean";
std::string Platform::ARG_APPLICATION = "openCherry.application";
std::string Platform::ARG_HOME = "openCherry.home";
std::string Platform::ARG_PLUGIN_CACHE = "openCherry.plugin_cache_dir";
std::string Platform::ARG_PLUGIN_DIRS = "openCherry.plugin_dirs";

const Path* Platform::GetConfigurationPath()
{
  return InternalPlatform::GetInstance()->GetConfigurationPath();
}

IExtensionPointService* Platform::GetExtensionPointService()
{
  return InternalPlatform::GetInstance()->GetExtensionPointService();
}

PlatformEvents& Platform::GetEvents()
{
  return InternalPlatform::GetInstance()->GetEvents();
}

const Path* Platform::GetInstallPath()
{
  return InternalPlatform::GetInstance()->GetInstallPath();
}

const Path* Platform::GetInstancePath()
{
  return InternalPlatform::GetInstance()->GetInstancePath();
}

int Platform::GetOS()
{
  return CHERRY_OS;
}

int Platform::GetOSArch()
{
  return CHERRY_ARCH; 
}

bool Platform::IsUnix()
{
#ifdef CHERRY_OS_FAMILY_UNIX
  return true;
#else
  return false;
#endif
}

bool Platform::IsWindows()
{
#ifdef CHERRY_OS_FAMILY_WINDOWS
  return true;
#else
  return false;
#endif
}

bool Platform::IsBSD()
{
#ifdef CHERRY_OS_FAMILY_BSD
  return true;
#else
  return false;
#endif
}

bool Platform::IsLinux()
{
#ifdef CHERRY_OS_FAMILY_LINUX
  return true;
#else
  return false;
#endif
}

bool Platform::IsVMS()
{
#ifdef CHERRY_OS_FAMILY_VMS
  return true;
#else
  return false;
#endif
}

const Path* Platform::GetStatePath(IBundle* bundle)
{
  return InternalPlatform::GetInstance()->GetStatePath(bundle);
}

const Path* Platform::GetUserPath()
{
  return InternalPlatform::GetInstance()->GetUserPath();
}

std::string GetProperty(const std::string& /*key*/)
{
  return "";
}

bool Platform::IsRunning()
{
  return InternalPlatform::GetInstance()->IsRunning();
}

void Platform::GetRawApplicationArgs(int& argc, char**& argv)
{
  InternalPlatform::GetInstance()->GetRawArguments(argc, argv);
}

bool Platform::HasArgument(const std::string& arg)
{
  return InternalPlatform::GetInstance()->HasArgument(arg);
}

const std::string& Platform::GetArgValue(const std::string& arg)
{
  return InternalPlatform::GetInstance()->GetArgValue(arg);
}

ServiceRegistry& Platform::GetServiceRegistry()
{
  return InternalPlatform::GetInstance()->GetServiceRegistry();
}

IBundle::Pointer Platform::GetBundle(const std::string& id)
{
  return InternalPlatform::GetInstance()->GetBundle(id);
}

}
