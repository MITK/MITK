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

#ifndef BERRYSHOWINCONTEXT_H
#define BERRYSHOWINCONTEXT_H

#include "berryObject.h"

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

struct ISelection;

/**
 * Carries the context for the Show In action.
 * The default implementation carries an input and a selection.
 * Subclasses may extend.
 *
 * @see IShowInSource
 * @see IShowInTarget
 */
class BERRY_UI_QT ShowInContext : public virtual Object
{

private:

  Object::Pointer input;
  SmartPointer<const ISelection> selection;

public:

  berryObjectMacro(berry::ShowInContext)

  /**
   * Constructs a new <code>ShowInContext</code> with the given input and
   * selection.
   *
   * @param input the input or <code>null</code>
   * @param selection the selection or <code>null</code>
   */
  ShowInContext(const Object::Pointer& input, const SmartPointer<const ISelection>& selection);

  /**
   * Returns the input, or <code>null</code> to indicate no input
   *
   * @return the input or <code>null</code>.
   */
  Object::Pointer GetInput() const;

  /**
   * Returns the selection, or <code>null</code> to indicate no selection.
   *
   * @return the selection or <code>null</code>
   */
  SmartPointer<const ISelection> GetSelection() const;

  /**
   * Sets the input, or <code>null</code> to indicate no input.
   *
   * @param input the input or <code>null</code>
   */
  void SetInput(const Object::Pointer& input);

  /**
   * Sets the selection, or <code>null</code> to indicate no selection.
   *
   * @param selection the selection or <code>null</code>
   */
  void SetSelection(const SmartPointer<const ISelection>& selection);

};

}

#endif // BERRYSHOWINCONTEXT_H
