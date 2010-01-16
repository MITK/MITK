/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef BERRYIEDITORMATCHINGSTRATEGY_H_
#define BERRYIEDITORMATCHINGSTRATEGY_H_

#include <osgi/framework/Macros.h>

#include "berryIEditorReference.h"
#include "berryIEditorInput.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui
 *
 * An editor matching strategy allows editor extensions to provide their own
 * algorithm for matching the input of an open editor of that type to a
 * given editor input.  This is used to find a matching editor during
 * {@link org.blueberry.ui.IWorkbenchPage#openEditor(IEditorInput, String, boolean)} and
 * {@link org.blueberry.ui.IWorkbenchPage#findEditor(IEditorInput)}.
 *
 * @since 3.1
 */
struct BERRY_UI IEditorMatchingStrategy : public Object {

  osgiInterfaceMacro(berry::IEditorMatchingStrategy);

  ~IEditorMatchingStrategy() {}

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

#endif /*BERRYIEDITORMATCHINGSTRATEGY_H_*/
