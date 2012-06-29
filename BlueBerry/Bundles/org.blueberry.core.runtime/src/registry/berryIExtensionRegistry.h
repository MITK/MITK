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


#ifndef BERRYIEXTENSIONREGISTRY_H
#define BERRYIEXTENSIONREGISTRY_H

#include <berrySmartPointer.h>

#include <QList>

namespace berry {

struct IConfigurationElement;
struct IExtension;
struct IExtensionPoint;

/**
 * The extension registry holds the master list of all
 * discovered namespaces, extension points and extensions.
 * <p>
 * The extension registry can be queried, by name, for
 * extension points and extensions.
 * </p>
 * <p>
 * The various objects that describe the contents of the extension registry
 * ({@link IExtensionPoint}, {@link IExtension}, and {@link IConfigurationElement})
 * are intended for relatively short-term use. Clients that deal with these objects
 * must be aware that they may become invalid if the declaring plug-in is updated
 * or uninstalled. If this happens, all methods on these object except
 * <code>isValid()</code> will throw {@link org.eclipse.core.runtime.InvalidRegistryObjectException}.
 * Code in a plug-in that has declared that it is not dynamic aware (or not declared
 * anything) can safely ignore this issue, since the registry would not be
 * modified while it is active. However, code in a plug-in that declares that it
 * is dynamic aware must be careful if it accesses extension registry objects,
 * because it's at risk if plug-in are removed. Similarly, tools that analyze
 * or display the extension registry are vulnerable. Client code can pre-test for
 * invalid objects by calling <code>isValid()</code>, which never throws this exception.
 * However, pre-tests are usually not sufficient because of the possibility of the
 * extension registry object becoming invalid as a result of a concurrent activity.
 * At-risk clients must treat <code>InvalidRegistryObjectException</code> as if it
 * were a checked exception. Also, such clients should probably register a listener
 * with the extension registry so that they receive notification of any changes to
 * the registry.
 * </p>
 * <p>
 * Extensions and extension points are declared by generic entities called
 * <cite>namespaces</cite>. The only fact known about namespaces is that they
 * have unique string-based identifiers. One example of a namespace
 * is a plug-in, for which the namespace id is the plug-in id.
 * </p><p>
 * This interface is not intended to be implemented by clients.
 * </p>
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct IExtensionRegistry
{

  virtual ~IExtensionRegistry();

  /**
   * Returns all configuration elements from all extensions configured
   * into the identified extension point. Returns an empty list if the extension
   * point does not exist, has no extensions configured, or none of the extensions
   * contain configuration elements.
   *
   * @param extensionPointId the unique identifier of the extension point
   *    (e.g. <code>"org.blueberry.core.applications"</code>)
   * @return the configuration elements
   */
  virtual QList<SmartPointer<IConfigurationElement> > GetConfigurationElementsFor(
      const QString& extensionPointId) const = 0;

  /**
   * Returns all configuration elements from all extensions configured
   * into the identified extension point. Returns an empty list if the extension
   * point does not exist, has no extensions configured, or none of the extensions
   * contain configuration elements.
   *
   * @param namespace the namespace for the extension point
   *    (e.g. <code>"org.eclipse.core.resources"</code>)
   * @param extensionPointName the simple identifier of the
   *    extension point (e.g. <code>"builders"</code>)
   * @return the configuration elements
   */
  virtual QList<SmartPointer<IConfigurationElement> > GetConfigurationElementsFor(
      const QString& namespaze, const QString& extensionPointName) const = 0;

  /**
   * Returns all configuration elements from the identified extension.
   * Returns an empty array if the extension does not exist or
   * contains no configuration elements.
   *
   * @param namespace the namespace for the extension point
   *    (e.g. <code>"org.eclipse.core.resources"</code>)
   * @param extensionPointName the simple identifier of the
   *    extension point (e.g. <code>"builders"</code>)
   * @param extensionId the unique identifier of the extension
   *    (e.g. <code>"com.example.acme.coolbuilder"</code>)
   * @return the configuration elements
   */
  virtual QList<SmartPointer<IConfigurationElement> > GetConfigurationElementsFor(
      const QString& namespaze, const QString& extensionPointName, const QString& extensionId) const = 0;

  /**
   * Returns the specified extension in this extension registry,
   * or <code>null</code> if there is no such extension.
   *
   * @param extensionId the unique identifier of the extension
   *    (e.g. <code>"com.example.acme.coolbuilder"</code>)
   * @return the extension, or <code>null</code>
   */
  virtual SmartPointer<IExtension> GetExtension(const QString& extensionId) const = 0;

