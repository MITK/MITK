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

#ifndef BERRYIEXTENSIONELEMENT_H_
#define BERRYIEXTENSIONELEMENT_H_

#include <berryObject.h>

#include <berryLog.h>

namespace berry {

struct IContributor;
struct IExtension;

class Handle;

/**
 * A configuration element, with its attributes and children,
 * directly reflects the content and structure of the extension section
 * within the declaring plug-in's manifest (<code>plugin.xml</code>) file.
 * <p>
 * This interface also provides a way to create executable extension
 * objects.
 * </p>
 * <p>
 * These registry objects are intended for relatively short-term use. Clients that
 * deal with these objects must be aware that they may become invalid if the
 * declaring plug-in is updated or uninstalled. If this happens, all methods except
 * {@link #IsValid()} will throw {@link InvalidRegistryObjectException}.
 * For configuration element objects, the most common case is code in a plug-in dealing
 * with extensions contributed to one of the extension points it declares.
 * Code in a plug-in that has declared that it is not dynamic aware (or not
 * declared anything) can safely ignore this issue, since the registry
 * would not be modified while it is active. However, code in a plug-in that
 * declares that it is dynamic aware must be careful when accessing the extension
 * and configuration element objects because they become invalid if the contributing
 * plug-in is removed. Similarly, tools that analyze or display the extension registry
 * are vulnerable. Client code can pre-test for invalid objects by calling {@link #IsValid()},
 * which never throws this exception. However, pre-tests are usually not sufficient
 * because of the possibility of the extension or configuration element object becoming
 * invalid as a result of a concurrent activity. At-risk clients must treat
 * <code>InvalidRegistryObjectException</code> as if it were a checked exception.
 * Also, such clients should probably register a listener with the extension registry
 * so that they receive notification of any changes to the registry.
 * </p><p>
 * This interface is not intended to be implemented by clients.
 * </p>
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct org_blueberry_core_runtime_EXPORT IConfigurationElement : public virtual Object
{

  berryObjectMacro(berry::IConfigurationElement)

  ~IConfigurationElement() override;

  /**
   * Creates and returns a new instance of the executable extension
   * identified by the named attribute of this configuration element.
   * The named attribute value must contain a fully qualified name
   * of a class. The class can either refer to a class implementing
   * the executable extension or to a factory capable of returning the
   * executable extension.
   * <p>
   * The specified class is instantiated using its 0-argument public constructor.
   * <p>
   * Then the following checks are done:<br>
   * If the specified class implements the {@link IExecutableExtension}
   * interface, the method {@link IExecutableExtension#SetInitializationData(IConfigurationElement, QString, Object*)}
   * is called, passing to the object the configuration information that was used to create it.
   * <p>
   * If the specified class implements {@link IExecutableExtensionFactory}
   * interface, the method {@link IExecutableExtensionFactory#Create()}
   * is invoked.
   * </p>
   * <p>
   * Unlike other methods on this object, invoking this method may activate
   * the plug-in.
   * </p>
   *
   * @param propertyName the name of the property
   * @return the executable instance
   * @exception CoreException if an instance of the executable extension
   *   could not be created for any reason
   * @see IExecutableExtension#SetInitializationData(IConfigurationElement, QString, Object*)
   * @see IExecutableExtensionFactory
   * @throws InvalidRegistryObjectException if this configuration element is no longer valid
   */
  virtual QObject* CreateExecutableExtension(const QString& propertyName) const = 0;

  template<class C>
  C* CreateExecutableExtension(const QString &propertyName) const
  {
    C* interface = qobject_cast<C*>(this->CreateExecutableExtension(propertyName));
    if (interface == nullptr)
    {
      BERRY_WARN << "The QObject subclass " << this->GetAttribute(propertyName).toStdString()
                 << " does not seem to implement the required interface \""
                 << qobject_interface_iid<C*>() << "\", or you forgot the Q_INTERFACES macro.";
    }
    return interface;
  }

  /**
   * Returns the named attribute of this configuration element, or
   * <code>null</code> if none.
   * <p>
   * The names of configuration element attributes
   * are the same as the attribute names of the corresponding XML element.
   * For example, the configuration markup
   * <pre>
   * &lt;bg pattern="stripes"/&gt;
   * </pre>
   * corresponds to a configuration element named <code>"bg"</code>
   * with an attribute named <code>"pattern"</code>
   * with attribute value <code>"stripes"</code>.
   * </p>
   * <p> Note that any translation specified in the plug-in manifest
   * file is automatically applied.
   * </p>
   *
   * @param name the name of the attribute
   * @return attribute value, or <code>null</code> if none
   * @throws InvalidRegistryObjectException if this configuration element is no longer valid
   */
  virtual QString GetAttribute(const QString& name) const = 0;

  /**
   * Returns the names of the attributes of this configuration element.
   * Returns an empty list if this configuration element has no attributes.
   * <p>
   * The names of configuration element attributes
   * are the same as the attribute names of the corresponding XML element.
   * For example, the configuration markup
   * <pre>
   * &lt;bg color="blue" pattern="stripes"/&gt;
   * </pre>
   * corresponds to a configuration element named <code>"bg"</code>
   * with attributes named <code>"color"</code>
   * and <code>"pattern"</code>.
   * </p>
   *
   * @return the names of the attributes
   * @throws InvalidRegistryObjectException if this configuration element is no longer valid
   */
  virtual QList<QString> GetAttributeNames() const = 0;

