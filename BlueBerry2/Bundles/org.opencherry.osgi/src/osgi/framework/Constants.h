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
 PURPOSE.  See the above copyright notices for more information.; // =========================================================================*/


#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <string>

namespace osgi {
namespace framework {

/**
 * Defines standard names for the OSGi environment system properties, service
 * properties, and Manifest header attribute keys.
 *
 * <p>
 * The values associated with these keys are of type
 * <code>std::string</code>, unless otherwise indicated.
 *
 */

struct Constants {

  /**
   * Location identifier of the OSGi <i>system bundle </i>, which is defined
   * to be &quot;System Bundle&quot;.
   */
   static const std::string  SYSTEM_BUNDLE_LOCATION; // = "System Bundle";

  /**
   * Alias for the symbolic name of the OSGi <i>system bundle </i>. It is
   * defined to be &quot;system.bundle&quot;.
   *
   * @since 1.3
   */
   static const std::string  SYSTEM_BUNDLE_SYMBOLICNAME; // = "system.bundle";

  /**
   * Manifest header identifying the bundle's category.
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   */
   static const std::string  BUNDLE_CATEGORY; // = "Bundle-Category";

  /**
   * Manifest header identifying a list of directories and embedded JAR files,
   * which are bundle resources used to extend the bundle's classpath.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle#GetHeaders</code> method.
   */
   static const std::string  BUNDLE_CLASSPATH; // = "Bundle-ClassPath";

  /**
   * Manifest header identifying the bundle's copyright information.
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle#GetHeaders</code> method.
   */
   static const std::string  BUNDLE_COPYRIGHT; // = "Bundle-Copyright";

  /**
   * Manifest header containing a brief description of the bundle's
   * functionality.
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle#GetHeaders</code> method.
   */
   static const std::string  BUNDLE_DESCRIPTION; // = "Bundle-Description";

  /**
   * Manifest header identifying the bundle's name.
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle#GetHeaders</code> method.
   */
   static const std::string  BUNDLE_NAME; // = "Bundle-Name";

  /**
   * Manifest header identifying a number of hardware environments and the
   * native language code libraries that the bundle is carrying for each of
   * these environments.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle#GetHeaders</code> method.
   */
   static const std::string  BUNDLE_NATIVECODE; // = "Bundle-NativeCode";

  /**
   * Manifest header identifying the packages that the bundle offers to the
   * Framework for export.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle#GetHeaders</code> method.
   */
   static const std::string  EXPORT_PACKAGE; // = "Export-Package";

  /**
   * Manifest header identifying the fully qualified class names of the
   * services that the bundle may register (used for informational purposes
   * only).
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   *
   * @deprecated As of 1.2.
   */
   static const std::string  EXPORT_SERVICE; // = "Export-Service";

  /**
   * Manifest header identifying the packages on which the bundle depends.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   */
   static const std::string  IMPORT_PACKAGE; // = "Import-Package";

  /**
   * Manifest header identifying the packages that the bundle may dynamically
   * import during execution.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   *
   * @since 1.2
   */
   static const std::string  DYNAMICIMPORT_PACKAGE; // = "DynamicImport-Package";

  /**
   * Manifest header identifying the fully qualified class names of the
   * services that the bundle requires (used for informational purposes only).
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   *
   * @deprecated As of 1.2.
   */
   static const std::string  IMPORT_SERVICE; // = "Import-Service";

  /**
   * Manifest header identifying the bundle's vendor.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   */
   static const std::string  BUNDLE_VENDOR; // = "Bundle-Vendor";

  /**
   * Manifest header identifying the bundle's version.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   */
   static const std::string  BUNDLE_VERSION; // = "Bundle-Version";

  /**
   * Manifest header identifying the bundle's documentation URL, from which
   * further information about the bundle may be obtained.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   */
   static const std::string  BUNDLE_DOCURL; // = "Bundle-DocURL";

  /**
   * Manifest header identifying the contact address where problems with the
   * bundle may be reported; for example, an email address.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   */
   static const std::string  BUNDLE_CONTACTADDRESS; // = "Bundle-ContactAddress";

  /**
   * Manifest header attribute identifying the bundle's activator class.
   *
   * <p>
   * If present, this header specifies the name of the bundle resource class
   * that implements the <code>BundleActivator</code> interface and whose
   * <code>start</code> and <code>stop</code> methods are called by the
   * Framework when the bundle is started and stopped, respectively.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   */
   static const std::string  BUNDLE_ACTIVATOR; // = "Bundle-Activator";

  /**
   * Manifest header identifying the location from which a new bundle version
   * is obtained during a bundle update operation.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   */
   static const std::string  BUNDLE_UPDATELOCATION; // = "Bundle-UpdateLocation";

  /**
   * Manifest header attribute identifying the version of a package specified
   * in the Export-Package or Import-Package manifest header.
   *
   * @deprecated As of 1.3. This has been replaced by
   *             {@link #VERSION_ATTRIBUTE}.
   */
   static const std::string  PACKAGE_SPECIFICATION_VERSION; // = "specification-version";

