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

#ifndef CHERRYEXTENSION_H_
#define CHERRYEXTENSION_H_

#include "../service/cherryIExtension.h"
#include "cherryConfigurationElement.h"

namespace cherry {

/**
 * An object which represents the user-defined extension in a
 * plug-in manifest.
 * <p>
 * This class may be instantiated, or further subclassed.
 * </p>
 */
class Extension : public IExtension, public Object {


private:

  // DTD properties (included in plug-in manifest)
  std::string extensionPoint;
  std::string id;
  std::string label;
  std::string namespaze;
  std::vector<IConfigurationElement::Pointer> elements;


public:

  cherryClassMacro(Extension);

  Extension(const std::string& namespaze);

  /**
   * Two Extensions are equal if they have the same Id
   * and target the same extension point.
   */
  bool operator==(const Object* em) const;

  /**
   * Returns the extension point with which this extension is associated.
   *
   * @return the extension point with which this extension is associated
   *  or <code>null</code>
   */
  std::string GetExtensionPointIdentifier() const;

  std::string GetNamespace() const;

  /**
   * Returns the simple identifier of this extension, or <code>null</code>
   * if this extension does not have an identifier.
   * This identifier is specified in the plug-in manifest as a non-empty
   * string containing no period characters (<code>'.'</code>) and
   * must be unique within the defining plug-in.
   *
   * @return the simple identifier of the extension (e.g. <code>"main"</code>)
   *  or <code>null</code>
   */
  std::string GetSimpleIdentifier() const;

  std::string GetUniqueIdentifier() const;

  const std::vector<IConfigurationElement::Pointer> GetConfigurationElements() const;

  std::string GetLabel() const;

  /**
   * Set the extension point with which this extension is associated.
   * This object must not be read-only.
   *
   * @return the extension point with which this extension is associated.
   *    May be <code>null</code>.
   */
  void SetExtensionPointIdentifier(const std::string& value);

  /**
   * Sets the simple identifier of this extension, or <code>null</code>
   * if this extension does not have an identifier.
   * This identifier is specified in the plug-in manifest as a non-empty
   * string containing no period characters (<code>'.'</code>) and
   * must be unique within the defining plug-in.
   * This object must not be read-only.
   *
   * @param value the simple identifier of the extension (e.g. <code>"main"</code>).
   *    May be <code>null</code>.
   */
  void SetSimpleIdentifier(const std::string& value);

  /**
   * Sets the configuration element children of this extension.
   *
   * @param value the configuration elements in this extension.
   *    May be <code>null</code>.
   */
  void SetSubElements(const std::vector<IConfigurationElement::Pointer>& value);

  void SetLabel(const std::string& l);

  bool operator<(const IExtension* e2) const;

};

}

#endif /*CHERRYEXTENSION_H_*/
