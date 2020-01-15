/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYREGISTRYTOGGLESTATE_H
#define BERRYREGISTRYTOGGLESTATE_H

#include "berryToggleState.h"
#include "berryIExecutableExtension.h"

namespace berry {

/**
 * <p>
 * A toggle state that can be read from the registry. This stores a piece of
 * boolean state information.
 * </p>
 * <p>
 * When parsing from the registry, this state understands two parameters:
 * <code>default</code>, which is the default value for this item; and
 * <code>persisted</code>, which is whether the state should be persisted
 * between sessions. The <code>default</code> parameter defaults to
 * <code>false</code>, and the <code>persisted</code> parameter defaults to
 * <code>true</code>. If only one parameter is passed (i.e., using the class
 * name followed by a colon), then it is assumed to be the <code>default</code>
 * parameter.
 * </p>
 * <p>
 * Clients may instantiate this class, but must not extend.
 * </p>
 */
class RegistryToggleState : public ToggleState, public IExecutableExtension
{

public:

  /**
   * The state ID for a toggle state understood by the system.
   */
  static const QString STATE_ID; // = "org.blueberry.ui.commands.toggleState";

  void SetInitializationData(const SmartPointer<IConfigurationElement>& configurationElement,
                             const QString& propertyName, const Object::Pointer& data) override;

private:

  /**
   * Reads the <code>default</code> parameter from the given string. This
   * converts the string to a boolean, using <code>true</code> as the
   * default.
   *
   * @param defaultString
   *            The string to parse; may be <code>null</code>.
   */
  void ReadDefault(const QString& defaultString);

  /**
   * Reads the <code>persisted</code> parameter from the given string. This
   * converts the string to a boolean, using <code>true</code> as the
   * default.
   *
   * @param persistedString
   *            The string to parse; may be <code>null</code>.
   */
  void ReadPersisted(const QString& persistedString);

};

}

#endif // BERRYREGISTRYTOGGLESTATE_H