  /**
   * Manifest header attribute identifying the processor required to run
   * native bundle code specified in the Bundle-NativeCode manifest header).
   *
   * <p>
   * The attribute value is encoded in the Bundle-NativeCode manifest header
   * like:
   *
   * <pre>
   *     Bundle-NativeCode: http.so ; processor; // =x86 ...
   * </pre>
   *
   * @see #BUNDLE_NATIVECODE
   */
   static const std::string  BUNDLE_NATIVECODE_PROCESSOR; // = "processor";

  /**
   * Manifest header attribute identifying the operating system required to
   * run native bundle code specified in the Bundle-NativeCode manifest
   * header).
   * <p>
   * The attribute value is encoded in the Bundle-NativeCode manifest header
   * like:
   *
   * <pre>
   *     Bundle-NativeCode: http.so ; osname; // =Linux ...
   * </pre>
   *
   * @see #BUNDLE_NATIVECODE
   */
   static const std::string  BUNDLE_NATIVECODE_OSNAME; // = "osname";

  /**
   * Manifest header attribute identifying the operating system version
   * required to run native bundle code specified in the Bundle-NativeCode
   * manifest header).
   * <p>
   * The attribute value is encoded in the Bundle-NativeCode manifest header
   * like:
   *
   * <pre>
   *     Bundle-NativeCode: http.so ; osversion; // =&quot;2.34&quot; ...
   * </pre>
   *
   * @see #BUNDLE_NATIVECODE
   */
   static const std::string  BUNDLE_NATIVECODE_OSVERSION; // = "osversion";

  /**
   * Manifest header attribute identifying the language in which the native
   * bundle code is written specified in the Bundle-NativeCode manifest
   * header. See ISO 639 for possible values.
   * <p>
   * The attribute value is encoded in the Bundle-NativeCode manifest header
   * like:
   *
   * <pre>
   *     Bundle-NativeCode: http.so ; language; // =nl_be ...
   * </pre>
   *
   * @see #BUNDLE_NATIVECODE
   */
   static const std::string  BUNDLE_NATIVECODE_LANGUAGE; // = "language";

  /**
   * Manifest header identifying the required execution environment for the
   * bundle. The service platform may run this bundle if any of the execution
   * environments named in this header matches one of the execution
   * environments it implements.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   *
   * @since 1.2
   */
   static const std::string  BUNDLE_REQUIREDEXECUTIONENVIRONMENT; // = "Bundle-RequiredExecutionEnvironment";

  /**
   * Manifest header identifying the bundle's symbolic name.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   *
   * @since 1.3
   */
   const static std::string  BUNDLE_SYMBOLICNAME; // = "Bundle-SymbolicName";

  /**
   * Manifest header directive identifying whether a bundle is a singleton.
   * The default value is <code>false</code>.
   *
   * <p>
   * The directive value is encoded in the Bundle-SymbolicName manifest header
   * like:
   *
   * <pre>
   *     Bundle-SymbolicName: com.acme.module.test; singleton:; // =true
   * </pre>
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   *
   * @see #BUNDLE_SYMBOLICNAME
   * @since 1.3
   */
   const static std::string  SINGLETON_DIRECTIVE; // = "singleton";

  /**
   * Manifest header directive identifying if and when a fragment may attach
   * to a host bundle. The default value is
   * {@link #FRAGMENT_ATTACHMENT_ALWAYS always}.
   *
   * <p>
   * The directive value is encoded in the Bundle-SymbolicName manifest header
   * like:
   *
   * <pre>
   *     Bundle-SymbolicName: com.acme.module.test; fragment-attachment:; // =&quot;never&quot;
   * </pre>
   *
   * @see #BUNDLE_SYMBOLICNAME
   * @see #FRAGMENT_ATTACHMENT_ALWAYS
   * @see #FRAGMENT_ATTACHMENT_RESOLVETIME
   * @see #FRAGMENT_ATTACHMENT_NEVER
   * @since 1.3
   */
   const static std::string  FRAGMENT_ATTACHMENT_DIRECTIVE; // = "fragment-attachment";

  /**
   * Manifest header directive value identifying a fragment attachment type of
   * always. A fragment attachment type of always indicates that fragments are
   * allowed to attach to the host bundle at any time (while the host is
   * resolved or during the process of resolving the host bundle).
   *
   * <p>
   * The directive value is encoded in the Bundle-SymbolicName manifest header
   * like:
   *
   * <pre>
   *     Bundle-SymbolicName: com.acme.module.test; fragment-attachment:; // =&quot;always&quot;
   * </pre>
   *
   * @see #FRAGMENT_ATTACHMENT_DIRECTIVE
   * @since 1.3
   */
   const static std::string  FRAGMENT_ATTACHMENT_ALWAYS; // = "always";

  /**
   * Manifest header directive value identifying a fragment attachment type of
   * resolve-time. A fragment attachment type of resolve-time indicates that
   * fragments are allowed to attach to the host bundle only during the
   * process of resolving the host bundle.
   *
   * <p>
   * The directive value is encoded in the Bundle-SymbolicName manifest header
   * like:
   *
   * <pre>
   *     Bundle-SymbolicName: com.acme.module.test; fragment-attachment:; // =&quot;resolve-time&quot;
   * </pre>
   *
   * @see #FRAGMENT_ATTACHMENT_DIRECTIVE
   * @since 1.3
   */
   const static std::string  FRAGMENT_ATTACHMENT_RESOLVETIME; // = "resolve-time";

