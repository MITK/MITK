/*=========================================================================

Program:   openCherry Platform
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

#ifndef CHERRYIEDITORMATCHINGSTRATEGY_H_
#define CHERRYIEDITORMATCHINGSTRATEGY_H_

#include <osgi/framework/Macros.h>

#include "cherryIEditorReference.h"
#include "cherryIEditorInput.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 *
 * An editor matching strategy allows editor extensions to provide their own
 * algorithm for matching the input of an open editor of that type to a
 * given editor input.  This is used to find a matching editor during
 * {@link org.opencherry.ui.IWorkbenchPage#openEditor(IEditorInput, String, boolean)} and
 * {@link org.opencherry.ui.IWorkbenchPage#findEditor(IEditorInput)}.
 *
 * @since 3.1
 */
struct CHERRY_UI IEditorMatchingStrategy : public Object {

  osgiInterfaceMacro(cherry::IEditorMatchingStrategy);

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

#endif /*CHERRYIEDITORMATCHINGSTRATEGY_H_*/