  /**
   * Returns all configuration elements that are children of this
   * configuration element.
   * Returns an empty list if this configuration element has no children.
   * <p>
   * Each child corresponds to a nested
   * XML element in the configuration markup.
   * For example, the configuration markup
   * <pre>
   * &lt;view&gt;
   * &nbsp&nbsp&nbsp&nbsp&lt;verticalHint&gt;top&lt;/verticalHint&gt;
   * &nbsp&nbsp&nbsp&nbsp&lt;horizontalHint&gt;left&lt;/horizontalHint&gt;
   * &lt;/view&gt;
   * </pre>
   * corresponds to a configuration element, named <code>"view"</code>,
   * with two children.
   * </p>
   *
   * @return the child configuration elements
   * @throws InvalidRegistryObjectException if this configuration element is no longer valid
   * @see #getChildren(String)
   */
  virtual QList<IConfigurationElement::Pointer> GetChildren() const = 0;

  /**
   * Returns all child configuration elements with the given name.
   * Returns an empty list if this configuration element has no children
   * with the given name.
   *
   * @param name the name of the child configuration element
   * @return the child configuration elements with that name
   * @throws InvalidRegistryObjectException if this configuration element is no longer valid
   * @see #getChildren()
   */
  virtual QList<IConfigurationElement::Pointer> GetChildren(const QString& name) const = 0;

  /**
   * Returns the extension that declares this configuration element.
   *
   * @return the extension
   * @throws InvalidRegistryObjectException if this configuration element is no longer valid
   */
  virtual SmartPointer<IExtension> GetDeclaringExtension() const = 0;

  /**
   * Returns the name of this configuration element.
   * The name of a configuration element is the same as
   * the XML tag of the corresponding XML element.
   * For example, the configuration markup
   * <pre>
   * &lt;wizard name="Create Project"/&gt;
   * </pre>
   * corresponds to a configuration element named <code>"wizard"</code>.
   *
   * @return the name of this configuration element
   * @throws InvalidRegistryObjectException if this configuration element is no longer valid
   */
  virtual QString GetName() const = 0;

  /**
   * Returns the element which contains this element. If this element
   * is an immediate child of an extension, the
   * returned value can be downcast to <code>IExtension</code>.
   * Otherwise the returned value can be downcast to
   * <code>IConfigurationElement</code>.
   *
   * @return the parent of this configuration element
   *  or <code>null</code>
   * @throws InvalidRegistryObjectException if this configuration element is no longer valid
   */
  virtual SmartPointer<Object> GetParent() const = 0;

  /**
   * Returns the text value of this configuration element.
   * For example, the configuration markup
   * <pre>
   * &lt;script lang="javascript"&gt;.\scripts\cp.js&lt;/script&gt;
   * </pre>
   * corresponds to a configuration element <code>"script"</code>
   * with value <code>".\scripts\cp.js"</code>.
   * <p> Values may span multiple lines (i.e., contain carriage returns
   * and/or line feeds).
   * <p> Note that any translation specified in the plug-in manifest
   * file is automatically applied.
   * </p>
   *
   * @return the text value of this configuration element or <code>null</code>
   * @throws InvalidRegistryObjectException if this configuration element is no longer valid
   */
  virtual QString GetValue() const = 0;

  /**
   * When multi-language support is enabled, this method returns the text value of this
   * configuration element in the specified locale, or <code>null</code> if none.
   * <p>
   * The locale matching tries to find the best match between available translations and
   * the requested locale, falling back to a more generic locale ("en") when the specific
   * locale ("en_US") is not available.
   * </p><p>
   * If multi-language support is not enabled, this method is equivalent to the method
   * {@link #getValue()}.
   * </p>
   * @param locale the requested locale
   * @return the text value of this configuration element in the specified locale,
   * or <code>null</code>
   * @throws InvalidRegistryObjectException if this configuration element is no longer valid
   * @see #GetValue(String)
   * @see IExtensionRegistry#IsMultiLanguage()
   */
  virtual QString GetValue(const QLocale& locale) const = 0;

  /**
   * Returns the namespace name for this configuration element.
   *
   * @return the namespace name for this configuration element
   * @throws InvalidRegistryObjectException if this configuration element is no longer valid
   */
  virtual QString GetNamespaceIdentifier() const = 0;

  /**
   * Returns the contributor of this configuration element.
   *
   * @return the contributor for this configuration element
   * @throws InvalidRegistryObjectException if this configuration element is no longer valid
   */
  virtual SmartPointer<IContributor> GetContributor() const = 0;

  /**
   * Returns whether this configuration element object is valid.
   *
   * @return <code>true</code> if the object is valid, and <code>false</code>
   * if it is no longer valid
   */
  virtual bool IsValid() const = 0;

};

}  // namespace berry

Q_DECLARE_INTERFACE(berry::IConfigurationElement, "org.blueberry.core.IConfigurationElement")


#endif /*BERRYIEXTENSIONELEMENT_H_*/
