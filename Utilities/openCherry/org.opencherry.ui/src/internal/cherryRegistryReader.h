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

#ifndef CHERRYREGISTRYREADER_H_
#define CHERRYREGISTRYREADER_H_

#include "service/cherryIConfigurationElement.h"
#include "service/cherryIExtension.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui_internal
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

    /**
     * Logs the error in the workbench log using the provided
     * text and the information in the configuration element.
     */
   static void LogError(IConfigurationElement::Pointer element, const std::string& text);

    /**
     * Logs a very common registry error when a required attribute is missing.
     */
    static void LogMissingAttribute(IConfigurationElement::Pointer element,
            const std::string& attributeName);

    /**
     * Logs a very common registry error when a required child is missing.
     */
    static void LogMissingElement(IConfigurationElement::Pointer element,
            const std::string& elementName);

    /**
     * Logs a registry error when the configuration element is unknown.
     */
    static void LogUnknownElement(IConfigurationElement::Pointer element);

    
public:
  
    /**
     * Apply a reproducable order to the list of extensions
     * provided, such that the order will not change as
     * extensions are added or removed.
     * @param extensions the extensions to order
     * @return ordered extensions
     */
    static const std::vector<const IExtension*> OrderExtensions(const std::vector<const IExtension*>& extensions);

    
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
    virtual bool ReadElement(IConfigurationElement::Pointer element) = 0;

    /**
     * Read the element's children. This is called by
     * the subclass' readElement method when it wants
     * to read the children of the element.
     */
    virtual void ReadElementChildren(IConfigurationElement::Pointer element);

    /**
     * Read each element one at a time by calling the
     * subclass implementation of <code>readElement</code>.
     *
     * Logs an error if the element was not recognized.
     */
    virtual void ReadElements(const std::vector<IConfigurationElement::Pointer>& elements);

    /**
     * Read one extension by looping through its
     * configuration elements.
     */
    virtual void ReadExtension(const IExtension* extension);

    
public:
  
    /**
     *  Start the registry reading process using the
     * supplied plugin ID and extension point.
     * 
     * @param registry the registry to read from
     * @param pluginId the plugin id of the extenion point
     * @param extensionPoint the extension point id
     */
    virtual void ReadRegistry(const std::string& pluginId,
            const std::string& extensionPoint);
    
    /**
     * Utility for extracting the description child of an element.
     * 
     * @param configElement the element
     * @return the description
     * @since 3.1
     */
    static std::string GetDescription(IConfigurationElement::Pointer configElement);
    
    /**
   * Utility for extracting the value of a class attribute or a nested class
   * element that follows the pattern set forth by
   * {@link org.opencherry.core.runtime.IExecutableExtension}.
   * 
   * @param configElement
   *            the element
   * @param classAttributeName
   *            the name of the class attribute to check
   * @return the value of the attribute or nested class element
   * @since 3.1
   */
    static std::string GetClassValue(IConfigurationElement::Pointer configElement, const std::string& classAttributeName);
};

}

#endif /*CHERRYREGISTRYREADER_H_*/
