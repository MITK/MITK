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

#ifndef CHERRYPARTPANE_H_
#define CHERRYPARTPANE_H_

#include "internal/cherryWorkbenchPartReference.h"
#include "internal/cherryStackablePart.h"

#include "cherryUiDll.h"
#include "cherryRectangle.h"
#include "cherryIPropertyChangeListener.h"

#include "guitk/cherryGuiTkIControlListener.h"

namespace cherry {

class WorkbenchPage;
struct IStackableContainer;

/**
 * Provides the common behavior for both views
 * and editor panes.
 *
 */
class CHERRY_UI PartPane : public StackablePart,
                           public IPropertyChangeListener,
                           public GuiTk::IControlListener
{

public:
  cherryClassMacro(PartPane);

  friend class PartSashContainer;
  friend class EditorSashContainer;
  friend class WorkbenchPage;
  friend struct IStackableContainer;
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

    protected: IWorkbenchPartReference::Pointer partReference;

    protected: SmartPointer<WorkbenchPage> page;

    protected: void* control;

    private: bool inLayout;

//    private: TraverseListener traverseListener = new TraverseListener() {
//        /* (non-Javadoc)
//         * @see org.opencherry.swt.events.TraverseListener#keyTraversed(org.opencherry.swt.events.TraverseEvent)
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

  //private: SmartPointer<PartStack> partStack;

  protected:
   /*static*/ class Sashes {
        public:
          Sashes();

          /*Sash*/ void* left;
          /*Sash*/ void* right;
          /*Sash*/ void* top;
          /*Sash*/ void* bottom;
    };

    /**
     * Construct a pane for a part.
     */
    public: PartPane(IWorkbenchPartReference::Pointer partReference,
            SmartPointer<WorkbenchPage> workbenchPage);

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
     * IWorkbenchPart::CreatePartControl(void*)
     */
    public: virtual void CreateControl(void* parent);

    //public: virtual void SetControlEnabled(bool enabled) = 0;

    /**
     * Create a title bar for the pane if required.
     */
   // protected: virtual void CreateTitleBar() = 0;

    public: bool IsPlaceHolder();
    /**
     * @private:
     */
    public: virtual ~PartPane();

    /**
     * User has requested to close the pane.
     * Take appropriate action depending on type.
     */
    public: void DoHide();

    protected: Rectangle GetParentBounds();

    /**
     * Get the control.
     */
    public: void* GetControl();

    /**
     * Answer the part child.
     */
    public: IWorkbenchPartReference::Pointer GetPartReference() const;

    /**
     * @see GuiTk::IControlListener
     */
    public: void ControlActivated(GuiTk::ControlEvent::Pointer e);


    /**
     * Move the control over another one.
     */
    public: void MoveAbove(void* refControl);

    /**
     * Notify the workbook page that the part pane has
     * been activated by the user.
     */
    public: void RequestActivation();

    /**
     * Shows the receiver if <code>visible</code> is true otherwise hide it.
     */
    public: void SetVisible(bool makeVisible);
    public: virtual bool GetVisible();

    /**
     * Sets focus to this part.
     */
    public: void SetFocus();

    /**
     * Sets the workbench page of the view.
     */
    public: void SetWorkbenchPage(SmartPointer<WorkbenchPage> workbenchPage);

    public: void Reparent(void* newParent);

    /**
     * Informs the pane that it's window shell has
     * been activated.
     */
   // /* package */virtual void shellActivated() = 0;

    /**
     * Informs the pane that it's window shell has
     * been deactivated.
     */
   // /* package */virtual void shellDeactivated() = 0;

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
    //public: SmartPointer<PartStack> GetStack();

    public: void SetContainer(SmartPointer<IStackableContainer> stack);

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
    public: virtual void* GetToolBar();

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
    public: void DescribeLayout(std::string& buf) const;

    /**
     * @return
     * @since 3.1
     */
    public: bool IsCloseable();

    public: void SetInLayout(bool inLayout);

    public: bool GetInLayout();

    public: bool AllowsAutoFocus();

    /**
     * Clears all contribution items from the contribution managers (this is done separately
     * from dispose() since it is done after the part is disposed). This is a bit of a hack.
     * Really, the contribution managers should be part of the site, not the PartPane. If these
     * were moved elsewhere, then disposal of the PartPane would be atomic and this method could
     * be removed.
     */
  public: virtual void RemoveContributions();

  public: void AddPropertyListener(IPropertyChangeListener::Pointer listener);

  public: void RemovePropertyListener(IPropertyChangeListener::Pointer listener);

  public: void FirePropertyChange(PropertyChangeEvent::Pointer event);


  /* (non-Javadoc)
   * @see IPropertyChangeListener#PropertyChange(PropertyChangeEvent::Pointer)
   */
  public: void PropertyChange(PropertyChangeEvent::Pointer event);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.LayoutPart#computePreferredSize(boolean, int, int, int)
   */
  public: int ComputePreferredSize(bool width, int availableParallel,
          int availablePerpendicular, int preferredParallel);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.LayoutPart#getSizeFlags(boolean)
   */
  public: int GetSizeFlags(bool horizontal);

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

#endif /*CHERRYPARTPANE_H_*/
