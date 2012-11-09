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

#ifndef BERRYIREUSABLEEDITOR_H_
#define BERRYIREUSABLEEDITOR_H_

#include "berryIEditorPart.h"
#include "berryIEditorInput.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui
 *
 * Interface for reusable editors.
 *
 * An editors may support changing its input so that
 * the workbench may change its contents instead of
 * opening a new editor.
 */
struct BERRY_UI IReusableEditor : public virtual IEditorPart {

  berryInterfaceMacro(IReusableEditor, berry)

  virtual ~IReusableEditor();

    /**
     * Sets the input to this editor.
     *
     * <p><b>Note:</b> Clients must fire the {@link IEditorPart#PROP_INPUT }
     * property change within their implementation of
     * <code>setInput()</code>.<p>
     *
     * @param input the editor input
     */
    virtual void SetInput(IEditorInput::Pointer input) = 0;
};

}

#endif /*BERRYIREUSABLEEDITOR_H_*/