  /**
   * Returns the specified extension in this extension registry,
   * or <code>null</code> if there is no such extension.
   * The first parameter identifies the extension point, and the second
   * parameter identifies an extension plugged in to that extension point.
   *
   * @param extensionPointId the unique identifier of the extension point
   *    (e.g. <code>"org.eclipse.core.resources.builders"</code>)
   * @param extensionId the unique identifier of the extension
   *    (e.g. <code>"com.example.acme.coolbuilder"</code>)
   * @return the extension, or <code>null</code>
   */
  virtual SmartPointer<IExtension> GetExtension(const QString& extensionPointId,
                                                const QString& extensionId) const = 0;

  /**
   * Returns the specified extension in this extension registry,
   * or <code>null</code> if there is no such extension.
   * The first two parameters identify the extension point, and the third
   * parameter identifies an extension plugged in to that extension point.
   *
   * @param namespace the namespace for the extension point
   *    (e.g. <code>"org.eclipse.core.resources"</code>)
   * @param extensionPointName the simple identifier of the
   *    extension point (e.g. <code>"builders"</code>)
   * @param extensionId the unique identifier of the extension
   *    (e.g. <code>"com.example.acme.coolbuilder"</code>)
   * @return the extension, or <code>null</code>
   */
  virtual SmartPointer<IExtension> GetExtension(const QString& namespaze,
                                                const QString& extensionPointName,
                                                const QString& extensionId) const = 0;

  /**
   * Returns the extension point with the given extension point identifier
   * in this extension registry, or <code>null</code> if there is no such
   * extension point.
   *
   * @param extensionPointId the unique identifier of the extension point
   *    (e.g., <code>"org.blueberry.core.applications"</code>)
   * @return the extension point, or <code>null</code>
   */
  virtual SmartPointer<IExtensionPoint> GetExtensionPoint(const QString& extensionPointId) const = 0;

  /**
   * Returns the extension point in this extension registry
   * with the given namespace and extension point simple identifier,
   * or <code>null</code> if there is no such extension point.
   *
   * @param namespace the namespace for the given extension point
   *    (e.g. <code>"org.eclipse.core.resources"</code>)
   * @param extensionPointName the simple identifier of the
   *    extension point (e.g. <code>"builders"</code>)
   * @return the extension point, or <code>null</code>
   */
  virtual SmartPointer<IExtensionPoint> GetExtensionPoint(const QString& namespaze,
                                                          const QString& extensionPointName) const = 0;

  /**
   * Returns all extension points known to this extension registry.
   * Returns an empty array if there are no extension points.
   *
   * @return the extension points known to this extension registry
   */
  virtual QList<SmartPointer<IExtensionPoint> > GetExtensionPoints() const = 0;

  /**
   * Returns all extension points declared in the given namespace. Returns an empty array if
   * there are no extension points declared in the namespace.
   *
   * @param namespace the namespace for the extension points
   *    (e.g. <code>"org.eclipse.core.resources"</code>)
   * @return the extension points in this registry declared in the given namespace
   */
  virtual QList<SmartPointer<IExtensionPoint> > GetExtensionPoints(const QString& namespaze) const = 0;

  /**
   * Returns all extension points supplied by the contributor, or <code>null</code>
   * if there are no such extension points.
   *
   * @param contributor the contributor for the extensions (for OSGi registry, bundles and
   * fragments are different contributors)
   * @return the extension points, or <code>null</code>
   * @since 3.4
   */
  virtual QList<SmartPointer<IExtensionPoint> > GetExtensionPointsFromContributor(
      const QString& contributor) const = 0;

  /**
   * Returns all extensions declared in the given namespace. Returns an empty array if
   * no extensions are declared in the namespace.
   *
   * @param namespace the namespace for the extensions
   *    (e.g. <code>"org.eclipse.core.resources"</code>)
   * @return the extensions in this registry declared in the given namespace
   */
  virtual QList<SmartPointer<IExtension> > GetExtensions(const QString& namespaze) const = 0;

  /**
   * Returns all extensions supplied by the contributor, or <code>null</code> if there
   * are no such extensions.
   * @param contributor the contributor for the extensions (for OSGi registry, bundles and
   * fragments are different contributors)
   * @return the extensions, or <code>null</code>
   */
  virtual QList<SmartPointer<IExtension> > GetExtensionsFromContributor(const QString& contributor) const = 0;

  /**
   * Returns all namespaces currently used by extensions and extension points in this
   * registry. Returns an empty array if there are no known extensions/extension points
   * in this registry.
   * <p>
   * The fully-qualified name of an extension point or an extension consist of
   * a namespace and a simple name (much like a qualified Java class name consist
   * of a package name and a class name). The simple names are presumed to be unique
   * in the namespace.
   * </p>
   * @return all namespaces known to this registry
   */
  virtual QList<QString> GetNamespaces() const = 0;

};

}

Q_DECLARE_INTERFACE(berry::IExtensionRegistry, "org.blueberry.service.IExtensionRegistry")

#endif // BERRYIEXTENSIONREGISTRY_H
