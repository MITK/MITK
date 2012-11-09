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

#ifndef BERRYISELECTION_H_
#define BERRYISELECTION_H_

#include <berryMacros.h>
#include <berryObject.h>

#include <org_blueberry_ui_Export.h>

namespace berry {

/**
 * \ingroup org_blueberry_ui
 *
 * Interface for a selection.
 *
 * @see ISelectionProvider
 * @see ISelectionChangedListener
 * @see SelectionChangedEvent
 *
 **/
struct BERRY_UI ISelection : public Object
{
  berryInterfaceMacro(ISelection, berry)

  ~ISelection();

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
