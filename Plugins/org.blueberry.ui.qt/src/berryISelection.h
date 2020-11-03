/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYISELECTION_H_
#define BERRYISELECTION_H_

#include <berryMacros.h>
#include <berryObject.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Interface for a selection.
 *
 * @see ISelectionProvider
 * @see ISelectionChangedListener
 * @see SelectionChangedEvent
 *
 **/
struct BERRY_UI_QT ISelection : public Object
{
  berryObjectMacro(berry::ISelection);

  ~ISelection() override;

  /**
   * Returns whether this selection is empty.
   *
   * @return <code>true</code> if this selection is empty,
   *   and <code>false</code> otherwise
   */
  virtual bool IsEmpty() const = 0;

};

}

#endif /*BERRYISELECTION_H_*/
