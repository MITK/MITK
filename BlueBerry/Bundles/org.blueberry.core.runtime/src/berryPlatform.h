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

#ifndef BERRY_Platform_INCLUDED
#define BERRY_Platform_INCLUDED


//
// Platform Identification
//
#define BERRY_OS_FREE_BSD      0x0001
#define BERRY_OS_AIX           0x0002
#define BERRY_OS_HPUX          0x0003
#define BERRY_OS_TRU64         0x0004
#define BERRY_OS_LINUX         0x0005
#define BERRY_OS_MAC_OS_X      0x0006
#define BERRY_OS_NET_BSD       0x0007
#define BERRY_OS_OPEN_BSD      0x0008
#define BERRY_OS_IRIX          0x0009
#define BERRY_OS_SOLARIS       0x000a
#define BERRY_OS_QNX           0x000b
#define BERRY_OS_VXWORKS       0x000c
#define BERRY_OS_CYGWIN        0x000d
#define BERRY_OS_UNKNOWN_UNIX  0x00ff
#define BERRY_OS_WINDOWS_NT    0x1001
#define BERRY_OS_WINDOWS_CE    0x1011
#define BERRY_OS_VMS           0x2001


#if defined(__FreeBSD__)
  #define BERRY_OS_FAMILY_UNIX 1
  #define BERRY_OS_FAMILY_BSD 1
  #define BERRY_OS BERRY_OS_FREE_BSD
#elif defined(_AIX) || defined(__TOS_AIX__)
  #define BERRY_OS_FAMILY_UNIX 1
  #define BERRY_OS BERRY_OS_AIX
#elif defined(hpux) || defined(_hpux)
  #define BERRY_OS_FAMILY_UNIX 1
  #define BERRY_OS BERRY_OS_HPUX
#elif defined(__digital__) || defined(__osf__)
  #define BERRY_OS_FAMILY_UNIX 1
  #define BERRY_OS BERRY_OS_TRU64
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__TOS_LINUX__)
  #define BERRY_OS_FAMILY_UNIX 1
  #define BERRY_OS BERRY_OS_LINUX
#elif defined(__APPLE__) || defined(__TOS_MACOS__)
  #define BERRY_OS_FAMILY_UNIX 1
  #define BERRY_OS_FAMILY_BSD 1
  #define BERRY_OS BERRY_OS_MAC_OS_X
#elif defined(__NetBSD__)
  #define BERRY_OS_FAMILY_UNIX 1
  #define BERRY_OS_FAMILY_BSD 1
  #define BERRY_OS BERRY_OS_NET_BSD
#elif defined(__OpenBSD__)
  #define BERRY_OS_FAMILY_UNIX 1
  #define BERRY_OS_FAMILY_BSD 1
  #define BERRY_OS BERRY_OS_OPEN_BSD
#elif defined(sgi) || defined(__sgi)
  #define BERRY_OS_FAMILY_UNIX 1
  #define BERRY_OS BERRY_OS_IRIX
#elif defined(sun) || defined(__sun)
  #define BERRY_OS_FAMILY_UNIX 1
  #define BERRY_OS BERRY_OS_SOLARIS
#elif defined(__QNX__)
  #define BERRY_OS_FAMILY_UNIX 1
  #define BERRY_OS BERRY_OS_QNX
#elif defined(unix) || defined(__unix) || defined(__unix__)
  #define BERRY_OS_FAMILY_UNIX 1
  #define BERRY_OS BERRY_OS_UNKNOWN_UNIX
#elif defined(_WIN32_WCE)
  #define BERRY_OS_FAMILY_WINDOWS 1
  #define BERRY_OS BERRY_OS_WINDOWS_CE
#elif defined(_WIN32) || defined(_WIN64)
  #define BERRY_OS_FAMILY_WINDOWS 1
  #define BERRY_OS BERRY_OS_WINDOWS_NT
#elif defined(__CYGWIN__)
  #define BERRY_OS_FAMILY_UNIX 1
  #define BERRY_OS BERRY_OS_CYGWIN
#elif defined(__VMS)
  #define BERRY_OS_FAMILY_VMS 1
  #define BERRY_OS BERRY_OS_VMS
#endif


//
// Hardware Architecture and Byte Order
//
#define BERRY_ARCH_ALPHA   0x01
#define BERRY_ARCH_IA32    0x02
#define BERRY_ARCH_IA64    0x03
#define BERRY_ARCH_MIPS    0x04
#define BERRY_ARCH_HPPA    0x05
#define BERRY_ARCH_PPC     0x06
#define BERRY_ARCH_POWER   0x07
#define BERRY_ARCH_SPARC   0x08
#define BERRY_ARCH_AMD64   0x09
#define BERRY_ARCH_ARM     0x0a


#if defined(__ALPHA) || defined(__alpha) || defined(__alpha__) || defined(_M_ALPHA)
  #define BERRY_ARCH BERRY_ARCH_ALPHA
  #define BERRY_ARCH_LITTLE_ENDIAN 1
#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(_M_IX86)
  #define BERRY_ARCH BERRY_ARCH_IA32
  #define BERRY_ARCH_LITTLE_ENDIAN 1
