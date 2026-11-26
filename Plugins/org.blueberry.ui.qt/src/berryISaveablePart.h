/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYISAVEABLEPART_H_
#define BERRYISAVEABLEPART_H_

#include <berryMacros.h>
#include <berryObject.h>

#include <org_blueberry_ui_qt_Export.h>

#include <QtPlugin>

namespace berry {

/**
 * Workbench parts implement or adapt to this interface to participate
 * in the enablement and execution of the <code>Save</code> and
 * <code>Save As</code> actions, and to have a say when they are about to be
 * closed while they are dirty.
 *
 * Only IsDirty() has to be implemented. A part that does nothing else is
 * closed without a question, like a part that is not saveable.
 *
 * @see IEditorPart
 */
struct BERRY_UI_QT ISaveablePart : public virtual Object
{

  berryObjectMacro(berry::ISaveablePart);

    /**
     * The property id for <code>isDirty</code>.
     */
    static const int PROP_DIRTY; // = IWorkbenchPartConstants.PROP_DIRTY;

    /**
     * What to do with a dirty part that is about to be closed.
     */
    enum class CloseAnswer
    {
      /** Save the part with DoSave() and close it. */
      Save,
      /** Close the part without saving it. */
      Discard,
      /** Keep the part open. */
      Cancel,
      /** The part did not ask. It is closed without saving. */
      NotAsked
    };

    /**
     * Saves the contents of this part.
     * <p>
     * If the save is successful, the part should fire a property changed event
     * reflecting the new dirty state (<code>PROP_DIRTY</code> property).
     * </p>
     * <p>
     * If the save is cancelled through user action, or for any other reason, the
     * part should invoke <code>setCancelled</code> on the <code>IProgressMonitor</code>
     * to inform the caller.
     * </p>
     * <p>
     * This method is long-running; progress and cancellation are provided
     * by the given progress monitor.
     * </p>
     * <p>
     * The default does nothing.
     * </p>
     */
    virtual void DoSave(/*IProgressMonitor monitor*/) {}

    /**
     * Saves the contents of this part to another object.
     * <p>
     * Implementors are expected to open a "Save As" dialog where the user will
     * be able to select a new name for the contents. After the selection is made,
     * the contents should be saved to that new name.  During this operation a
     * <code>IProgressMonitor</code> should be used to indicate progress.
     * </p>
     * <p>
     * If the save is successful, the part fires a property changed event
     * reflecting the new dirty state (<code>PROP_DIRTY</code> property).
     * </p>
     * <p>
     * The default does nothing.
     * </p>
     */
    virtual void DoSaveAs() {}

    /**
     * Returns whether the contents of this part have changed since the last save
     * operation. If this value changes the part must fire a property listener
     * event with <code>PROP_DIRTY</code>.
     * <p>
     * <b>Note:</b> this method is called often on a part open or part
     * activation switch, for example by actions to determine their
     * enabled status.
     * </p>
     *
     * @return <code>true</code> if the contents have been modified and need
     *   saving, and <code>false</code> if they have not changed since the last
     *   save
     */
    virtual bool IsDirty() const = 0;

    /**
     * Returns whether the "Save As" operation is supported by this part.
     *
     * @return <code>true</code> if "Save As" is supported, and <code>false</code>
     *  if not supported, which is the default
     */
    virtual bool IsSaveAsAllowed() const { return false; }

    /**
     * Returns whether the contents of this part should be saved when the part
     * is closed.
     *
     * @return <code>true</code> if the contents of the part should be saved on
     *   close, which is the default, and <code>false</code> if the contents
     *   are expendable
     */
    virtual bool IsSaveOnCloseNeeded() const { return true; }

    /**
     * Asks the user what to do with the dirty part that is about to be closed.
     * <p>
     * The workbench calls this when a view or an editor is closed, or when a
     * perspective is reset, and the part is dirty and IsSaveOnCloseNeeded()
     * returns <code>true</code>. A part that is still dirty after the
     * DoSave() that follows <code>CloseAnswer::Save</code> stays open.
     * Closing the whole workbench does not ask the parts. A part that needs
     * a say there implements IWorkbenchListener.
     * </p>
     * <p>
     * The default does not ask and returns <code>CloseAnswer::NotAsked</code>.
     * </p>
     */
    virtual CloseAnswer PromptToSaveOnClose() { return CloseAnswer::NotAsked; }
};

}

Q_DECLARE_INTERFACE(berry::ISaveablePart, "org.blueberry.ui.ISaveablePart")


#endif /* BERRYISAVEABLEPART_H_ */
