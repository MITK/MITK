/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIREUSABLEEDITOR_H_
#define BERRYIREUSABLEEDITOR_H_

#include "berryIEditorPart.h"
#include "berryIEditorInput.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Interface for reusable editors.
 *
 * An editors may support changing its input so that
 * the workbench may change its contents instead of
 * opening a new editor.
 */
struct BERRY_UI_QT IReusableEditor : public virtual IEditorPart
{

  berryObjectMacro(berry::IReusableEditor);

  ~IReusableEditor() override;

    /**
     * Sets the input to this editor.
     *
     * <p><b>Note:</b> Clients must fire the {@link IWorkbenchPartConstants#PROP_INPUT }
     * property change within their implementation of
     * <code>setInput()</code>.<p>
     *
     * @param input the editor input
     */
    virtual void SetInput(IEditorInput::Pointer input) = 0;
};

}

#endif /*BERRYIREUSABLEEDITOR_H_*/
