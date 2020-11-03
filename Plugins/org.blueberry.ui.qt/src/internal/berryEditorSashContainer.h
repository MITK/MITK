/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEDITORSASHCONTAINER_H_
#define BERRYEDITORSASHCONTAINER_H_

#include "berryPartSashContainer.h"
#include "berryPartStack.h"

#include <list>

namespace berry
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
  berryObjectMacro(EditorSashContainer);

private:

  QList<PartStack::Pointer> editorWorkbooks;

  PartStack::Pointer activeEditorWorkbook;

  // DropTarget dropTarget;

  void AddDropSupport();

  PartStack::Pointer NewEditorWorkbook();

protected:

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.PartSashContainer#addChild(org.blueberry.ui.internal.PartSashContainer.RelationshipInfo)
   */
  void AddChild(const RelationshipInfo& info) override;

  /**
   * Notification that a child layout part has been
   * added to the container. Subclasses may override
   * this method to perform any container specific
   * work.
   */
  void ChildAdded(LayoutPart::Pointer child) override;

  /**
   * Notification that a child layout part has been
   * removed from the container. Subclasses may override
   * this method to perform any container specific
   * work.
   */
  void ChildRemoved(LayoutPart::Pointer child) override;

  PartStack::Pointer CreateDefaultWorkbook();

  /**
   * Subclasses override this method to specify
   * the composite to use to parent all children
   * layout parts it contains.
   */
  QWidget* CreateParent(QWidget* parentWidget) override;

  /**
   * Subclasses override this method to dispose
   * of any swt resources created during createParent.
   */
  void DisposeParent() override;

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
   * @see org.blueberry.ui.internal.PartSashContainer#createStack(org.blueberry.ui.internal.LayoutPart)
   */
  PartStack::Pointer CreateStack() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.PartSashContainer#setVisiblePart(org.blueberry.ui.internal.ILayoutContainer, org.blueberry.ui.internal.LayoutPart)
   */
  void SetVisiblePart(ILayoutContainer::Pointer container,
      PartPane::Pointer visiblePart) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.PartSashContainer#getVisiblePart(org.blueberry.ui.internal.ILayoutContainer)
   */
  LayoutPart::Pointer GetVisiblePart(ILayoutContainer::Pointer container) override;

public:

  static const QString DEFAULT_WORKBOOK_ID;

  EditorSashContainer(const QString& editorId, WorkbenchPage* page,
      QWidget* parent);

  bool AllowsAdd(LayoutPart::Pointer layoutPart) override;

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
  QString GetActiveWorkbookID();

  /**
   * Return the all the editor workbooks.
   */
  QList<PartStack::Pointer> GetEditorWorkbooks();

  /**
   * Return the all the editor workbooks.
   */
  std::size_t GetEditorWorkbookCount();

  /**
   * Find the sashes around the specified part.
   */
  void FindSashes(LayoutPart::Pointer pane, PartPane::Sashes& sashes) override;

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
  void SetActiveWorkbookFromID(const QString& id);

  PartStack::Pointer GetWorkbookFromID(const QString& id);

  /**
   * Updates the editor area's tab list to include the active
   * editor and its tab.
   */
  void UpdateTabList();

  /**
   * @see org.blueberry.ui.internal.LayoutPart#createControl(org.blueberry.swt.widgets.Composite)
   */
  void CreateControl(QWidget* parent) override;

  /**
   * @see org.blueberry.ui.internal.LayoutPart#getImportance()
   */
  bool IsCompressible() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.PartSashContainer#isStackType(org.blueberry.ui.internal.LayoutPart)
   */
  bool IsStackType(ILayoutContainer::Pointer toTest) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.PartSashContainer#isPaneType(org.blueberry.ui.internal.LayoutPart)
   */
  bool IsPaneType(LayoutPart::Pointer toTest) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.PartSashContainer#pickPartToZoom()
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

#endif /* BERRYEDITORSASHCONTAINER_H_ */