  /**
   * Manifest header directive value identifying a fragment attachment type of
   * never. A fragment attachment type of never indicates that no fragments
   * are allowed to attach to the host bundle at any time.
   *
   * <p>
   * The directive value is encoded in the Bundle-SymbolicName manifest header
   * like:
   *
   * <pre>
   *     Bundle-SymbolicName: com.acme.module.test; fragment-attachment:; // =&quot;never&quot;
   * </pre>
   *
   * @see #FRAGMENT_ATTACHMENT_DIRECTIVE
   * @since 1.3
   */
   const static std::string  FRAGMENT_ATTACHMENT_NEVER; // = "never";

  /**
   * Manifest header identifying the base name of the bundle's localization
   * entries.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   *
   * @see #BUNDLE_LOCALIZATION_DEFAULT_BASENAME
   * @since 1.3
   */
   const static std::string  BUNDLE_LOCALIZATION; // = "Bundle-Localization";

  /**
   * Default value for the <code>Bundle-Localization</code> manifest header.
   *
   * @see #BUNDLE_LOCALIZATION
   * @since 1.3
   */
   const static std::string  BUNDLE_LOCALIZATION_DEFAULT_BASENAME; // = "OSGI-INF/l10n/bundle";

  /**
   * Manifest header identifying the symbolic names of other bundles required
   * by the bundle.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   *
   * @since 1.3
   */
   const static std::string  REQUIRE_BUNDLE; // = "Require-Bundle";

  /**
   * Manifest header attribute identifying a range of versions for a bundle
   * specified in the <code>Require-Bundle</code> or
   * <code>Fragment-Host</code> manifest headers. The default value is
   * <code>0.0.0</code>.
   *
   * <p>
   * The attribute value is encoded in the Require-Bundle manifest header
   * like:
   *
   * <pre>
   *     Require-Bundle: com.acme.module.test; bundle-version; // =&quot;1.1&quot;
   *     Require-Bundle: com.acme.module.test; bundle-version; // =&quot;[1.0,2.0)&quot;
   * </pre>
   *
   * <p>
   * The bundle-version attribute value uses a mathematical interval notation
   * to specify a range of bundle versions. A bundle-version attribute value
   * specified as a single version means a version range that includes any
   * bundle version greater than or equal to the specified version.
   *
   * @see #REQUIRE_BUNDLE
   * @since 1.3
   */
   static const std::string  BUNDLE_VERSION_ATTRIBUTE; // = "bundle-version";

  /**
   * Manifest header identifying the symbolic name of another bundle for which
   * that the bundle is a fragment.
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   *
   * @since 1.3
   */
   const static std::string  FRAGMENT_HOST; // = "Fragment-Host";

  /**
   * Manifest header attribute is used for selection by filtering based upon
   * system properties.
   *
   * <p>
   * The attribute value is encoded in manifest headers like:
   *
   * <pre>
   *     Bundle-NativeCode: libgtk.so; selection-filter; // =&quot;(ws=gtk)&quot;; ...
   * </pre>
   *
   * @see #BUNDLE_NATIVECODE
   * @since 1.3
   */
   const static std::string  SELECTION_FILTER_ATTRIBUTE; // = "selection-filter";

  /**
   * Manifest header identifying the bundle manifest version. A bundle
   * manifest may express the version of the syntax in which it is written by
   * specifying a bundle manifest version. Bundles exploiting OSGi Release 4,
   * or later, syntax must specify a bundle manifest version.
   * <p>
   * The bundle manifest version defined by OSGi Release 4 or, more
   * specifically, by version 1.3 of the OSGi Core Specification is "2".
   *
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   *
   * @since 1.3
   */
   const static std::string  BUNDLE_MANIFESTVERSION; // = "Bundle-ManifestVersion";

  /**
   * Manifest header attribute identifying the version of a package specified
   * in the Export-Package or Import-Package manifest header.
   *
   * <p>
   * The attribute value is encoded in the Export-Package or Import-Package
   * manifest header like:
   *
   * <pre>
   *     Import-Package: org.osgi.framework; version; // =&quot;1.1&quot;
   * </pre>
   *
   * @see #EXPORT_PACKAGE
   * @see #IMPORT_PACKAGE
   * @since 1.3
   */
   const static std::string  VERSION_ATTRIBUTE; // = "version";

  /**
   * Manifest header attribute identifying the symbolic name of a bundle that
   * exports a package specified in the Import-Package manifest header.
   *
   * <p>
   * The attribute value is encoded in the Import-Package manifest header
   * like:
   *
   * <pre>
   *     Import-Package: org.osgi.framework; bundle-symbolic-name; // =&quot;com.acme.module.test&quot;
   * </pre>
   *
   * @see #IMPORT_PACKAGE
   * @since 1.3
   */
   const static std::string  BUNDLE_SYMBOLICNAME_ATTRIBUTE; // = "bundle-symbolic-name";

