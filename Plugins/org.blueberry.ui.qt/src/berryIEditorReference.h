/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIEDITORREFERENCE_H_
#define BERRYIEDITORREFERENCE_H_

#include "berryIWorkbenchPartReference.h"
#include "berryIEditorPart.h"
#include "berryIEditorInput.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Implements a reference to a editor.
 * The IEditorPart will not be instanciated until
 * the editor becomes visible or the API getEditor
 * is sent with true;
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 */
struct BERRY_UI_QT IEditorReference : public virtual IWorkbenchPartReference
{

  berryObjectMacro(berry::IEditorReference);

  ~IEditorReference() override;

    /**
     * Returns the factory id of the factory used to
     * restore this editor. Returns null if the editor
     * is not persistable.
     */
    virtual QString GetFactoryId() = 0;

    /**
     * Returns the editor input name. May return null is the
     * name is not available or if the editor failed to be
     * restored.
     */
    virtual QString GetName() = 0;

    /**
     * Returns the editor referenced by this object.
     * Returns <code>null</code> if the editor was not instantiated or
     * it failed to be restored. Tries to restore the editor
     * if <code>restore</code> is true.
     */
    virtual IEditorPart::Pointer GetEditor(bool restore) = 0;


    /**
     * Returns the editor input for the editor referenced by this object.
     * <p>
     * Unlike most of the other methods on this type, this method
     * can trigger plug-in activation.
     * </p>
     *
     * @return the editor input for the editor referenced by this object
     * @throws PartInitException if there was an error restoring the editor input
     * @since 3.1
     */
    virtual IEditorInput::Pointer GetEditorInput() = 0;
};

} // namespace berry

#endif /*BERRYIEDITORREFERENCE_H_*/
