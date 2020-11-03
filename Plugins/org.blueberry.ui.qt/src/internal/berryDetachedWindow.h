/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYDETACHEDWINDOW_H_
#define BERRYDETACHEDWINDOW_H_

#include "berryPartStack.h"
#include "berryWorkbenchPage.h"
#include "berryLayoutPart.h"
#include "berryIDragOverListener.h"
#include "berryIShellListener.h"
#include "guitk/berryGuiTkIControlListener.h"

#include "berryShell.h"

namespace berry
{

/**
 * TODO: Drag from detached to fast view bar back to detached causes NPE
 */
class DetachedWindow: public Object, private IPropertyChangeListener,
    private IDragOverListener
{

public:
  berryObjectMacro(DetachedWindow);

private:
  PartStack::Pointer folder;

  WorkbenchPage* page;

  QRect bounds;

  Shell::Pointer windowShell;

  bool hideViewsOnClose;

  struct ShellListener: public IShellListener
  {
    ShellListener(DetachedWindow* wnd);

    void ShellClosed(const ShellEvent::Pointer& e) override;

  private:
    DetachedWindow* window;
  };

  QScopedPointer<IShellListener> shellListener;

  struct ShellControlListener: public GuiTk::IControlListener
  {

    ShellControlListener(DetachedWindow* wnd);

    Events::Types GetEventTypes() const override;

    void ControlResized(GuiTk::ControlEvent::Pointer e) override;

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
  DetachedWindow(WorkbenchPage* workbenchPage);
  using IPropertyChangeListener::PropertyChange;

  void PropertyChange(const Object::Pointer& source, int propId) override;

  Shell::Pointer GetShell();

  void Create();

  /**
   * Adds a visual part to this window.
   * Supports reparenting.
   */
  void Add(LayoutPart::Pointer part);

  bool BelongsToWorkbenchPage(IWorkbenchPage::Pointer workbenchPage);

  bool Close();

  /*
   * @see org.blueberry.ui.internal.IDragOverListener#Drag(QWidget*, Object::Pointer, const QPoint&, const QRect& )
   */
  IDropTarget::Pointer Drag(QWidget* currentControl,
      const Object::Pointer& draggedObject, const QPoint& position,
      const QRect& dragRectangle) override;

  ILayoutContainer::ChildrenType GetChildren() const;

  WorkbenchPage::Pointer GetWorkbenchPage();

  /**
   * @see IPersistablePart
   */
  void RestoreState(IMemento::Pointer memento);

  /**
   * @see IPersistablePart
   */
  void SaveState(IMemento::Pointer memento);

  QWidget* GetControl();

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
  QWidget* CreateContents(QWidget* parent);

private:

  void UpdateTitle();

  /**
   * Ensure that the shell's minimum size is equal to the minimum size
   * of the first part added to the shell.
   */
  void UpdateMinimumSize();

  static IWorkbenchPartReference::Pointer GetPartReference(
      LayoutPart::Pointer pane);

  /**
   * Closes this window and disposes its shell.
   */
  bool HandleClose();

  /**
   * Prompts for and handles the saving of dirty, saveable views
   * @param views The list of ViewPanes
   * @return <code>true</code> unless the user cancels the save(s)
   */
  bool HandleSaves(QList<PartPane::Pointer> views);

  /**
   * Answer a list of the view panes.
   */
  void CollectViewPanes(QList<PartPane::Pointer>& result,
      const QList<LayoutPart::Pointer>& parts);

};

}

#endif /* BERRYDETACHEDWINDOW_H_ */
