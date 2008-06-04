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

#ifndef CHERRY_Platform_INCLUDED
#define CHERRY_Platform_INCLUDED


//
// Platform Identification
//
#define CHERRY_OS_FREE_BSD      0x0001
#define CHERRY_OS_AIX           0x0002
#define CHERRY_OS_HPUX          0x0003
#define CHERRY_OS_TRU64         0x0004
#define CHERRY_OS_LINUX         0x0005
#define CHERRY_OS_MAC_OS_X      0x0006
#define CHERRY_OS_NET_BSD       0x0007
#define CHERRY_OS_OPEN_BSD      0x0008
#define CHERRY_OS_IRIX          0x0009
#define CHERRY_OS_SOLARIS       0x000a
#define CHERRY_OS_QNX           0x000b
#define CHERRY_OS_VXWORKS       0x000c
#define CHERRY_OS_CYGWIN        0x000d
#define CHERRY_OS_UNKNOWN_UNIX  0x00ff
#define CHERRY_OS_WINDOWS_NT    0x1001
#define CHERRY_OS_WINDOWS_CE    0x1011
#define CHERRY_OS_VMS           0x2001


#if defined(__FreeBSD__)
  #define CHERRY_OS_FAMILY_UNIX 1
  #define CHERRY_OS_FAMILY_BSD 1
  #define CHERRY_OS CHERRY_OS_FREE_BSD
#elif defined(_AIX) || defined(__TOS_AIX__)
  #define CHERRY_OS_FAMILY_UNIX 1
  #define CHERRY_OS CHERRY_OS_AIX
#elif defined(hpux) || defined(_hpux)
  #define CHERRY_OS_FAMILY_UNIX 1
  #define CHERRY_OS CHERRY_OS_HPUX
#elif defined(__digital__) || defined(__osf__)
  #define CHERRY_OS_FAMILY_UNIX 1
  #define CHERRY_OS CHERRY_OS_TRU64
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__TOS_LINUX__)
  #define CHERRY_OS_FAMILY_UNIX 1
  #define CHERRY_OS CHERRY_OS_LINUX
#elif defined(__APPLE__) || defined(__TOS_MACOS__)
  #define CHERRY_OS_FAMILY_UNIX 1
  #define CHERRY_OS_FAMILY_BSD 1
  #define CHERRY_OS CHERRY_OS_MAC_OS_X
#elif defined(__NetBSD__)
  #define CHERRY_OS_FAMILY_UNIX 1
  #define CHERRY_OS_FAMILY_BSD 1
  #define CHERRY_OS CHERRY_OS_NET_BSD
#elif defined(__OpenBSD__)
  #define CHERRY_OS_FAMILY_UNIX 1
  #define CHERRY_OS_FAMILY_BSD 1
  #define CHERRY_OS CHERRY_OS_OPEN_BSD
#elif defined(sgi) || defined(__sgi)
  #define CHERRY_OS_FAMILY_UNIX 1
  #define CHERRY_OS CHERRY_OS_IRIX
#elif defined(sun) || defined(__sun)
  #define CHERRY_OS_FAMILY_UNIX 1
  #define CHERRY_OS CHERRY_OS_SOLARIS
#elif defined(__QNX__)
  #define CHERRY_OS_FAMILY_UNIX 1
  #define CHERRY_OS CHERRY_OS_QNX
#elif defined(unix) || defined(__unix) || defined(__unix__)
  #define CHERRY_OS_FAMILY_UNIX 1
  #define CHERRY_OS CHERRY_OS_UNKNOWN_UNIX
#elif defined(_WIN32_WCE)
  #define CHERRY_OS_FAMILY_WINDOWS 1
  #define CHERRY_OS CHERRY_OS_WINDOWS_CE
#elif defined(_WIN32) || defined(_WIN64)
  #define CHERRY_OS_FAMILY_WINDOWS 1
  #define CHERRY_OS CHERRY_OS_WINDOWS_NT
#elif defined(__CYGWIN__)
  #define CHERRY_OS_FAMILY_UNIX 1
  #define CHERRY_OS CHERRY_OS_CYGWIN
#elif defined(__VMS)
  #define CHERRY_OS_FAMILY_VMS 1
  #define CHERRY_OS CHERRY_OS_VMS
#endif


//
// Hardware Architecture and Byte Order
//
#define CHERRY_ARCH_ALPHA   0x01
#define CHERRY_ARCH_IA32    0x02
#define CHERRY_ARCH_IA64    0x03
#define CHERRY_ARCH_MIPS    0x04
#define CHERRY_ARCH_HPPA    0x05
#define CHERRY_ARCH_PPC     0x06
#define CHERRY_ARCH_POWER   0x07
#define CHERRY_ARCH_SPARC   0x08
#define CHERRY_ARCH_AMD64   0x09
#define CHERRY_ARCH_ARM     0x0a


#if defined(__ALPHA) || defined(__alpha) || defined(__alpha__) || defined(_M_ALPHA)
  #define CHERRY_ARCH CHERRY_ARCH_ALPHA
  #define CHERRY_ARCH_LITTLE_ENDIAN 1
