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

#ifndef BERRYRADIOSTATE_H
#define BERRYRADIOSTATE_H

#include "internal/berryPersistentState.h"
#include "berryIExecutableExtension.h"

namespace berry {

/**
 * This state supports a radio-button like command, where the value of the
 * parameterized command is stored as state. The command must define a state
 * using the {@link #STATE_ID} id and a string commandParameter using the
 * {@link #PARAMETER_ID} id. Menu contributions supplied by
 * <code>org.eclipse.ui.menus</code> can then set the {@link #PARAMETER_ID}.
 * <p>
 * When parsing from the registry, this state understands two parameters:
 * <code>default</code>, which is the default value for this item; and
 * <code>persisted</code>, which is whether the state should be persisted
 * between sessions. The <code>default</code> parameter has no default value and
 * must be specified in one of its forms, and the <code>persisted</code>
 * parameter defaults to <code>true</code>. If only one parameter is passed
 * (i.e., using the class name followed by a colon), then it is assumed to be
 * the <code>default</code> parameter.
 * </p>
 *
 * @see HandlerUtil#updateRadioState(org.eclipse.core.commands.Command, String)
 * @see HandlerUtil#matchesRadioState(org.eclipse.core.commands.ExecutionEvent)
 */
class RadioState : public PersistentState, public IExecutableExtension
{

public:

  /**
   * The state ID for a radio state understood by the system.
   */
  static const QString STATE_ID; // = "org.blueberry.ui.commands.radioState";

  /**
   * The parameter ID for a radio state understood by the system.
   */
  static const QString PARAMETER_ID; // = "org.blueberry.ui.commands.radioStateParameter";

  RadioState();

  void SetInitializationData(const SmartPointer<IConfigurationElement>& config,
                             const QString& propertyName, const Object::Pointer& data) override;

  void Load(const SmartPointer<IPreferences>& store, const QString& preferenceKey) override;

  void Save(const SmartPointer<IPreferences>& store, const QString& preferenceKey) override;

  void SetValue(const Object::Pointer& value) override;

};

}

#endif // BERRYRADIOSTATE_H