#elif defined(_IA64) || defined(__IA64__) || defined(__ia64__) || defined(__ia64) || defined(_M_IA64)
  #define BERRY_ARCH BERRY_ARCH_IA64
  #if defined(hpux) || defined(_hpux)
    #define BERRY_ARCH_BIG_ENDIAN 1
  #else
    #define BERRY_ARCH_LITTLE_ENDIAN 1
  #endif
#elif defined(__x86_64__)
  #define BERRY_ARCH BERRY_ARCH_AMD64
  #define BERRY_ARCH_LITTLE_ENDIAN 1
#elif defined(_M_X64)
#define BERRY_ARCH BERRY_ARCH_AMD64
#define BERRY_ARCH_LITTLE_ENDIAN 1
#elif defined(__mips__) || defined(__mips) || defined(__MIPS__) || defined(_M_MRX000)
  #define BERRY_ARCH BERRY_ARCH_MIPS
  #define BERRY_ARCH_BIG_ENDIAN 1
#elif defined(__hppa) || defined(__hppa__)
  #define BERRY_ARCH BERRY_ARCH_HPPA
  #define BERRY_ARCH_BIG_ENDIAN 1
#elif defined(__PPC) || defined(__POWERPC__) || defined(__powerpc) || defined(__PPC__) || \
      defined(__powerpc__) || defined(__ppc__) || defined(_ARCH_PPC) || defined(_M_PPC)
  #define BERRY_ARCH BERRY_ARCH_PPC
  #define BERRY_ARCH_BIG_ENDIAN 1
#elif defined(_POWER) || defined(_ARCH_PWR) || defined(_ARCH_PWR2) || defined(_ARCH_PWR3) || \
      defined(_ARCH_PWR4) || defined(__THW_RS6000)
  #define BERRY_ARCH BERRY_ARCH_POWER
  #define BERRY_ARCH_BIG_ENDIAN 1
#elif defined(__sparc__) || defined(__sparc) || defined(sparc)
  #define BERRY_ARCH BERRY_ARCH_SPARC
  #define BERRY_ARCH_BIG_ENDIAN 1
#elif defined(__arm__) || defined(__arm) || defined(ARM) || defined(_ARM_) || defined(__ARM__) || defined(_M_ARM)
  #define BERRY_ARCH BERRY_ARCH_ARM
  #if defined(__ARMEB__)
    #define BERRY_ARCH_BIG_ENDIAN 1
  #else
    #define BERRY_ARCH_LITTLE_ENDIAN 1
  #endif
#endif


#include <org_blueberry_core_runtime_Export.h>

#include <Poco/Util/LayeredConfiguration.h>
#include <Poco/Util/OptionSet.h>

#include <QDir>
#include <QSharedPointer>

class ctkPlugin;

namespace Poco {

class Path;

}

namespace berry {

struct IAdapterManager;
struct IBundle;
struct IExtensionPointService;
struct IExtensionRegistry;
struct IPreferencesService;

/**
 * The central class of the BlueBerry Platform Runtime. This class cannot
 * be instantiated or subclassed by clients; all functionality is provided
 * by static methods.  Features include:
 * <ul>
 * <li>the platform registry of installed plug-ins</li>
 * <li>the platform adapter manager</li>
 * <li>the platform log</li>
 * </ul>
 * <p>
 * Most users don't have to worry about Platform's lifecycle. However, if your
 * code can call methods of this class when Platform is not running, it becomes
 * necessary to check {@link #IsRunning()} before making the call. A runtime
 * exception might be thrown or incorrect result might be returned if a method
 * from this class is called while Platform is not running.
 * </p>
 */
class org_blueberry_core_runtime_EXPORT Platform
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

  static QString PROP_QTPLUGIN_PATH;

  static QString ARG_NEWINSTANCE;
  static QString ARG_CLEAN;
  static QString ARG_APPLICATION;
  static QString ARG_HOME;
  static QString ARG_STORAGE_DIR;
  static QString ARG_PLUGIN_CACHE;
  static QString ARG_PLUGIN_DIRS;
  static QString ARG_FORCE_PLUGIN_INSTALL;
  static QString ARG_PRELOAD_LIBRARY;
  static QString ARG_PROVISIONING;

  static QString ARG_CONSOLELOG;
  static QString ARG_TESTPLUGIN;
  static QString ARG_TESTAPPLICATION;

  static QString ARG_NO_REGISTRY_CACHE;
  static QString ARG_NO_LAZY_REGISTRY_CACHE_LOADING;
  static QString ARG_REGISTRY_MULTI_LANGUAGE;

  static QString ARG_XARGS;

  /**
   * Returns the adapter manager used for extending
   * <code>IAdaptable</code> objects.
   *
   * @return the adapter manager for this platform
   * @see IAdapterManager
   */
  static IAdapterManager* GetAdapterManager();

  /**
   * Returns the extension registry for this platform.
   * May return <code>null</code> if the registry has not been created yet.
   *
   * @return existing extension registry or <code>null</code>
   * @see IExtensionRegistry
   */
  static IExtensionRegistry* GetExtensionRegistry();

