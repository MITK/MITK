/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYREGISTRYREADER_H_
#define BERRYREGISTRYREADER_H_

#include <berrySmartPointer.h>

#include <QList>

namespace berry {

struct IExtension;
struct IExtensionRegistry;
struct IConfigurationElement;

/**
 * \ingroup org_blueberry_ui_internal
 *
 * Template implementation of a registry reader that creates objects
 * representing registry contents. Typically, an extension
 * contains one element, but this reader handles multiple
 * elements per extension.
 *
 * To start reading the extensions from the registry for an
 * extension point, call the method <code>readRegistry</code>.
 *
 * To read children of an IConfigurationElement, call the
 * method <code>readElementChildren</code> from your implementation
 * of the method <code>readElement</code>, as it will not be
 * done by default.
 */
class RegistryReader {

    // for dynamic UI - remove this cache to avoid inconsistency
    //protected static Hashtable extensionPoints = new Hashtable();
    /**
     * The constructor.
     */
protected:

  RegistryReader();
  virtual ~RegistryReader();

    /**
     * Logs the error in the workbench log using the provided
     * text and the information in the configuration element.
     */
   static void LogError(const SmartPointer<IConfigurationElement>& element,
                        const QString& text);

    /**
     * Logs a very common registry error when a required attribute is missing.
     */
    static void LogMissingAttribute(const SmartPointer<IConfigurationElement>& element,
                                    const QString& attributeName);

    /**
     * Logs a very common registry error when a required child is missing.
     */
    static void LogMissingElement(const SmartPointer<IConfigurationElement>& element,
                                  const QString& elementName);

    /**
     * Logs a registry error when the configuration element is unknown.
     */
    static void LogUnknownElement(const SmartPointer<IConfigurationElement>& element);


public:

    /**
     * Apply a reproducable order to the list of extensions
     * provided, such that the order will not change as
     * extensions are added or removed.
     * @param extensions the extensions to order
     * @return ordered extensions
     */
    static QList<SmartPointer<IExtension> > OrderExtensions(const QList<SmartPointer<IExtension> >& extensions);


protected:

    /**
     * Implement this method to read element's attributes.
     * If children should also be read, then implementor
     * is responsible for calling <code>readElementChildren</code>.
     * Implementor is also responsible for logging missing
     * attributes.
     *
     * @return true if element was recognized, false if not.
     */
    virtual bool ReadElement(const SmartPointer<IConfigurationElement>& element) = 0;

    /**
     * Read the element's children. This is called by
     * the subclass' readElement method when it wants
     * to read the children of the element.
     */
    virtual void ReadElementChildren(const SmartPointer<IConfigurationElement>& element);

    /**
     * Read each element one at a time by calling the
     * subclass implementation of <code>readElement</code>.
     *
     * Logs an error if the element was not recognized.
     */
    virtual void ReadElements(const QList<SmartPointer<IConfigurationElement> >& elements);

    /**
     * Read one extension by looping through its
     * configuration elements.
     */
    virtual void ReadExtension(const SmartPointer<IExtension>& extension);


public:

    /**
     *  Start the registry reading process using the
     * supplied plugin ID and extension point.
     *
     * @param registry the registry to read from
     * @param pluginId the plugin id of the extenion point
     * @param extensionPoint the extension point id
     */
    virtual void ReadRegistry(IExtensionRegistry* registry,
                              const QString& pluginId,
                              const QString& extensionPoint);

    /**
     * Utility for extracting the description child of an element.
     *
     * @param configElement the element
     * @return the description
     */
    static QString GetDescription(const SmartPointer<IConfigurationElement>& configElement);

    /**
   * Utility for extracting the value of a class attribute or a nested class
   * element that follows the pattern set forth by
   * {@link org.blueberry.core.runtime.IExecutableExtension}.
   *
   * @param configElement
   *            the element
   * @param classAttributeName
   *            the name of the class attribute to check
   * @return the value of the attribute or nested class element
   */
    static QString GetClassValue(const SmartPointer<IConfigurationElement>& configElement,
                                 const QString& classAttributeName);
};

}

#endif /*BERRYREGISTRYREADER_H_*/
