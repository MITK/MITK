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

#ifndef CHERRYIEXTENSIONPOINT_H_
#define CHERRYIEXTENSIONPOINT_H_

#include "../cherryOSGiDll.h"

#include "cherryIExtension.h"

#include <vector>

namespace cherry {

struct IBundle;

/**
 * An extension point declared in a plug-in.
 * Except for the list of extensions plugged in to it, the information
 * available for an extension point is obtained from the declaring plug-in's
 * manifest (<code>plugin.xml</code>) file.
 * <p>
 * These registry objects are intended for relatively short-term use. Clients that
 * deal with these objects must be aware that they may become invalid if the
 * declaring plug-in is updated or uninstalled. If this happens, all methods except
 * {@link #isValid()} will throw {@link InvalidRegistryObjectException}.
 * For extension point objects, the most common case is code in a plug-in dealing
 * with one of the extension points it declares. These extension point objects are
 * guaranteed to be valid while the plug-in is active. Code in a plug-in that has
 * declared that it is not dynamic aware (or not declared anything) can also safely
 * ignore this issue, since the registry would not be modified while it is
 * active. However, code in a plug-in that declares that it is dynamic aware
 * must be careful if it access the extension point object of a different plug-in,
 * because it's at risk if that other plug-in is removed. Similarly,
 * tools that analyze or display the extension registry are vulnerable.
 * Client code can pre-test for invalid objects by calling {@link #isValid()},
 * which never throws this exception. However, pre-tests are usually not sufficient
 * because of the possibility of the extension point object becoming invalid as a
 * result of a concurrent activity. At-risk clients must treat
 * <code>InvalidRegistryObjectException</code> as if it were a checked exception.
 * Also, such clients should probably register a listener with the extension registry
 * so that they receive notification of any changes to the registry.
 * </p>
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 */
struct CHERRY_OSGI IExtensionPoint
{

public:

  virtual ~IExtensionPoint() {};

  virtual std::string GetContributor() const = 0;

  /**
   * Returns all configuration elements from all extensions configured
   * into this extension point. Returns an empty array if this extension
   * point has no extensions configured, or none of the extensions
   * contain configuration elements.
   *
   * @return the configuration elements for all extension configured
   *   into this extension point
   * @throws InvalidRegistryObjectException if this extension point is no longer valid
   */
  virtual const std::vector<IConfigurationElement::Pointer > GetConfigurationElements() const = 0;

  /**
   * Returns the extension with the given unique identifier configured into
   * this extension point, or <code>null</code> if there is no such extension.
   * Since an extension might not have an identifier, some extensions
   * can only be found via the <code>getExtensions</code> method.
   *
   * @param extensionId the unique identifier of an extension
   *    (e.g. <code>"com.example.acme.main"</code>).
   * @return an extension, or <code>null</code>
   * @throws InvalidRegistryObjectException if this extension point is no longer valid
   */
  virtual const IExtension* GetExtension(const std::string& extensionId) const = 0;

  /**
   * Returns all extensions configured into this extension point.
   * Returns an empty array if this extension point has no extensions.
   *
   * @return the extensions configured into this extension point
   * @throws InvalidRegistryObjectException if this extension point is no longer valid
   */
  virtual const std::vector<const IExtension*> GetExtensions() const = 0;

  /**
   * Returns a displayable label for this extension point.
   * Returns the empty string if no label for this extension point
   * is specified in the plug-in manifest file.
   * <p> Note that any translation specified in the plug-in manifest
   * file is automatically applied.
   * </p>
   *
   * @return a displayable string label for this extension point,
   *    possibly the empty string
   * @throws InvalidRegistryObjectException if this extension point is no longer valid
   */
  virtual std::string GetLabel() const = 0;

  /**
   * Returns the simple identifier of this extension point.
   * This identifier is a non-empty string containing no
   * period characters (<code>'.'</code>) and is guaranteed
   * to be unique within the defining plug-in.
   *
   * @return the simple identifier of the extension point (e.g. <code>"builders"</code>)
   * @throws InvalidRegistryObjectException if this extension point is no longer valid
   */
  virtual std::string GetSimpleIdentifier() const = 0;

  /**
   * Returns the unique identifier of this extension point.
   * This identifier is unique within the plug-in registry, and
   * is composed of the namespace for this extension point
   * and this extension point's simple identifier.
   *
   *
   * @return the unique identifier of the extension point
   *    (e.g. <code>"org.opencherry.core.resources.builders"</code>)
   * @throws InvalidRegistryObjectException if this extension point is no longer valid
   */
  virtual std::string GetUniqueIdentifier() const = 0;

};

}

#endif /*CHERRYIEXTENSIONPOINT_H_*/
