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

#ifndef BERRYNULLEDITORINPUT_H_
#define BERRYNULLEDITORINPUT_H_

#include "berryIEditorInput.h"
#include "berryEditorReference.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui_internal
 *
 */
class NullEditorInput : public IEditorInput {

private: EditorReference::Pointer editorReference;

public:

  berryObjectMacro(NullEditorInput);

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
     * @see org.blueberry.ui.IEditorInput#exists()
     */
   bool Exists() const;

    /* (non-Javadoc)
     * @see org.blueberry.ui.IEditorInput#getImageDescriptor()
     */
//   ImageDescriptor getImageDescriptor() {
//        return ImageDescriptor.getMissingImageDescriptor();
//    }

    /* (non-Javadoc)
     * @see org.blueberry.ui.IEditorInput#getName()
     */
    std::string GetName() const;

    /* (non-Javadoc)
     * @see org.blueberry.ui.IEditorInput#getToolTipText()
     */
    std::string GetToolTipText() const;

    bool operator==(const Object* o) const;

};

}

#endif /*BERRYNULLEDITORINPUT_H_*/
