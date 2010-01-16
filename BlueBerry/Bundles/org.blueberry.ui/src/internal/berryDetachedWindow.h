/*=========================================================================

 Program:   BlueBerry Platform
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

#ifndef BERRYDETACHEDWINDOW_H_
#define BERRYDETACHEDWINDOW_H_

#include "berryPartStack.h"
#include "berryWorkbenchPage.h"
#include "berryLayoutPart.h"
#include "berryIDragOverListener.h"
#include "../berryIShellListener.h"
#include "../guitk/berryGuiTkIControlListener.h"

#include "../berryRectangle.h"
#include "../berryShell.h"

namespace berry
{

/**
 * TODO: Drag from detached to fast view bar back to detached causes NPE
 *
 * @since 3.1
 */
class DetachedWindow: public IPropertyChangeListener,
    public IDragOverListener
{

public:
  berryObjectMacro(DetachedWindow);

private:
  PartStack::Pointer folder;

  WorkbenchPage::Pointer page;

  Rectangle bounds;

  Shell::Pointer windowShell;

  bool hideViewsOnClose;

  struct ShellListener: public IShellListener
  {
    ShellListener(DetachedWindow* wnd);

    void ShellClosed(ShellEvent::Pointer e);

  private:
    DetachedWindow* window;
  };

  IShellListener::Pointer shellListener;

  struct ShellControlListener: public GuiTk::IControlListener
  {

    ShellControlListener(DetachedWindow* wnd);

    Events::Types GetEventTypes() const;

    void ControlResized(GuiTk::ControlEvent::Pointer e);

  private:
    DetachedWindow* window;
  };

  GuiTk::IControlListener::Pointer resizeListener;

  //      Listener activationListener = new Listener() {
  //          public void handleEvent(Event event) {
  //            switch (event.type) {
  //            case SWT.Activate:
  //              page.window.liftRestrictions();
  //              break;
  //            case SWT.Deactivate:
  //              page.window.imposeRestrictions();
  //              break;
  //            }
  //          }
  //      };

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

  /*
   * @see org.blueberry.ui.internal.IDragOverListener#Drag(void*, Object::Pointer, const Point&, const Rectangle& )
   */
  IDropTarget::Pointer Drag(void* currentControl,
      Object::Pointer draggedObject, const Point& position,
      const Rectangle& dragRectangle);

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
      StackablePart::Pointer pane);

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

#endif /* BERRYDETACHEDWINDOW_H_ */
