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

#ifndef CHERRYIEXTENSION_H_
#define CHERRYIEXTENSION_H_

#include "cherryIConfigurationElement.h"

namespace cherry {

/**
 * An extension declared in a host.
 * All information is obtained from the declaring host 
 * extensions manifest file. 
 * <p>
 * This interface is not intended to be implemented by clients.
 * 
 */
struct CHERRY_OSGI IExtension {
  
  /**
   * Returns all configuration elements declared by this extension.
   * These elements are a direct reflection of the configuration 
   * markup supplied in the manifest file for the host that declares 
   * this extension.
   * Returns an empty array if this extension does not declare any
   * configuration elements.
   *
   * @return the configuration elements declared by this extension 
   */
  virtual const std::vector<IConfigurationElement::Pointer> GetConfigurationElements() const = 0;
  
  virtual std::string GetNamespace() const = 0;
  
  /**
   * Returns the unique identifier of the extension point
   * that this extension gets plugged into.
   *
   * @return the unique identifier of the relevant extension point
   */
  virtual std::string GetExtensionPointIdentifier() const = 0;

  /**
   * Returns the simple identifier of this extension, or <code>null</code>
   * if this extension does not have an identifier.
   * This identifier is specified in the extensions manifest  
   * file as a non-empty string containing no period characters 
   * (<code>'.'</code>) and must be unique within the defining host.
   *
   * @return the simple identifier of the extension (e.g. <code>"main"</code>)
   *  or <code>null</code>
   */
  virtual std::string GetSimpleIdentifier() const = 0;
  
  /**
   * Returns the unique identifier of this extension, or <code>null</code>
   * if this extension does not have an identifier.
   * If available, this identifier is unique within the extension registry, and
   * is composed of the identifier of the host that declared
   * this extension and this extension's simple identifier.
   *
   * @return the unique identifier of the extension
   *    (e.g. <code>"com.example.acme.main"</code>), or <code>null</code>
   */
  virtual std::string GetUniqueIdentifier() const = 0;
  
  /**
   * Returns a displayable label for this extension.
   * Returns the empty string if no label for this extension
   * is specified in the extension manifest file.
   * <p> Note that any translation specified in the extension manifest
   * file is automatically applied.
   * <p>
   *
   * @return a displayable string label for this extension,
   *    possibly the empty string
   */
  virtual std::string GetLabel() const = 0;
  
  virtual bool operator<(const IExtension* e2) const = 0;
  
  virtual ~IExtension() {}

};

}

#endif /*CHERRYIEXTENSION_H_*/
