/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPARTPANE_H_
#define BERRYPARTPANE_H_

#include "berryWorkbenchPartReference.h"
#include "berryLayoutPart.h"

#include "berryIPropertyChangeListener.h"

#include "guitk/berryGuiTkIControlListener.h"

class QToolBar;

namespace berry {

class WorkbenchPage;
class PartStack;
struct ILayoutContainer;

/**
 * Provides the common behavior for both views
 * and editor panes.
 *
 */
class PartPane : public LayoutPart,
                 public IPropertyChangeListener,
                 public GuiTk::IControlListener
{

public:
  berryObjectMacro(PartPane);

  friend class PartSashContainer;
  friend class EditorSashContainer;
  friend class WorkbenchPage;
  friend struct ILayoutContainer;
  friend class PartStack;
  friend class ContainerPlaceholder;
  friend class LayoutTree;
  friend class LayoutTreeNode;
  friend class DetachedPlaceHolder;
  friend class PerspectiveHelper;

//    private: MenuManager paneMenuManager;
//    private: ListenerList listeners = new ListenerList();
//    private: ListenerList partListeners = new ListenerList();
  private: IPropertyChangeListener::Events propertyChangeEvents;

    protected: IWorkbenchPartReference::WeakPtr partReference;

    protected: WorkbenchPage* page;

    protected: QWidget* control;

    private: bool inLayout;

//    private: TraverseListener traverseListener = new TraverseListener() {
//        /* (non-Javadoc)
//         * @see org.blueberry.swt.events.TraverseListener#keyTraversed(org.blueberry.swt.events.TraverseEvent)
//         */
//        public: void keyTraversed(TraverseEvent e) {
//            // Hack: Currently, SWT sets focus whenever we call Control.traverse. This doesn't
//            // cause too much of a problem for ctrl-pgup and ctrl-pgdn, but it is seriously unexpected
//            // for other traversal events. When (and if) it becomes possible to call traverse() without
//            // forcing a focus change, this if statement should be removed and ALL events should be
//            // forwarded to the container.
//            if (e.detail == SWT.TRAVERSE_PAGE_NEXT
//                    || e.detail == SWT.TRAVERSE_PAGE_PREVIOUS) {
//                ILayoutContainer container = getContainer();
//                if (container != null && container instanceof LayoutPart) {
//                    LayoutPart parent = (LayoutPart) container;
//                    Control parentControl = parent.getControl();
//                    if (parentControl != null && !parentControl.isDisposed()) {
//                        e.doit = parentControl.traverse(e.detail);
//                        if (e.doit) {
//              e.detail = SWT.TRAVERSE_NONE;
//            }
//                    }
//                }
//            }
//        }
//
//    };

  private: bool busy;

  private: bool hasFocus;

  //private: SmartPointer<PartStack> partStack;

  protected:
   /*static*/ class Sashes {
        public:
          Sashes();

          /*Sash*/ QWidget* left;
          /*Sash*/ QWidget* right;
          /*Sash*/ QWidget* top;
          /*Sash*/ QWidget* bottom;
    };

    /**
     * Construct a pane for a part.
     */
    public: PartPane(IWorkbenchPartReference::Pointer partReference,
            WorkbenchPage* workbenchPage);

//    public: void addSizeMenuItem(Menu menu, int index) {
//        //Add size menu
//        MenuItem item = new MenuItem(menu, SWT.CASCADE, index);
//        item.setText(WorkbenchMessages.PartPane_size);
//        Menu sizeMenu = new Menu(menu);
//        item.setMenu(sizeMenu);
//        addSizeItems(sizeMenu);
//    }

    /**
     *
     * Creates the GUI-dependent container control
     * for the part widgets. This is passed to
     * IWorkbenchPart::CreatePartControl(QWidget*)
     */
    public: void CreateControl(QWidget* parent) override;

    //public: virtual void SetControlEnabled(bool enabled) = 0;

    /**
     * Create a title bar for the pane if required.
     */
   // protected: virtual void CreateTitleBar() = 0;

    /**
     * @private:
     */
    public: ~PartPane() override;

    /**
     * User has requested to close the pane.
     * Take appropriate action depending on type.
     */
    public: void DoHide();

    protected: QRect GetParentBounds();

    /**
     * Get the control.
     */
    public: QWidget* GetControl() override;

    /**
     * Answer the part child.
     */
    public: IWorkbenchPartReference::Pointer GetPartReference() const;

    /**
     * @see GuiTk::IControlListener
     */
    public: void ControlActivated(GuiTk::ControlEvent::Pointer e) override;

    /**
     * @see GuiTk::IControlListener
     */
    public: GuiTk::IControlListener::Events::Types GetEventTypes() const override;

    /**
     * Move the control over another one.
     */
    public: void MoveAbove(QWidget* refControl) override;

    /**
     * Notify the workbook page that the part pane has
     * been activated by the user.
     */
    public: void RequestActivation();

    /**
     * Shows the receiver if <code>visible</code> is true otherwise hide it.
     */
    public: void SetVisible(bool makeVisible) override;
    public: bool GetVisible() override;

    /**
     * Sets focus to this part.
     */
    public: void SetFocus() override;

    /**
     * Sets the workbench page of the view.
     */
    public: void SetWorkbenchPage(SmartPointer<WorkbenchPage> workbenchPage);

    public: void Reparent(QWidget* newParent) override;

    /**
     * Indicate focus in part.
     */
    public: void ShowFocus(bool inFocus);

    /**
     * @see IPartDropTarget::targetPartFor
     */
//    public: LayoutPart targetPartFor(LayoutPart dragSource) {
//        return this;
//    }

    /**
     * Returns the PartStack that contains this PartPane, or null if none.
     *
     * @return
     */
    public: SmartPointer<PartStack> GetStack();

    public: void SetContainer(SmartPointer<ILayoutContainer> stack) override;

    /**
     * Show a title label menu for this pane.
     */
//    public: void ShowPaneMenu() {
//        PartStack folder = getStack();
//
//        if (folder != null) {
//            folder.showPaneMenu();
//        }
//    }

    /**
     * Show the context menu for this part.
     */
//    public: void showSystemMenu() {
//        PartStack folder = getStack();
//
//        if (folder != null) {
//            folder.showSystemMenu();
//        }
//    }

    /**
     * Finds and return the sashes around this part.
     */
    protected: Sashes FindSashes();

    /**
     * Enable the user to resize this part using
     * the keyboard to move the specified sash
     */
//    protected: void moveSash(final Sash sash) {
//        moveSash(sash, this);
//    }

//    public: static void moveSash(final Sash sash,
//            final LayoutPart toGetFocusWhenDone) {
//        final KeyListener listener = new KeyAdapter() {
//            public: void keyPressed(KeyEvent e) {
//                if (e.character == SWT.ESC || e.character == '\r') {
//                    if (toGetFocusWhenDone != null) {
//            toGetFocusWhenDone.setFocus();
//          }
//                }
//            }
//        };
//        sash.addFocusListener(new FocusAdapter() {
//            public: void focusGained(FocusEvent e) {
//                sash.setBackground(sash.getDisplay().getSystemColor(
//                        SWT.COLOR_LIST_SELECTION));
//                sash.addKeyListener(listener);
//            }
//
//            public: void focusLost(FocusEvent e) {
//                sash.setBackground(null);
//                sash.removeKeyListener(listener);
//            }
//        });
//        sash.setFocus();
//
//    }

    /**
     * Add a menu item to the Size Menu
     */
//    protected: void addSizeItem(Menu sizeMenu, String labelMessage,
//            final Sash sash) {
//        MenuItem item = new MenuItem(sizeMenu, SWT.NONE);
//        item.setText(labelMessage);
//        item.addSelectionListener(new SelectionAdapter() {
//            public: void widgetSelected(SelectionEvent e) {
//                moveSash(sash);
//            }
//        });
//        item.setEnabled(!isZoomed() && sash != null);
//    }

    /**
     * Returns the workbench page of this pane.
     */
    public: SmartPointer<WorkbenchPage> GetPage();

    /**
     * Add the Left,Right,Up,Botton menu items to the Size menu.
     */
//    protected: void addSizeItems(Menu sizeMenu) {
//        Sashes sashes = findSashes();
//        addSizeItem(sizeMenu,
//                WorkbenchMessages.PartPane_sizeLeft, sashes.left);
//        addSizeItem(sizeMenu,
//                WorkbenchMessages.PartPane_sizeRight, sashes.right);
//        addSizeItem(sizeMenu,
//                WorkbenchMessages.PartPane_sizeTop, sashes.top);
//        addSizeItem(sizeMenu, WorkbenchMessages.PartPane_sizeBottom, sashes.bottom);
//    }

    /**
     * Pin this part.
     */
    protected: virtual void DoDock();

    /**
     * Set the busy state of the pane.
     */
    public: virtual void SetBusy(bool isBusy);

    /**
     * Show a highlight for the receiver if it is
     * not currently the part in the front of its
     * presentation.
     *
     */
    public: virtual void ShowHighlight();

    /**
     * @return
     */
    public: virtual QToolBar* GetToolBar();

    /**
     * @return
     */
    public: bool HasViewMenu();

    /**
     * @param location
     */
//    public: void ShowViewMenu(Point location) {
//
//    }

    public: bool IsBusy();

    /**
     * Writes a description of the layout to the given string buffer.
     * This is used for drag-drop test suites to determine if two layouts are the
     * same. Like a hash code, the description should compare as equal iff the
     * layouts are the same. However, it should be user-readable in order to
     * help debug failed tests. Although these are english readable strings,
     * they do not need to be translated.
     *
     * @param buf
     */
    public: void DescribeLayout(QString& buf) const override;

    /**
     * @return
     * @since 3.1
     */
    public: bool IsCloseable();

    public: void SetInLayout(bool inLayout);

    public: bool GetInLayout();

    public: bool AllowsAutoFocus() override;

    /**
     * Clears all contribution items from the contribution managers (this is done separately
     * from dispose() since it is done after the part is disposed). This is a bit of a hack.
     * Really, the contribution managers should be part of the site, not the PartPane. If these
     * were moved elsewhere, then disposal of the PartPane would be atomic and this method could
     * be removed.
     */
  public: virtual void RemoveContributions();

  public: void AddPropertyListener(IPropertyChangeListener* listener);

  public: void RemovePropertyListener(IPropertyChangeListener* listener);

  public: void FirePropertyChange(const PropertyChangeEvent::Pointer& event);


  /* (non-Javadoc)
   * @see IPropertyChangeListener#PropertyChange(PropertyChangeEvent::Pointer)
   */
  public:

  using IPropertyChangeListener::PropertyChange;
  void PropertyChange(const PropertyChangeEvent::Pointer& event) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.LayoutPart#computePreferredSize(boolean, int, int, int)
   */
  public: int ComputePreferredSize(bool width, int availableParallel,
          int availablePerpendicular, int preferredParallel) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.LayoutPart#getSizeFlags(boolean)
   */
  public: int GetSizeFlags(bool horizontal) override;

  /**
   * Informs the pane that it's window shell has
   * been activated.
   */
  public: virtual void ShellActivated();

  /**
   * Informs the pane that it's window shell has
   * been deactivated.
   */
  public: virtual void ShellDeactivated();

};

}

#endif /*BERRYPARTPANE_H_*/
