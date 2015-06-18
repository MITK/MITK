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

#ifndef BERRYIEDITORMATCHINGSTRATEGY_H_
#define BERRYIEDITORMATCHINGSTRATEGY_H_

#include <berryMacros.h>

#include "berryIEditorReference.h"
#include "berryIEditorInput.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 *
 * An editor matching strategy allows editor extensions to provide their own
 * algorithm for matching the input of an open editor of that type to a
 * given editor input.  This is used to find a matching editor during
 * {@link org.blueberry.ui.IWorkbenchPage#openEditor(IEditorInput, String, boolean)} and
 * {@link org.blueberry.ui.IWorkbenchPage#findEditor(IEditorInput)}.
 */
struct BERRY_UI_QT IEditorMatchingStrategy : public Object
{

  berryObjectMacro(berry::IEditorMatchingStrategy)

  ~IEditorMatchingStrategy();

  /**
   * Returns whether the editor represented by the given editor reference
   * matches the given editor input.
   * <p>
   * Implementations should inspect the given editor input first,
   * and try to reject it early before calling <code>IEditorReference.getEditorInput()</code>,
   * since that method may be expensive.
   * </p>
   *
   * @param editorRef the editor reference to match against
   * @param input the editor input to match
   * @return <code>true</code> if the editor matches the given editor input,
   *   <code>false</code> if it does not match
   */
  virtual bool Matches(IEditorReference::Pointer editorRef, IEditorInput::Pointer input) = 0;

};

}

Q_DECLARE_INTERFACE(berry::IEditorMatchingStrategy, "org.blueberry.ui.IEditorMatchingStrategy")

#endif /*BERRYIEDITORMATCHINGSTRATEGY_H_*/
