/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYEDITORAREAHELPER_H_
#define BERRYEDITORAREAHELPER_H_

#include "berryLayoutPart.h"
#include "berryLayoutPart.h"
#include "berryEditorReference.h"

namespace berry {

class EditorSashContainer;
class WorkbenchPage;
class PartPane;
class PartStack;

/**
 * EditorAreaHelper is a wrapper for PartTabworkbook.
 */
class EditorAreaHelper {

    //private ArrayList editorTable = new ArrayList(4);

private:

  SmartPointer<EditorSashContainer> editorArea;

  /**
   * Closes an editor.
   *
   * @param part the editor to close
   */
  void CloseEditor(SmartPointer<PartPane> pane);

 void AddToLayout(SmartPointer<PartPane> pane, SmartPointer<PartStack> stack);


public:

    /**
     * Creates a new EditorAreaHelper.
     */
    EditorAreaHelper(WorkbenchPage* page);

    /**
   * Displays a list of open editors
   */
    void DisplayEditorList();

    /**
     * Closes an editor.
     *
     * @param part the editor to close
     */
    void CloseEditor(IEditorReference::Pointer ref);

    /**
     * Closes an editor.
     *
     * @param part the editor to close
     */
    void CloseEditor(IEditorPart::Pointer part);

    /**
     * Deref a given part.  Deconstruct its container as required.
     * Do not remove drag listeners.
     */
    static void DerefPart(LayoutPart::Pointer part);

    /**
     * Dispose of the editor presentation.
     */
    ~EditorAreaHelper();

    /**
     * @see IEditorPresentation
     */
    QString GetActiveEditorWorkbookID();

    SmartPointer<PartStack> GetActiveWorkbook();

    /**
     * Returns the editor area.
     */
    LayoutPart::Pointer GetLayoutPart();

    /**
     * Returns the active editor in this perspective.  If the editors appear
     * in a workbook this will be the visible editor.  If the editors are
     * scattered around the workbench this will be the most recent editor
     * to hold focus.
     *
     * @return the active editor, or <code>null</code> if no editor is active
     */
    IEditorReference::Pointer GetVisibleEditor();

    void MoveEditor(IEditorPart::Pointer part, int position);

    /**
     * Main entry point for adding an editor. Adds the editor to the layout in the given
     * stack, and notifies the workbench page when done.
     *
     * @param ref editor to add
     * @param workbookId workbook that will contain the editor (or null if the editor
     * should be added to the default workbook)
     */
    void AddEditor(EditorReference::Pointer ref, const QString& workbookId);

    /**
     * @see IPersistablePart
     */
    bool RestoreState(IMemento::Pointer memento);

    /**
     * Restore the presentation
     * @param areaMem
     * @return
     */
    bool RestorePresentationState(IMemento::Pointer areaMem);

    /**
     * @see IPersistablePart
     */
    bool SaveState(IMemento::Pointer memento);

    /**
     * @see IEditorPresentation
     */
    void SetActiveEditorWorkbookFromID(const QString& id);

    void SetActiveWorkbook(SmartPointer<PartStack> workbook, bool hasFocus);

    /**
     * Brings an editor to the front and optionally gives it focus.
     *
     * @param part the editor to make visible
     * @param setFocus whether to give the editor focus
     * @return true if the visible editor was changed, false if not.
     */
    bool SetVisibleEditor(IEditorReference::Pointer ref, bool setFocus);

    /**
     * Method getWorkbooks.
     * @return ArrayList
     */
    QList<SmartPointer<PartStack> > GetWorkbooks();

    QList<IEditorReference::Pointer> GetEditors();

    SmartPointer<PartStack> GetWorkbookFromID(const QString& workbookId);

    void UpdateStackButtons();
};

}

#endif /* BERRYEDITORAREAHELPER_H_ */
