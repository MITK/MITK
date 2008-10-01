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

#ifndef CHERRYDETACHEDWINDOW_H_
#define CHERRYDETACHEDWINDOW_H_

#include "cherryPartStack.h"
#include "cherryWorkbenchPage.h"
#include "cherryLayoutPart.h"

#include "../cherryRectangle.h"
#include "../cherryShell.h"

namespace cherry
{

/**
 * TODO: Drag from detached to fast view bar back to detached causes NPE
 *
 * @since 3.1
 */
class DetachedWindow : public IPropertyChangeListener, public virtual Object //: public IDragOverListener
{

public:
  cherryClassMacro(DetachedWindow);

private:
  PartStack::Pointer folder;

  WorkbenchPage::Pointer page;

  Rectangle bounds;

  Shell::Pointer windowShell;

  bool hideViewsOnClose;

  //    ShellListener shellListener = new ShellAdapter() {
  //        public void shellClosed(ShellEvent e) {
  //          // only continue to close if the handleClose
  //          // wasn't canceled
  //            e.doit = handleClose();
  //        }
  //    };
  //
  //    Listener resizeListener = new Listener() {
  //        public void handleEvent(Event event) {
  //            Shell shell = (Shell) event.widget;
  //            folder.setBounds(shell.getClientArea());
  //        }
  //    };
  //
  //    Listener activationListener = new Listener() {
  //        public void handleEvent(Event event) {
  //          switch (event.type) {
  //          case SWT.Activate:
  //            page.window.liftRestrictions();
  //            break;
  //          case SWT.Deactivate:
  //            page.window.imposeRestrictions();
  //            break;
  //          }
  //        }
  //    };
  //
  //    IPropertyListener propertyListener = new IPropertyListener() {
  //        public void propertyChanged(Object source, int propId) {
  //            if (propId == PartStack.PROP_SELECTION) {
  //                activePartChanged(getPartReference(folder.getSelection()));
  //            }
  //        }
  //    };

  IWorkbenchPartReference::Pointer activePart;

public:

  /**
   * Create a new FloatingWindow.
   */
  DetachedWindow(WorkbenchPage::Pointer workbenchPage);

  void PropertyChange(Object::Pointer source, int propId);

  Shell::Pointer GetShell();

  void Create();

  /**
   * Adds a visual part to this window.
   * Supports reparenting.
   */
  void Add(StackablePart::Pointer part);

  bool BelongsToWorkbenchPage(IWorkbenchPage::Pointer workbenchPage);

  bool Close();

  //TODO DND
  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.dnd.IDragOverListener#drag(org.opencherry.swt.widgets.Control, java.lang.Object, org.opencherry.swt.graphics.Point, org.opencherry.swt.graphics.Rectangle)
   */
  //        IDropTarget drag(Control currentControl, Object draggedObject,
  //                Point position, Rectangle dragRectangle) {
  //
  //            if (!(draggedObject instanceof PartPane)) {
  //                return null;
  //            }
  //
  //            final PartPane sourcePart = (PartPane) draggedObject;
  //
  //            if (sourcePart.getWorkbenchWindow() != page.getWorkbenchWindow()) {
  //                return null;
  //            }
  //
  //            // Only handle the event if the source part is acceptable to the particular PartStack
  //            IDropTarget target = null;
  //            if (folder.allowsDrop(sourcePart)) {
  //              target = folder.getDropTarget(draggedObject, position);
  //
  //              if (target == null) {
  //                Rectangle displayBounds = DragUtil.getDisplayBounds(folder.getControl());
  //                if (displayBounds.contains(position)) {
  //                    target = folder.createDropTarget(sourcePart, new StackDropResult(displayBounds, null));
  //                } else {
  //                    return null;
  //                }
  //              }
  //            }
  //
  //            return target;
  //        }

  IStackableContainer::ChildrenType GetChildren() const;

  WorkbenchPage::Pointer GetWorkbenchPage();

  /**
   * @see IPersistablePart
   */
  void RestoreState(IMemento::Pointer memento);

  /**
   * @see IPersistablePart
   */
  void SaveState(IMemento::Pointer memento);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.IWorkbenchDragDropPart#getControl()
   */
  void* GetControl();

  /**
   * Opens the detached window.
   */
  int Open();

protected:

  void ActivePartChanged(IWorkbenchPartReference::Pointer partReference);

  /**
   * This method will be called to initialize the given Shell's layout
   */
  void ConfigureShell(Shell::Pointer shell);

  /**
   * Override this method to create the widget tree that is used as the window's contents.
   */
  void* CreateContents(void* parent);

private:

  void UpdateTitle();

  /**
   * Ensure that the shell's minimum size is equal to the minimum size
   * of the first part added to the shell.
   */
  void UpdateMinimumSize();

  static IWorkbenchPartReference::Pointer GetPartReference(
      PartPane::Pointer pane);

  /**
   * Closes this window and disposes its shell.
   */
  bool HandleClose();

  /**
   * Prompts for and handles the saving of dirty, saveable views
   * @param views The list of ViewPanes
   * @return <code>true</code> unless the user cancels the save(s)
   */
  bool HandleSaves(std::list<PartPane::Pointer> views);

  /**
   * Answer a list of the view panes.
   */
  void CollectViewPanes(std::list<PartPane::Pointer>& result,
      const std::list<StackablePart::Pointer>& parts);

};

}

#endif /* CHERRYDETACHEDWINDOW_H_ */
