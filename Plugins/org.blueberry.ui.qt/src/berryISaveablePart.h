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
 * <code>Save As</code> actions.
 *
 * @see IEditorPart
 */
struct BERRY_UI_QT ISaveablePart : public virtual Object
{

  berryObjectMacro(berry::ISaveablePart)

    /**
     * The property id for <code>isDirty</code>.
     */
    static const int PROP_DIRTY; // = IWorkbenchPartConstants.PROP_DIRTY;

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
     *
     * @param monitor the progress monitor
     */
    virtual void DoSave(/*IProgressMonitor monitor*/) = 0;

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
     */
    virtual void DoSaveAs() = 0;

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
     *  if not supported
     */
    virtual bool IsSaveAsAllowed() const = 0;

    /**
     * Returns whether the contents of this part should be saved when the part
     * is closed.
     *
     * @return <code>true</code> if the contents of the part should be saved on
     *   close, and <code>false</code> if the contents are expendable
     */
    virtual bool IsSaveOnCloseNeeded() const = 0;
};

}

Q_DECLARE_INTERFACE(berry::ISaveablePart, "org.blueberry.ui.ISaveablePart")


#endif /* BERRYISAVEABLEPART_H_ */
