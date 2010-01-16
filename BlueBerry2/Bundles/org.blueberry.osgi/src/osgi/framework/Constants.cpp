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

#include "Constants.h"

namespace osgi
{
namespace framework
{

const std::string Constants::SYSTEM_BUNDLE_LOCATION = "System Bundle";

const std::string Constants::SYSTEM_BUNDLE_SYMBOLICNAME = "system.bundle";

const std::string Constants::BUNDLE_CATEGORY = "Bundle-Category";

const std::string Constants::BUNDLE_CLASSPATH = "Bundle-ClassPath";

const std::string Constants::BUNDLE_COPYRIGHT = "Bundle-Copyright";

const std::string Constants::BUNDLE_DESCRIPTION = "Bundle-Description";

const std::string Constants::BUNDLE_NAME = "Bundle-Name";

const std::string Constants::BUNDLE_NATIVECODE = "Bundle-NativeCode";

const std::string Constants::EXPORT_PACKAGE = "Export-Package";

const std::string Constants::EXPORT_SERVICE = "Export-Service";

const std::string Constants::IMPORT_PACKAGE = "Import-Package";

const std::string Constants::DYNAMICIMPORT_PACKAGE = "DynamicImport-Package";

const std::string Constants::IMPORT_SERVICE = "Import-Service";

const std::string Constants::BUNDLE_VENDOR = "Bundle-Vendor";

const std::string Constants::BUNDLE_VERSION = "Bundle-Version";

const std::string Constants::BUNDLE_DOCURL = "Bundle-DocURL";

const std::string Constants::BUNDLE_CONTACTADDRESS = "Bundle-ContactAddress";

const std::string Constants::BUNDLE_ACTIVATOR = "Bundle-Activator";

const std::string Constants::BUNDLE_UPDATELOCATION = "Bundle-UpdateLocation";

const std::string Constants::PACKAGE_SPECIFICATION_VERSION =
    "specification-version";

const std::string Constants::BUNDLE_NATIVECODE_PROCESSOR = "processor";

const std::string Constants::BUNDLE_NATIVECODE_OSNAME = "osname";

const std::string Constants::BUNDLE_NATIVECODE_OSVERSION = "osversion";

const std::string Constants::BUNDLE_NATIVECODE_LANGUAGE = "language";

const std::string Constants::BUNDLE_REQUIREDEXECUTIONENVIRONMENT =
    "Bundle-RequiredExecutionEnvironment";

const std::string Constants::BUNDLE_SYMBOLICNAME = "Bundle-SymbolicName";

const std::string Constants::SINGLETON_DIRECTIVE = "singleton";

const std::string Constants::FRAGMENT_ATTACHMENT_DIRECTIVE =
    "fragment-attachment";

const std::string Constants::FRAGMENT_ATTACHMENT_ALWAYS = "always";

const std::string Constants::FRAGMENT_ATTACHMENT_RESOLVETIME = "resolve-time";

const std::string Constants::FRAGMENT_ATTACHMENT_NEVER = "never";

const std::string Constants::BUNDLE_LOCALIZATION = "Bundle-Localization";

const std::string Constants::BUNDLE_LOCALIZATION_DEFAULT_BASENAME =
    "OSGI-INF/l10n/bundle";

const std::string Constants::REQUIRE_BUNDLE = "Require-Bundle";

const std::string Constants::BUNDLE_VERSION_ATTRIBUTE = "bundle-version";

const std::string Constants::FRAGMENT_HOST = "Fragment-Host";

const std::string Constants::SELECTION_FILTER_ATTRIBUTE = "selection-filter";

const std::string Constants::BUNDLE_MANIFESTVERSION = "Bundle-ManifestVersion";

const std::string Constants::VERSION_ATTRIBUTE = "version";

const std::string Constants::BUNDLE_SYMBOLICNAME_ATTRIBUTE =
    "bundle-symbolic-name";

const std::string Constants::RESOLUTION_DIRECTIVE = "resolution";

const std::string Constants::RESOLUTION_MANDATORY = "mandatory";

const std::string Constants::RESOLUTION_OPTIONAL = "optional";

const std::string Constants::USES_DIRECTIVE = "uses";

const std::string Constants::INCLUDE_DIRECTIVE = "include";

const std::string Constants::EXCLUDE_DIRECTIVE = "exclude";

const std::string Constants::MANDATORY_DIRECTIVE = "mandatory";

const std::string Constants::VISIBILITY_DIRECTIVE = "visibility";

const std::string Constants::VISIBILITY_PRIVATE = "private";

const std::string Constants::VISIBILITY_REEXPORT = "reexport";

const std::string Constants::EXTENSION_DIRECTIVE = "extension";

const std::string Constants::EXTENSION_FRAMEWORK = "framework";

const std::string Constants::EXTENSION_BOOTCLASSPATH = "bootclasspath";

const std::string Constants::BUNDLE_ACTIVATIONPOLICY =
    "Bundle-ActivationPolicy";

const std::string Constants::ACTIVATION_LAZY = "lazy";

const std::string Constants::FRAMEWORK_VERSION = "org.osgi.framework.version";

const std::string Constants::FRAMEWORK_VENDOR = "org.osgi.framework.vendor";

const std::string Constants::FRAMEWORK_LANGUAGE = "org.osgi.framework.language";

const std::string Constants::FRAMEWORK_OS_NAME = "org.osgi.framework.os.name";

const std::string Constants::FRAMEWORK_OS_VERSION =
    "org.osgi.framework.os.version";

const std::string Constants::FRAMEWORK_PROCESSOR =
    "org.osgi.framework.processor";

const std::string Constants::FRAMEWORK_EXECUTIONENVIRONMENT =
    "org.osgi.framework.executionenvironment";

const std::string Constants::FRAMEWORK_BOOTDELEGATION =
    "org.osgi.framework.bootdelegation";

const std::string Constants::FRAMEWORK_SYSTEMPACKAGES =
    "org.osgi.framework.system.packages";

const std::string Constants::FRAMEWORK_SYSTEMPACKAGES_EXTRA =
    "org.osgi.framework.system.packages.extra";

const std::string Constants::SUPPORTS_FRAMEWORK_EXTENSION =
    "org.osgi.supports.framework.extension";

const std::string Constants::SUPPORTS_BOOTCLASSPATH_EXTENSION =
    "org.osgi.supports.bootclasspath.extension";

const std::string Constants::SUPPORTS_FRAMEWORK_FRAGMENT =
    "org.osgi.supports.framework.fragment";

const std::string Constants::SUPPORTS_FRAMEWORK_REQUIREBUNDLE =
    "org.osgi.supports.framework.requirebundle";

const std::string Constants::FRAMEWORK_SECURITY = "org.osgi.framework.security";

const std::string Constants::FRAMEWORK_SECURITY_OSGI = "osgi";

const std::string Constants::FRAMEWORK_STORAGE = "org.osgi.framework.storage";

const std::string Constants::FRAMEWORK_STORAGE_CLEAN =
    "org.osgi.framework.storage.clean";

const std::string Constants::FRAMEWORK_STORAGE_CLEAN_ONFIRSTINIT =
    "onFirstInit";

const std::string Constants::FRAMEWORK_LIBRARY_EXTENSIONS =
    "org.osgi.framework.library.extensions";

const std::string Constants::FRAMEWORK_EXECPERMISSION =
    "org.osgi.framework.command.execpermission";

const std::string Constants::FRAMEWORK_TRUST_REPOSITORIES =
    "org.osgi.framework.trust.repositories";

const std::string Constants::FRAMEWORK_WINDOWSYSTEM =
    "org.osgi.framework.windowsystem";

const std::string Constants::FRAMEWORK_BEGINNING_STARTLEVEL =
    "org.osgi.framework.startlevel.beginning";

const std::string Constants::FRAMEWORK_BUNDLE_PARENT =
    "org.osgi.framework.bundle.parent";

const std::string Constants::FRAMEWORK_BUNDLE_PARENT_BOOT = "boot";

const std::string Constants::FRAMEWORK_BUNDLE_PARENT_EXT = "ext";

const std::string Constants::FRAMEWORK_BUNDLE_PARENT_APP = "app";

const std::string Constants::FRAMEWORK_BUNDLE_PARENT_FRAMEWORK = "framework";

const std::string Constants::OBJECTCLASS = "objectClass";

const std::string Constants::SERVICE_ID = "service.id";

const std::string Constants::SERVICE_PID = "service.pid";

const std::string Constants::SERVICE_RANKING = "service.ranking";

const std::string Constants::SERVICE_VENDOR = "service.vendor";

const std::string Constants::SERVICE_DESCRIPTION = "service.description";

}
}
