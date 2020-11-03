/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
            const QString& preferenceKey) override;

  void Save(const SmartPointer<IPreferences>& store,
            const QString& preferenceKey) override;

  void SetValue(const Object::Pointer& value) override;
};

}

#endif // BERRYTOGGLESTATE_H