  /**
   * Manifest header directive identifying the resolution type in the
   * Import-Package or Require-Bundle manifest header. The default value is
   * {@link #RESOLUTION_MANDATORY mandatory}.
   *
   * <p>
   * The directive value is encoded in the Import-Package or Require-Bundle
   * manifest header like:
   *
   * <pre>
   *     Import-Package: org.osgi.framework; resolution:; // =&quot;optional&quot;
   *     Require-Bundle: com.acme.module.test; resolution:; // =&quot;optional&quot;
   * </pre>
   *
   * @see #IMPORT_PACKAGE
   * @see #REQUIRE_BUNDLE
   * @see #RESOLUTION_MANDATORY
   * @see #RESOLUTION_OPTIONAL
   * @since 1.3
   */
   const static std::string  RESOLUTION_DIRECTIVE; // = "resolution";

  /**
   * Manifest header directive value identifying a mandatory resolution type.
   * A mandatory resolution type indicates that the import package or require
   * bundle must be resolved when the bundle is resolved. If such an import or
   * require bundle cannot be resolved, the module fails to resolve.
   *
   * <p>
   * The directive value is encoded in the Import-Package or Require-Bundle
   * manifest header like:
   *
   * <pre>
   *     Import-Package: org.osgi.framework; resolution:; // =&quot;manditory&quot;
   *     Require-Bundle: com.acme.module.test; resolution:; // =&quot;manditory&quot;
   * </pre>
   *
   * @see #RESOLUTION_DIRECTIVE
   * @since 1.3
   */
   const static std::string  RESOLUTION_MANDATORY; // = "mandatory";

  /**
   * Manifest header directive value identifying an optional resolution type.
   * An optional resolution type indicates that the import or require bundle
   * is optional and the bundle may be resolved without the import or require
   * bundle being resolved. If the import or require bundle is not resolved
   * when the bundle is resolved, the import or require bundle may not be
   * resolved before the bundle is refreshed.
   *
   * <p>
   * The directive value is encoded in the Import-Package or Require-Bundle
   * manifest header like:
   *
   * <pre>
   *     Import-Package: org.osgi.framework; resolution:; // =&quot;optional&quot;
   *     Require-Bundle: com.acme.module.test; resolution:; // =&quot;optional&quot;
   * </pre>
   *
   * @see #RESOLUTION_DIRECTIVE
   * @since 1.3
   */
   const static std::string  RESOLUTION_OPTIONAL; // = "optional";

  /**
   * Manifest header directive identifying a list of packages that an exported
   * package uses.
   *
   * <p>
   * The directive value is encoded in the Export-Package manifest header
   * like:
   *
   * <pre>
   *     Export-Package: org.osgi.util.tracker; uses:; // =&quot;org.osgi.framework&quot;
   * </pre>
   *
   * @see #EXPORT_PACKAGE
   * @since 1.3
   */
   const static std::string  USES_DIRECTIVE; // = "uses";

  /**
   * Manifest header directive identifying a list of classes to include in the
   * exported package.
   *
   * <p>
   * This directive is used by the Export-Package manifest header to identify
   * a list of classes of the specified package which must be allowed to be
   * exported. The directive value is encoded in the Export-Package manifest
   * header like:
   *
   * <pre>
   *     Export-Package: org.osgi.framework; include:; // =&quot;MyClass*&quot;
   * </pre>
   *
   * <p>
   * This directive is also used by the Bundle-ActivationPolicy manifest
   * header to identify the packages from which class loads will trigger lazy
   * activation. The directive value is encoded in the Bundle-ActivationPolicy
   * manifest header like:
   *
   * <pre>
   *     Bundle-ActivationPolicy: lazy; include:; // =&quot;org.osgi.framework&quot;
   * </pre>
   *
   * @see #EXPORT_PACKAGE
   * @see #BUNDLE_ACTIVATIONPOLICY
   * @since 1.3
   */
   const static std::string  INCLUDE_DIRECTIVE; // = "include";

  /**
   * Manifest header directive identifying a list of classes to exclude in the
   * exported package..
   * <p>
   * This directive is used by the Export-Package manifest header to identify
   * a list of classes of the specified package which must not be allowed to
   * be exported. The directive value is encoded in the Export-Package
   * manifest header like:
   *
   * <pre>
   *     Export-Package: org.osgi.framework; exclude:; // =&quot;*Impl&quot;
   * </pre>
   *
   * <p>
   * This directive is also used by the Bundle-ActivationPolicy manifest
   * header to identify the packages from which class loads will not trigger
   * lazy activation. The directive value is encoded in the
   * Bundle-ActivationPolicy manifest header like:
   *
   * <pre>
   *     Bundle-ActivationPolicy: lazy; exclude:; // =&quot;org.osgi.framework&quot;
   * </pre>
   *
   * @see #EXPORT_PACKAGE
   * @see #BUNDLE_ACTIVATIONPOLICY
   * @since 1.3
   */
   const static std::string  EXCLUDE_DIRECTIVE; // = "exclude";

  /**
   * Manifest header directive identifying names of matching attributes which
   * must be specified by matching Import-Package statements in the
   * Export-Package manifest header.
   *
   * <p>
   * The directive value is encoded in the Export-Package manifest header
   * like:
   *
   * <pre>
   *     Export-Package: org.osgi.framework; mandatory:; // =&quot;bundle-symbolic-name&quot;
   * </pre>
   *
   * @see #EXPORT_PACKAGE
   * @since 1.3
   */
   const static std::string  MANDATORY_DIRECTIVE; // = "mandatory";

