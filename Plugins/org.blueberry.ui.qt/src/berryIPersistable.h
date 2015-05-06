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

