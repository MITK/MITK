/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIPERSISTABLE_H
#define BERRYIPERSISTABLE_H

#include <org_blueberry_ui_qt_Export.h>
#include <berrySmartPointer.h>

namespace berry {

struct IMemento;

/**
 * Objects implementing this interface are capable of saving their
 * state in an {@link IMemento}.
 */
struct BERRY_UI_QT IPersistable
{
  virtual ~IPersistable();

  /**
   * Saves the state of the object in the given memento.
   *
   * @param memento the storage area for object's state
   */
  virtual void SaveState(const SmartPointer<IMemento>& memento) const = 0;
};

}

#endif // BERRYIPERSISTABLE_H
