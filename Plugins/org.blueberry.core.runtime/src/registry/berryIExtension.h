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

#ifndef BERRYIEXTENSION_H_
#define BERRYIEXTENSION_H_


#include <berryObject.h>

#include <QList>

namespace berry {

struct IConfigurationElement;
struct IContributor;

/**
 * An extension declared in a plug-in.
 * All information is obtained from the declaring plug-in's
 * manifest (<code>plugin.xml</code>) file.
 * <p>
 * These registry objects are intended for relatively short-term use. Clients that
 * deal with these objects must be aware that they may become invalid if the
 * declaring plug-in is updated or uninstalled. If this happens, all methods except
 * {@link #IsValid()} will throw {@link InvalidRegistryObjectException}.
 * For extension objects, the most common case is code in a plug-in dealing
 * with extensions contributed to one of the extension points it declares.
 * Code in a plug-in that has declared that it is not dynamic aware (or not
 * declared anything) can safely ignore this issue, since the registry
 * would not be modified while it is active. However, code in a plug-in that
 * declares that it is dynamic aware must be careful when accessing the extension
 * objects because they become invalid if the contributing plug-in is removed.
 * Similarly, tools that analyze or display the extension registry are vulnerable.
 * Client code can pre-test for invalid objects by calling {@link #IsValid()},
 * which never throws this exception. However, pre-tests are usually not sufficient
 * because of the possibility of the extension object becoming invalid as a
 * result of a concurrent activity. At-risk clients must treat
 * <code>InvalidRegistryObjectException</code> as if it were a checked exception.
 * Also, such clients should probably register a listener with the extension registry
 * so that they receive notification of any changes to the registry.
 * </p><p>
 * This interface is not intended to be implemented by clients.
 * </p>
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct org_blueberry_core_runtime_EXPORT IExtension : public virtual Object
{
  berryObjectMacro(berry::IExtension)

  ~IExtension() override;

  /**
   * Returns all configuration elements declared by this extension.
   * These elements are a direct reflection of the configuration
   * markup supplied in the manifest (<code>plugin.xml</code>)
   * file for the plug-in that declares this extension.
   * Returns an empty array if this extension does not declare any
   * configuration elements.
   *
   * @return the configuration elements declared by this extension
   * @throws InvalidRegistryObjectException if this extension is no longer valid
   */
  virtual QList<SmartPointer<IConfigurationElement> > GetConfigurationElements() const = 0;

  /**
   * Returns the namespace name for this extension.
   *
   * @return the namespace name for this extension
   * @throws InvalidRegistryObjectException if this extension is no longer valid
   */
  virtual QString GetNamespaceIdentifier() const = 0;

  /**
   * Returns the contributor of this extension.
   *
   * @return the contributor for this extension
   * @throws InvalidRegistryObjectException if this extension is no longer valid
   */
  virtual SmartPointer<IContributor> GetContributor() const = 0;

  /**
   * Returns the unique identifier of the extension point
   * to which this extension should be contributed.
   *
   * @return the unique identifier of the relevant extension point
   * @throws InvalidRegistryObjectException if this extension is no longer valid
   */
  virtual QString GetExtensionPointUniqueIdentifier() const = 0;

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
   * @throws InvalidRegistryObjectException if this extension is no longer valid
   */
  virtual QString GetLabel() const = 0;

  /**
   * Returns the simple identifier of this extension, or <code>null</code>
   * if this extension does not have an identifier.
   * This identifier is specified in the extensions manifest
   * file as a non-empty string containing no period characters
   * (<code>'.'</code>) and must be unique within the defining host.
   *
   * @return the simple identifier of the extension (e.g. <code>"main"</code>)
   *  or <code>null</code>
   * @throws InvalidRegistryObjectException if this extension is no longer valid
   */
  virtual QString GetSimpleIdentifier() const = 0;

  /**
   * Returns the unique identifier of this extension, or <code>null</code>
   * if this extension does not have an identifier.
   * If available, this identifier is unique within the extension registry, and
   * is composed of the identifier of the host that declared
   * this extension and this extension's simple identifier.
   *
   * @return the unique identifier of the extension
   *    (e.g. <code>"com.example.acme.main"</code>), or <code>null</code>
   * @throws InvalidRegistryObjectException if this extension is no longer valid
   */
  virtual QString GetUniqueIdentifier() const = 0;

  /**
   * Returns whether this extension object is valid.
   *
   * @return <code>true</code> if the object is valid, and <code>false</code>
   * if it is no longer valid
   */
  virtual bool IsValid() const = 0;

};

}

#endif /*BERRYIEXTENSION_H_*/
