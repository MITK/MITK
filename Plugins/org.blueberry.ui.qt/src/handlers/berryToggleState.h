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

#ifndef BERRYTOGGLESTATE_H
#define BERRYTOGGLESTATE_H

#include "internal/berryPersistentState.h"

namespace berry {

/**
 * <p>
 * A piece of state storing a \c bool.
 * </p>
 * <p>
 * If this state is registered using {@link IMenuStateIds#STYLE}, then it will
 * control the presentation of the command if displayed in the menus, tool bars
 * or status line.
 * </p>
 * <p>
 * Clients may instantiate this class, but must not extend.
 * </p>
 */
class ToggleState : public PersistentState
{

public:

  /**
   * Constructs a new <code>ToggleState</code>. By default, the toggle is
   * off (e.g., <code>false</code>).
   */
  ToggleState();

  void Load(const SmartPointer<IPreferences>& store,
            const QString& preferenceKey);

  void Save(const SmartPointer<IPreferences>& store,
            const QString& preferenceKey);

  void SetValue(const Object::Pointer& value);
};

}

#endif // BERRYTOGGLESTATE_H