  /**
   * Manifest header directive identifying the visibility of a required bundle
   * in the Require-Bundle manifest header. The default value is
   * {@link #VISIBILITY_PRIVATE private}.
   *
   * <p>
   * The directive value is encoded in the Require-Bundle manifest header
   * like:
   *
   * <pre>
   *     Require-Bundle: com.acme.module.test; visibility:; // =&quot;reexport&quot;
   * </pre>
   *
   * @see #REQUIRE_BUNDLE
   * @see #VISIBILITY_PRIVATE
   * @see #VISIBILITY_REEXPORT
   * @since 1.3
   */
   const static std::string  VISIBILITY_DIRECTIVE; // = "visibility";

  /**
   * Manifest header directive value identifying a private visibility type. A
   * private visibility type indicates that any packages that are exported by
   * the required bundle are not made visible on the export signature of the
   * requiring bundle.
   *
   * <p>
   * The directive value is encoded in the Require-Bundle manifest header
   * like:
   *
   * <pre>
   *     Require-Bundle: com.acme.module.test; visibility:; // =&quot;private&quot;
   * </pre>
   *
   * @see #VISIBILITY_DIRECTIVE
   * @since 1.3
   */
   const static std::string  VISIBILITY_PRIVATE; // = "private";

  /**
   * Manifest header directive value identifying a reexport visibility type. A
   * reexport visibility type indicates any packages that are exported by the
   * required bundle are re-exported by the requiring bundle. Any arbitrary
   * arbitrary matching attributes with which they were exported by the
   * required bundle are deleted.
   *
   * <p>
   * The directive value is encoded in the Require-Bundle manifest header
   * like:
   *
   * <pre>
   *     Require-Bundle: com.acme.module.test; visibility:; // =&quot;reexport&quot;
   * </pre>
   *
   * @see #VISIBILITY_DIRECTIVE
   * @since 1.3
   */
   const static std::string  VISIBILITY_REEXPORT; // = "reexport";

  /**
   * Manifest header directive identifying the type of the extension fragment.
   *
   * <p>
   * The directive value is encoded in the Fragment-Host manifest header like:
   *
   * <pre>
   *     Fragment-Host: system.bundle; extension:; // =&quot;framework&quot;
   * </pre>
   *
   * @see #FRAGMENT_HOST
   * @see #EXTENSION_FRAMEWORK
   * @see #EXTENSION_BOOTCLASSPATH
   * @since 1.3
   */
   const static std::string  EXTENSION_DIRECTIVE; // = "extension";

  /**
   * Manifest header directive value identifying the type of extension
   * fragment. An extension fragment type of framework indicates that the
   * extension fragment is to be loaded by the framework's class loader.
   *
   * <p>
   * The directive value is encoded in the Fragment-Host manifest header like:
   *
   * <pre>
   *     Fragment-Host: system.bundle; extension:; // =&quot;framework&quot;
   * </pre>
   *
   * @see #EXTENSION_DIRECTIVE
   * @since 1.3
   */
   const static std::string  EXTENSION_FRAMEWORK; // = "framework";

  /**
   * Manifest header directive value identifying the type of extension
   * fragment. An extension fragment type of bootclasspath indicates that the
   * extension fragment is to be loaded by the boot class loader.
   *
   * <p>
   * The directive value is encoded in the Fragment-Host manifest header like:
   *
   * <pre>
   *     Fragment-Host: system.bundle; extension:; // =&quot;bootclasspath&quot;
   * </pre>
   *
   * @see #EXTENSION_DIRECTIVE
   * @since 1.3
   */
   const static std::string  EXTENSION_BOOTCLASSPATH; // = "bootclasspath";

  /**
   * Manifest header identifying the bundle's activation policy.
   * <p>
   * The attribute value may be retrieved from the <code>Dictionary</code>
   * object returned by the <code>Bundle.getHeaders</code> method.
   *
   * @since 1.4
   * @see #ACTIVATION_LAZY
   * @see #INCLUDE_DIRECTIVE
   * @see #EXCLUDE_DIRECTIVE
   */
   const static std::string  BUNDLE_ACTIVATIONPOLICY; // = "Bundle-ActivationPolicy";

  /**
   * Bundle activation policy declaring the bundle must be activated when the
   * first class load is made from the bundle.
   * <p>
   * A bundle with the lazy activation policy that is started with the
   * {@link Bundle#START_ACTIVATION_POLICY START_ACTIVATION_POLICY} option
   * will wait in the {@link Bundle#STARTING STARTING} state until the first
   * class load from the bundle occurs. The bundle will then be activated
   * before the class is returned to the requester.
   * <p>
   * The activation policy value is specified as in the
   * Bundle-ActivationPolicy manifest header like:
   *
   * <pre>
   *       Bundle-ActivationPolicy: lazy
   * </pre>
   *
   * @see #BUNDLE_ACTIVATIONPOLICY
   * @see Bundle#start(int)
   * @see Bundle#START_ACTIVATION_POLICY
   * @since 1.4
   */
   const static std::string  ACTIVATION_LAZY; // = "lazy";

  /**
   * Framework environment property identifying the Framework version.
   *
   * <p>
   * The value of this property may be retrieved by calling the
   * <code>BundleContext.getProperty</code> method.
   */
   static const std::string  FRAMEWORK_VERSION; // = "org.osgi.framework.version";

