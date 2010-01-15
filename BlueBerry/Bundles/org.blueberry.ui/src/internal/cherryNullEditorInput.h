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

#ifndef CHERRYNULLEDITORINPUT_H_
#define CHERRYNULLEDITORINPUT_H_

#include "../cherryIEditorInput.h"
#include "cherryEditorReference.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui_internal
 *
 */
class NullEditorInput : public IEditorInput {

private: EditorReference::Pointer editorReference;

public:

  cherryObjectMacro(NullEditorInput)

  NullEditorInput();

  /**
   * Creates a <code>NullEditorInput</code> for the
   * given editor reference.
   *
   * @param editorReference the editor reference
   * @since 3.4
   */
  NullEditorInput(EditorReference::Pointer editorReference);

    /* (non-Javadoc)
     * @see org.opencherry.ui.IEditorInput#exists()
     */
   bool Exists() const;

    /* (non-Javadoc)
     * @see org.opencherry.ui.IEditorInput#getImageDescriptor()
     */
//   ImageDescriptor getImageDescriptor() {
//        return ImageDescriptor.getMissingImageDescriptor();
//    }

    /* (non-Javadoc)
     * @see org.opencherry.ui.IEditorInput#getName()
     */
    std::string GetName() const;

    /* (non-Javadoc)
     * @see org.opencherry.ui.IEditorInput#getToolTipText()
     */
    std::string GetToolTipText() const;

    bool operator==(const Object* o) const;

};

}

#endif /*CHERRYNULLEDITORINPUT_H_*/
