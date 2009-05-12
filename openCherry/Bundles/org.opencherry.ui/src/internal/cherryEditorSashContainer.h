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

#ifndef CHERRYEDITORSASHCONTAINER_H_
#define CHERRYEDITORSASHCONTAINER_H_

#include "cherryPartSashContainer.h"
#include "cherryPartStack.h"

#include <list>

namespace cherry
{

/**
 * Represents the area set aside for editor workbooks.
 * This container only accepts editor stacks (PartStack) and PartSash
 * as layout parts.
 *
 * Note no views are allowed within this container.
 */
class EditorSashContainer: public PartSashContainer
{

public:
  cherryObjectMacro(EditorSashContainer);

private:

  std::list<PartStack::Pointer> editorWorkbooks;

  PartStack::Pointer activeEditorWorkbook;

  // DropTarget dropTarget;

  //TODO DND
  // void AddDropSupport() {
  //        if (dropTarget == null) {
  //            WorkbenchWindowConfigurer winConfigurer = ((WorkbenchWindow) page
  //                    .getWorkbenchWindow()).getWindowConfigurer();
  //
  //            dropTarget = new DropTarget(getControl(), DND.DROP_DEFAULT
  //                    | DND.DROP_COPY | DND.DROP_LINK);
  //            dropTarget.setTransfer(winConfigurer.getTransfers());
  //            if (winConfigurer.getDropTargetListener() != null) {
  //                dropTarget.addDropListener(winConfigurer
  //                        .getDropTargetListener());
  //            }
  //        }
  //    }

  PartStack::Pointer NewEditorWorkbook();

protected:

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.PartSashContainer#addChild(org.opencherry.ui.internal.PartSashContainer.RelationshipInfo)
   */
  void AddChild(const RelationshipInfo& info);

  /**
   * Notification that a child layout part has been
   * added to the container. Subclasses may override
   * this method to perform any container specific
   * work.
   */
  void ChildAdded(LayoutPart::Pointer child);

  /**
   * Notification that a child layout part has been
   * removed from the container. Subclasses may override
   * this method to perform any container specific
   * work.
   */
  void ChildRemoved(LayoutPart::Pointer child);

  PartStack::Pointer CreateDefaultWorkbook();

  /**
   * Subclasses override this method to specify
   * the composite to use to parent all children
   * layout parts it contains.
   */
  void* CreateParent(void* parentWidget);

  /**
   * Subclasses override this method to dispose
   * of any swt resources created during createParent.
   */
  void DisposeParent();

  /**
   * Return true is the workbook specified
   * is the active one.
   */
  bool IsActiveWorkbook(PartStack::Pointer workbook);

  //TODO DND
  //           /* package */DropTarget getDropTarget() {
  //                   return dropTarget;
  //               }

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.PartSashContainer#createStack(org.opencherry.ui.internal.LayoutPart)
   */
  PartStack::Pointer CreateStack();

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.PartSashContainer#setVisiblePart(org.opencherry.ui.internal.ILayoutContainer, org.opencherry.ui.internal.LayoutPart)
   */
  void SetVisiblePart(IStackableContainer::Pointer container,
      PartPane::Pointer visiblePart);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.PartSashContainer#getVisiblePart(org.opencherry.ui.internal.ILayoutContainer)
   */
  StackablePart::Pointer GetVisiblePart(IStackableContainer::Pointer container);

public:

  static const std::string DEFAULT_WORKBOOK_ID;

  EditorSashContainer(const std::string& editorId, WorkbenchPage::Pointer page,
      void* parent);

  bool AllowsAdd(LayoutPart::Pointer layoutPart);

  /**
   * Add an editor to the active workbook.
   */
  void AddEditor(PartPane::Pointer pane, PartStack::Pointer stack);

  /**
   * Hides the min/max buttons for all editor stacks
   * -except- for the upper/left one.
   */
  void UpdateStackButtons();

  /**
   * @param stacks
   * @return the EditorStack in the upper right position
   */
  PartStack::Pointer GetUpperRightEditorStack();

  /**
   * @param stacks
   * @return the EditorStack in the upper right position
   */
  PartStack::Pointer GetUpperRightEditorStack(
      const ILayoutContainer::ChildrenType& stacks);

  /**
   * Return the editor workbook which is active.
   */
  PartStack::Pointer GetActiveWorkbook();

  /**
   * Return the editor workbook id which is active.
   */
  std::string GetActiveWorkbookID();

  /**
   * Return the all the editor workbooks.
   */
  std::list<PartStack::Pointer> GetEditorWorkbooks();

  /**
   * Return the all the editor workbooks.
   */
  int GetEditorWorkbookCount();

  /**
   * Find the sashes around the specified part.
   */
  void FindSashes(LayoutPart::Pointer pane, PartPane::Sashes& sashes);

  /**
   * Remove all the editors
   */
  void RemoveAllEditors();

  /**
   * Remove an editor from its' workbook.
   */
  void RemoveEditor(PartPane::Pointer pane);

  /**
   * @see IPersistablePart
   */
  bool RestoreState(IMemento::Pointer memento);

  /**
   * @see IPersistablePart
   */
  bool SaveState(IMemento::Pointer memento);

  /**
   * Set the editor workbook which is active.
   */
  void SetActiveWorkbook(PartStack::Pointer newWorkbook, bool hasFocus);

  /**
   * Set the editor workbook which is active.
   */
  void SetActiveWorkbookFromID(const std::string& id);

  PartStack::Pointer GetWorkbookFromID(const std::string& id);

  /**
   * Updates the editor area's tab list to include the active
   * editor and its tab.
   */
  void UpdateTabList();

  /**
   * @see org.opencherry.ui.internal.LayoutPart#createControl(org.opencherry.swt.widgets.Composite)
   */
  void CreateControl(void* parent);

  /**
   * @see org.opencherry.ui.internal.LayoutPart#getImportance()
   */
  bool IsCompressible();

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.PartSashContainer#isStackType(org.opencherry.ui.internal.LayoutPart)
   */
  bool IsStackType(IStackableContainer::Pointer toTest);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.PartSashContainer#isPaneType(org.opencherry.ui.internal.LayoutPart)
   */
  bool IsPaneType(StackablePart::Pointer toTest);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.PartSashContainer#pickPartToZoom()
   */
  //     LayoutPart pickPartToZoom() {
  //        return getActiveWorkbook();
  //    }

  /**
   * Restore the presentation state.  Loop over the workbooks, create the appropriate serializer and pass to the presentation.
   *
   * @param areaMem the memento containing presentation
   * @return the restoration status
   */
  bool RestorePresentationState(IMemento::Pointer areaMem);

};

}

#endif /* CHERRYEDITORSASHCONTAINER_H_ */