  /**
   * Framework environment property identifying the Framework implementation
   * vendor.
   *
   * <p>
   * The value of this property may be retrieved by calling the
   * <code>BundleContext.getProperty</code> method.
   */
   static const std::string  FRAMEWORK_VENDOR; // = "org.osgi.framework.vendor";

  /**
   * Framework environment property identifying the Framework implementation
   * language (see ISO 639 for possible values).
   *
   * <p>
   * The value of this property may be retrieved by calling the
   * <code>BundleContext.getProperty</code> method.
   */
   static const std::string  FRAMEWORK_LANGUAGE; // = "org.osgi.framework.language";

  /**
   * Framework environment property identifying the Framework host-computer's
   * operating system.
   *
   * <p>
   * The value of this property may be retrieved by calling the
   * <code>BundleContext.getProperty</code> method.
   */
   static const std::string  FRAMEWORK_OS_NAME; // = "org.osgi.framework.os.name";

  /**
   * Framework environment property identifying the Framework host-computer's
   * operating system version number.
   *
   * <p>
   * The value of this property may be retrieved by calling the
   * <code>BundleContext.getProperty</code> method.
   */
   static const std::string  FRAMEWORK_OS_VERSION; // = "org.osgi.framework.os.version";

  /**
   * Framework environment property identifying the Framework host-computer's
   * processor name.
   *
   * <p>
   * The value of this property may be retrieved by calling the
   * <code>BundleContext.getProperty</code> method.
   */
   static const std::string  FRAMEWORK_PROCESSOR; // = "org.osgi.framework.processor";

  /**
   * Framework environment property identifying execution environments
   * provided by the Framework.
   *
   * <p>
   * The value of this property may be retrieved by calling the
   * <code>BundleContext.getProperty</code> method.
   *
   * @since 1.2
   */
   static const std::string  FRAMEWORK_EXECUTIONENVIRONMENT; // = "org.osgi.framework.executionenvironment";

  /**
   * Framework environment property identifying packages for which the
   * Framework must delegate class loading to the parent class loader of the
   * bundle.
   *
   * <p>
   * The value of this property may be retrieved by calling the
   * <code>BundleContext.getProperty</code> method.
   *
   * @see #FRAMEWORK_BUNDLE_PARENT
   * @since 1.3
   */
   static const std::string  FRAMEWORK_BOOTDELEGATION; // = "org.osgi.framework.bootdelegation";

  /**
   * Framework environment property identifying packages which the system
   * bundle must export.
   *
   * <p>
   * If this property is not specified then the framework must calculate a
   * reasonable default value for the current execution environment.
   *
   * <p>
   * The value of this property may be retrieved by calling the
   * <code>BundleContext.getProperty</code> method.
   *
   * @since 1.3
   */
   static const std::string  FRAMEWORK_SYSTEMPACKAGES; // = "org.osgi.framework.system.packages";

  /**
   * Framework environment property identifying extra packages which the
   * system bundle must export from the current execution environment.
   *
   * <p>
   * This property is useful for configuring extra system packages in addition
   * to the system packages calculated by the framework.
   *
   * <p>
   * The value of this property may be retrieved by calling the
   * <code>BundleContext.getProperty</code> method.
   *
   * @see #FRAMEWORK_SYSTEMPACKAGES
   * @since 1.5
   */
   static const std::string  FRAMEWORK_SYSTEMPACKAGES_EXTRA; // = "org.osgi.framework.system.packages.extra";

  /**
   * Framework environment property identifying whether the Framework supports
   * framework extension bundles.
   *
   * <p>
   * As of version 1.4, the value of this property must be <code>true</code>.
   * The Framework must support framework extension bundles.
   *
   * <p>
   * The value of this property may be retrieved by calling the
   * <code>BundleContext.getProperty</code> method.
   *
   * @since 1.3
   */
   static const std::string  SUPPORTS_FRAMEWORK_EXTENSION; // = "org.osgi.supports.framework.extension";

  /**
   * Framework environment property identifying whether the Framework supports
   * bootclasspath extension bundles.
   *
   * <p>
   * If the value of this property is <code>true</code>, then the Framework
   * supports bootclasspath extension bundles. The default value is
   * <code>false</code>.
   * <p>
   * The value of this property may be retrieved by calling the
   * <code>BundleContext.getProperty</code> method.
   *
   * @since 1.3
   */
   static const std::string  SUPPORTS_BOOTCLASSPATH_EXTENSION; // = "org.osgi.supports.bootclasspath.extension";

  /**
   * Framework environment property identifying whether the Framework supports
   * fragment bundles.
   *
   * <p>
   * As of version 1.4, the value of this property must be <code>true</code>.
   * The Framework must support fragment bundles.
   * <p>
   * The value of this property may be retrieved by calling the
   * <code>BundleContext.getProperty</code> method.
   *
   * @since 1.3
   */
   static const std::string  SUPPORTS_FRAMEWORK_FRAGMENT; // = "org.osgi.supports.framework.fragment";