#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(_M_IX86)
  #define CHERRY_ARCH CHERRY_ARCH_IA32
  #define CHERRY_ARCH_LITTLE_ENDIAN 1
#elif defined(_IA64) || defined(__IA64__) || defined(__ia64__) || defined(__ia64) || defined(_M_IA64)
  #define CHERRY_ARCH CHERRY_ARCH_IA64
  #if defined(hpux) || defined(_hpux)
    #define CHERRY_ARCH_BIG_ENDIAN 1
  #else
    #define CHERRY_ARCH_LITTLE_ENDIAN 1
  #endif
#elif defined(__x86_64__)
  #define CHERRY_ARCH CHERRY_ARCH_AMD64
  #define CHERRY_ARCH_LITTLE_ENDIAN 1
#elif defined(__mips__) || defined(__mips) || defined(__MIPS__) || defined(_M_MRX000)
  #define CHERRY_ARCH CHERRY_ARCH_MIPS
  #define CHERRY_ARCH_BIG_ENDIAN 1
#elif defined(__hppa) || defined(__hppa__)
  #define CHERRY_ARCH CHERRY_ARCH_HPPA
  #define CHERRY_ARCH_BIG_ENDIAN 1
#elif defined(__PPC) || defined(__POWERPC__) || defined(__powerpc) || defined(__PPC__) || \
      defined(__powerpc__) || defined(__ppc__) || defined(_ARCH_PPC) || defined(_M_PPC)
  #define CHERRY_ARCH CHERRY_ARCH_PPC
  #define CHERRY_ARCH_BIG_ENDIAN 1
#elif defined(_POWER) || defined(_ARCH_PWR) || defined(_ARCH_PWR2) || defined(_ARCH_PWR3) || \
      defined(_ARCH_PWR4) || defined(__THW_RS6000)
  #define CHERRY_ARCH CHERRY_ARCH_POWER
  #define CHERRY_ARCH_BIG_ENDIAN 1
#elif defined(__sparc__) || defined(__sparc) || defined(sparc)
  #define CHERRY_ARCH CHERRY_ARCH_SPARC
  #define CHERRY_ARCH_BIG_ENDIAN 1
#elif defined(__arm__) || defined(__arm) || defined(ARM) || defined(_ARM_) || defined(__ARM__) || defined(_M_ARM)
  #define CHERRY_ARCH CHERRY_ARCH_ARM
  #if defined(__ARMEB__)
    #define CHERRY_ARCH_BIG_ENDIAN 1
  #else
    #define CHERRY_ARCH_LITTLE_ENDIAN 1
  #endif
#endif


#include "cherryOSGiDll.h"

#include "event/cherryPlatformEvents.h"
#include "service/cherryServiceRegistry.h"
#include "service/cherryIExtensionPointService.h"

namespace cherry {

using namespace Poco;

class CHERRY_OSGI Platform
{
public:
  static int OS_FREE_BSD;
  static int OS_AIX;
  static int OS_HPUX;
  static int OS_TRU64;
  static int OS_LINUX;
  static int OS_MAC_OS_X;
  static int OS_NET_BSD;
  static int OS_OPEN_BSD;
  static int OS_IRIX;
  static int OS_SOLARIS;
  static int OS_QNX;
  static int OS_VXWORKS;
  static int OS_CYGWIN;
  static int OS_UNKNOWN_UNIX;
  static int OS_WINDOWS_NT;
  static int OS_WINDOWS_CE;
  static int OS_VMS;

  static int ARCH_ALPHA;
  static int ARCH_IA32;
  static int ARCH_IA64;
  static int ARCH_MIPS;
  static int ARCH_HPPA;
  static int ARCH_PPC;
  static int ARCH_POWER;
  static int ARCH_SPARC;
  static int ARCH_AMD64;
  static int ARCH_ARM;
  
  static std::string ARG_CLEAN;
  static std::string ARG_APPLICATION;
  static std::string ARG_HOME;
  static std::string ARG_PLUGIN_CACHE;
  static std::string ARG_PLUGIN_DIRS;
  
  static IExtensionPointService* GetExtensionPointService();
  // static IPreferenceService GetPreferenceService();
  
  static PlatformEvents& GetEvents();
  
  static const Path* GetConfigurationPath();
  static const Path* GetInstallPath();
  static const Path* GetInstancePath();
  static const Path* GetStatePath(IBundle* bundle);
  static const Path* GetUserPath();
  
  static int GetOS();
  static int GetOSArch();
  static bool IsUnix();
  static bool IsWindows();
  static bool IsBSD();
  static bool IsLinux();
  static bool IsVMS();
  
  static std::string GetProperty(const std::string& key);
  
  static bool IsRunning();
  
  static void GetRawApplicationArgs(int& argc, char**& argv);
  
  static bool HasArgument(const std::string& arg);
  static const std::string& GetArgValue(const std::string& arg);
  
  static ServiceRegistry& GetServiceRegistry();
  
  static IBundle::Pointer GetBundle(const std::string& id);

private:
  Platform();
};

}  // namespace

#endif // CHERRY_Platform_INCLUDED