  /**
   * Return the interface into the preference mechanism. The returned
   * object can be used for such operations as searching for preference
   * values across multiple scopes and preference import/export.
   * <p>
   * Clients are also able to acquire the IPreferencesService service via
   * CTK mechanisms and use it for preference functions.
   * </p>
   *
   * @return an object to interface into the preference mechanism
   */
  static IPreferencesService* GetPreferencesService();

  /**
   * Returns the path of the configuration information
   * used to run this instance of the BlueBerry platform.
   * The configuration area typically
   * contains the list of plug-ins available for use, various settings
   * (those shared across different instances of the same configuration)
   * and any other such data needed by plug-ins.
   * An empty path is returned if the platform is running without a configuration location.
   *
   * @return the location of the platform's configuration data area
   */
  static QDir GetConfigurationPath();

  /**
   * Returns the path of the base installation for the running platform
   *
   * @return the location of the platform's installation area or <code>null</code> if none
   */
  static QDir GetInstallPath();

  /**
   * Returns the path of the platform's working directory (also known as the instance data area).
   * An empty path is returned if the platform is running without an instance location.
   *
   * @return the location of the platform's instance data area or <code>null</code> if none
   */
  static QDir GetInstancePath();

  /**
   * Returns the path in the local file system of the
   * plug-in state area for the given plug-in.
   * If the plug-in state area did not exist prior to this call,
   * it is created.
   * <p>
   * The plug-in state area is a file directory within the
   * platform's metadata area where a plug-in is free to create files.
   * The content and structure of this area is defined by the plug-in,
   * and the particular plug-in is solely responsible for any files
   * it puts there. It is recommended for plug-in preference settings and
   * other configuration parameters.
   * </p>
   *
   * @param plugin the plug-in whose state location is returned
   * @return a local file system path
   * TODO Investigate the usage of a service factory
   */
  static bool GetStatePath(QDir& statePath, const QSharedPointer<ctkPlugin>& plugin, bool create = true);

  /**
   * Returns the path of the platform's user data area.  The user data area is a location on the system
   * which is specific to the system's current user.  By default it is located relative to the
   * location given by the System property "user.home".
   * An empty path is returned if the platform is running without an user location.
   *
   * @return the location of the platform's user data area or <code>null</code> if none
   */
  static QDir GetUserPath();

  static int GetOS();
  static int GetOSArch();
  static bool IsUnix();
  static bool IsWindows();
  static bool IsBSD();
  static bool IsLinux();
  static bool IsVMS();

  static QString GetProperty(const QString& key);

  static bool IsRunning();

  static Poco::Util::LayeredConfiguration& GetConfiguration();

  /**
   * Returns the unmodified, original command line arguments
   *
   */
  static int& GetRawApplicationArgs(char**& argv);

  /**
   * Returns the applications command line arguments which
   * have not been consumed by the platform.
   */
  static QStringList GetApplicationArgs();

  /**
   * Returns the "extended" command line arguments. This is
   * just the string given as argument to the "--xargs" option.
   */
  static QString GetExtendedApplicationArgs();

  static void GetOptionSet(Poco::Util::OptionSet &os);

  /**
   * @param symbolicName
   * @deprecated Use GetPlugin(const QString&) instead
   */
  static QSharedPointer<ctkPlugin> GetCTKPlugin(const QString& symbolicName);

  /**
   * @brief GetCTKPlugin
   * @param id
   * @deprecated Use GetPlugin(long) instead
   */
  static QSharedPointer<ctkPlugin> GetCTKPlugin(long id);

  /**
   * Returns the resolved plug-in with the specified symbolic name that has the
   * highest version. If no resolved plug-ins are installed that have the
   * specified symbolic name then null is returned.
   * <p>
   * Note that clients may want to filter
   * the results based on the state of the plug-ins.
   * </p>
   * @param symbolicName the symbolic name of the plug-in to be returned.
   * @return the plug-in that has the specified symbolic name with the
   * highest version, or <tt>null</tt> if no plug-in is found.
   */
  static QSharedPointer<ctkPlugin> GetPlugin(const QString& symbolicName);

  /**
   * Returns all plug-ins with the specified symbolic name.  If no resolved plug-ins
   * with the specified symbolic name can be found, an empty list is returned.
   * If the version argument is not null then only the plug-ins that have
   * the specified symbolic name and a version greater than or equal to the
   * specified version are returned. The returned plug-ins are ordered in
   * descending plug-in version order.
   * <p>
   * Note that clients may want to filter
   * the results based on the state of the plug-ins.
   * </p>
   * @param symbolicName the symbolic name of the plug-ins that are to be returned.
   * @param version the version that the returned plug-in versions must match,
   * or <tt>QString()</tt> if no version matching is to be done.
   * @return the list of plug-ins with the specified name that match the
   * specified version and match rule, or <tt>null</tt> if no plug-ins are found.
   */
  static QList<QSharedPointer<ctkPlugin> > GetPlugins(const QString& symbolicName,
                                                      const QString& version = QString());


private:
  Platform();
};

}  // namespace

#endif // BERRY_Platform_INCLUDED