  /**
   * Framework environment property identifying whether the Framework supports
   * the {@link #REQUIRE_BUNDLE Require-Bundle} manifest header.
   *
   * <p>
   * As of version 1.4, the value of this property must be <code>true</code>.
   * The Framework must support the <code>Require-Bundle</code> manifest
   * header.
   * <p>
   * The value of this property may be retrieved by calling the
   * <code>BundleContext.getProperty</code> method.
   *
   * @since 1.3
   */
   static const std::string  SUPPORTS_FRAMEWORK_REQUIREBUNDLE; // = "org.osgi.supports.framework.requirebundle";

  /**
   * Specifies the type of security manager the framework must use. If not
   * specified then the framework will not set the VM security manager.
   *
   * @see #FRAMEWORK_SECURITY_OSGI
   * @since 1.5
   */
   const static std::string  FRAMEWORK_SECURITY; // = "org.osgi.framework.security";

  /**
   * Specifies that a security manager that supports all security aspects of
   * the OSGi core specification including postponed conditions must be
   * installed.
   *
   * <p>
   * If this value is specified and there is a security manager already
   * installed, then a <code>SecurityException</code> must be thrown when the
   * Framework is initialized.
   *
   * @see #FRAMEWORK_SECURITY
   * @since 1.5
   */
   const static std::string  FRAMEWORK_SECURITY_OSGI; // = "osgi";

  /**
   * Specified the persistent storage area used by the framework. The value of
   * this property must be a valid file path in the file system to a
   * directory. If the specified directory does not exist then the framework
   * will create the directory. If the specified path exists but is not a
   * directory or if the framework fails to create the storage directory, then
   * framework initialization must fail. The framework is free to use this
   * directory as it sees fit. This area can not be shared with anything else.
   * <p>
   * If this property is not set, the framework should use a reasonable
   * platform default for the persistent storage area.
   *
   * @since 1.5
   */
   const static std::string  FRAMEWORK_STORAGE; // = "org.osgi.framework.storage";

  /**
   * Specifies if and when the persistent storage area for the framework
   * should be cleaned. If this property is not set, then the framework
   * storage area must not be cleaned.
   *
   * @see #FRAMEWORK_STORAGE_CLEAN_ONFIRSTINIT
   * @since 1.5
   */
   const static std::string  FRAMEWORK_STORAGE_CLEAN; // = "org.osgi.framework.storage.clean";

  /**
   * Specifies that the framework storage area must be cleaned before the
   * framework is initialized for the first time. Subsequent inits, starts or
   * updates of the framework will not result in cleaning the framework
   * storage area.
   *
   * @since 1.5
   */
   const static std::string  FRAMEWORK_STORAGE_CLEAN_ONFIRSTINIT; // = "onFirstInit";

  /**
   * Specifies a comma separated list of additional library file extensions
   * that must be used when a bundle's class loader is searching for native
   * libraries. If this property is not set, then only the library name
   * returned by <code>System.mapLibraryName(std::string)</code> will be used to
   * search. This is needed for certain operating systems which allow more
   * than one extension for a library. For example, AIX allows library
   * extensions of <code>.a</code> and <code>.so</code>, but
   * <code>System.mapLibraryName(std::string)</code> will only return names with
   * the <code>.a</code> extension.
   *
   * @since 1.5
   */
   const static std::string  FRAMEWORK_LIBRARY_EXTENSIONS; // = "org.osgi.framework.library.extensions";

  /**
   * Specifies an optional OS specific command to set file permissions on
   * extracted native code. On some operating systems, it is required that
   * native libraries be set to executable. This optional property allows you
   * to specify the command. For example, on a UNIX style OS, this property
   * could have the following value.
   *
   * <pre>
   * chmod +rx ${abspath}
   * </pre>
   *
   * The <code>${abspath}</code> is used by the framework to substitute the
   * actual absolute file path.
   *
   * @since 1.5
   */
   const static std::string  FRAMEWORK_EXECPERMISSION; // = "org.osgi.framework.command.execpermission";

  /**
   * Specifies the trust repositories used by the framework. The value is a
   * <code>java.io.File.pathSeparator</code> separated list of valid file
   * paths to files that contain key stores of type <code>JKS</code>. The
   * framework will use the key stores as trust repositories to authenticate
   * certificates of trusted signers. The key stores are only used as
   * read-only trust repositories to access  keys. No passwords are
   * required to access the key stores'  keys.
   * <p>
   * Note that framework implementations are allowed to use other trust
   * repositories in addition to the trust repositories specified by this
   * property. How these other trust repositories are configured and populated
   * is implementation specific.
   *
   * @since 1.5
   */
   const static std::string  FRAMEWORK_TRUST_REPOSITORIES; // = "org.osgi.framework.trust.repositories";

  /**
   * Specifies the current windowing system. The framework should provide a
   * reasonable default if this is not set.
   *
   * @since 1.5
   */
   const static std::string  FRAMEWORK_WINDOWSYSTEM; // = "org.osgi.framework.windowsystem";

  /**
   * Specifies the beginning start level of the framework.
   *
   * @see "Core Specification, section 8.2.3."
   * @since 1.5
   */
   const static std::string  FRAMEWORK_BEGINNING_STARTLEVEL; // = "org.osgi.framework.startlevel.beginning";

  /**
   * Specifies the parent class loader type for all bundle class loaders.
   * Default value is {@link #FRAMEWORK_BUNDLE_PARENT_BOOT boot}.
   *
   * @see #FRAMEWORK_BUNDLE_PARENT_BOOT
   * @see #FRAMEWORK_BUNDLE_PARENT_EXT
   * @see #FRAMEWORK_BUNDLE_PARENT_APP
   * @see #FRAMEWORK_BUNDLE_PARENT_FRAMEWORK
   * @since 1.5
   */
   const static std::string FRAMEWORK_BUNDLE_PARENT; // = "org.osgi.framework.bundle.parent";

  /**
   * Specifies to use of the boot class loader as the parent class loader for
   * all bundle class loaders.
   *
   * @since 1.5
   * @see #FRAMEWORK_BUNDLE_PARENT
   */
   const static std::string  FRAMEWORK_BUNDLE_PARENT_BOOT; // = "boot";

  /**
   * Specifies to use the extension class loader as the parent class loader
   * for all bundle class loaders.
   *
   * @since 1.5
   * @see #FRAMEWORK_BUNDLE_PARENT
   */
   const static std::string  FRAMEWORK_BUNDLE_PARENT_EXT; // = "ext";

  /**
   * Specifies to use the application class loader as the parent class loader
   * for all bundle class loaders.  Depending on how the framework is
   * launched, this may refer to the same class loader as
   * {@link #FRAMEWORK_BUNDLE_PARENT_FRAMEWORK}.
   *
   * @since 1.5
   * @see #FRAMEWORK_BUNDLE_PARENT
   */
   const static std::string  FRAMEWORK_BUNDLE_PARENT_APP; // = "app";

  /**
   * Specifies to use the framework class loader as the parent class loader
   * for all bundle class loaders. The framework class loader is the class
   * loader used to load the framework implementation.  Depending on how the
   * framework is launched, this may refer to the same class loader as
   * {@link #FRAMEWORK_BUNDLE_PARENT_APP}.
   *
   * @since 1.5
   * @see #FRAMEWORK_BUNDLE_PARENT
   */
   const static std::string  FRAMEWORK_BUNDLE_PARENT_FRAMEWORK; // = "framework";

  /*
   * Service properties.
   */

  /**
   * Service property identifying all of the class names under which a service
   * was registered in the Framework. The value of this property must be of
   * type <code>std::string[]</code>.
   *
   * <p>
   * This property is set by the Framework when a service is registered.
   */
   static const std::string  OBJECTCLASS; // = "objectClass";

  /**
   * Service property identifying a service's registration number. The value
   * of this property must be of type <code>Long</code>.
   *
   * <p>
   * The value of this property is assigned by the Framework when a service is
   * registered. The Framework assigns a unique value that is larger than all
   * previously assigned values since the Framework was started. These values
   * are NOT persistent across restarts of the Framework.
   */
   static const std::string  SERVICE_ID; // = "service.id";

  /**
   * Service property identifying a service's persistent identifier.
   *
   * <p>
   * This property may be supplied in the <code>properties</code>
   * <code>Dictionary</code> object passed to the
   * <code>BundleContext.registerService</code> method. The value of this
   * property must be of type <code>std::string</code>, <code>std::string[]</code>, or
   * <code>Collection</code> of <code>std::string</code>.
   *
   * <p>
   * A service's persistent identifier uniquely identifies the service and
   * persists across multiple Framework invocations.
   *
   * <p>
   * By convention, every bundle has its own unique namespace, starting with
   * the bundle's identifier (see {@link Bundle#getBundleId}) and followed by
   * a dot (.). A bundle may use this as the prefix of the persistent
   * identifiers for the services it registers.
   */
   static const std::string  SERVICE_PID; // = "service.pid";

  /**
   * Service property identifying a service's ranking number.
   *
   * <p>
   * This property may be supplied in the <code>properties
   * Dictionary</code> object passed to the
   * <code>BundleContext.registerService</code> method. The value of this
   * property must be of type <code>Integer</code>.
   *
   * <p>
   * The service ranking is used by the Framework to determine the <i>natural
   * order</i> of services, see {@link ServiceReference#compareTo(Object)},
   * and the <i>default</i> service to be returned from a call to the
   * {@link BundleContext#getServiceReference} method.
   *
   * <p>
   * The default ranking is zero (0). A service with a ranking of
   * <code>Integer.MAX_VALUE</code> is very likely to be returned as the
   * default service, whereas a service with a ranking of
   * <code>Integer.MIN_VALUE</code> is very unlikely to be returned.
   *
   * <p>
   * If the supplied property value is not of type <code>Integer</code>, it is
   * deemed to have a ranking value of zero.
   */
   static const std::string  SERVICE_RANKING; // = "service.ranking";

  /**
   * Service property identifying a service's vendor.
   *
   * <p>
   * This property may be supplied in the properties <code>Dictionary</code>
   * object passed to the <code>BundleContext.registerService</code> method.
   */
   static const std::string  SERVICE_VENDOR; // = "service.vendor";

  /**
   * Service property identifying a service's description.
   *
   * <p>
   * This property may be supplied in the properties <code>Dictionary</code>
   * object passed to the <code>BundleContext.registerService</code> method.
   */
   static const std::string  SERVICE_DESCRIPTION; // = "service.description";
};

}
}

#endif /* CONSTANTS_H_ */
