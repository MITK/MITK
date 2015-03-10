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

#ifndef BERRYPARTSTACK_H_
#define BERRYPARTSTACK_H_

#include "berryLayoutPart.h"
#include "berryILayoutContainer.h"
#include "berryWorkbenchPage.h"
#include "berryPresentablePart.h"
#include "berryPartPlaceholder.h"
#include "berryDefaultStackPresentationSite.h"
#include "berryPresentationFactoryUtil.h"
#include "berryAbstractDropTarget.h"

#include "berryPartPane.h"
#include "berryIMemento.h"

#include "presentations/berryIPresentationFactory.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui_internal
 *
 * Implements the common behavior for stacks of Panes (ie: EditorStack and ViewStack)
 * This layout container has PartPanes as children and belongs to a PartSashContainer.
 *
 * @since 3.0
 */
class PartStack : public LayoutPart, public ILayoutContainer {

  friend class EditorSashContainer;
  friend class PartSashContainer;
  friend class DetachedWindow;

public:

  berryObjectMacro(PartStack)

    public: static const int PROP_SELECTION; // = 0x42;

    private: typedef QList<LayoutPart::Pointer> ChildVector;
    private: ChildVector children;

    private: WorkbenchPage* page;

    private: bool isActive;

    private: bool allowStateChanges;

    private: typedef QList<IPresentablePart::Pointer> PresentableVector;
    private: PresentableVector presentableParts;

    private: QHash<QString, QString> properties;

    protected: int appearance;

    /**
     * Stores the last value passed to setSelection. If UI updates are being deferred,
     * this may be significantly different from the other current pointers. Once UI updates
     * are re-enabled, the stack will update the presentation selection to match the requested
     * current pointer.
     */
    private: LayoutPart::Pointer requestedCurrent;

    /**
     * Stores the current part for the stack. Whenever the outside world asks a PartStack
     * for the current part, this is what gets returned. This pointer is only updated after
     * the presentation selection has been restored and the stack has finished updating its
     * internal state. If the stack is still in the process of updating the presentation,
     * it will still point to the previous part until the presentation is up-to-date.
     */
    private: LayoutPart::Pointer current;

    /**
     * Stores the presentable part sent to the presentation. Whenever the presentation
     * asks for the current part, this is what gets returned. This is updated before sending
     * the part to the presentation, and it is not updated while UI updates are disabled.
     * When UI updates are enabled, the stack first makes presentationCurrent match
     * requestedCurrent. Once the presentation is displaying the correct part, the "current"
     * pointer on PartStack is updated.
     */
    private: PresentablePart::Pointer presentationCurrent;

    private: bool ignoreSelectionChanges;

    protected: IMemento::Pointer savedPresentationState;

    protected:

      class MyStackPresentationSite : public DefaultStackPresentationSite {

      private:

        PartStack* partStack;

        public:

          MyStackPresentationSite(PartStack* stack);

          void Close(IPresentablePart::Pointer part);

         void Close(const QList<IPresentablePart::Pointer>& parts);

         void DragStart(IPresentablePart::Pointer beingDragged,
                QPoint& initialLocation, bool keyboard);

         void DragStart(QPoint& initialLocation, bool keyboard);

         bool IsPartMoveable(IPresentablePart::Pointer part);

         void SelectPart(IPresentablePart::Pointer toSelect);

         bool SupportsState(int state);

         void SetState(int newState);

         IPresentablePart::Pointer GetSelectedPart();

//         void AddSystemActions(IMenuManager menuManager) {
//            PartStack.this.addSystemActions(menuManager);
//        }

         bool IsStackMoveable();

         void FlushLayout();

         PresentableVector GetPartList();

         QString GetProperty(const QString& id);
    };

      DefaultStackPresentationSite::Pointer presentationSite;

    private:

      class PartStackDropResult : public AbstractDropTarget {

        private:

          PartPane::WeakPtr pane;

        // Result of the presentation's dragOver method or null if we are stacking over the
        // client area of the pane.
         StackDropResult::Pointer dropResult;
         PartStack::WeakPtr stack;

        public:

          berryObjectMacro(PartStackDropResult)

        /**
         * Resets the target of this drop result (allows the same drop result object to be
         * reused)
         *
         * @param stack
         * @param pane
         * @param result result of the presentation's dragOver method, or null if we are
         * simply stacking anywhere.
         * @since 3.1
         */
        void SetTarget(PartStack::Pointer stack, PartPane::Pointer pane, StackDropResult::Pointer result);

        void Drop();

        CursorType GetCursor();

        QRect GetSnapRectangle();
    };

    private: static PartStackDropResult::Pointer dropResult;

//    protected: bool isMinimized;

    private: IPropertyChangeListener::Events propEvents;

    /**
     * Custom presentation factory to use for this stack, or null to
     * use the default
     */
    private: IPresentationFactory* factory;

//  private: boolean smartZoomed = false;
//  private: boolean doingUnzoom = false;

    protected: virtual bool IsMoveable(IPresentablePart::Pointer part);

//    protected: abstract void addSystemActions(IMenuManager menuManager);

    protected: virtual bool SupportsState(int newState);

    protected: virtual bool CanMoveFolder();

    protected: virtual void DerefPart(LayoutPart::Pointer toDeref);

    protected: virtual bool AllowsDrop(PartPane::Pointer part);

//    protected: static void appendToGroupIfPossible(IMenuManager m,
//            String groupId, ContributionItem item) {
//        try {
//            m.appendToGroup(groupId, item);
//        } catch (IllegalArgumentException e) {
//            m.add(item);
//        }
//    }

    /**
     * Creates a new part stack that uses the given custom presentation factory
     * @param appearance
     * @param factory custom factory to use (or null to use the default)
     */
    public: PartStack(WorkbenchPage* page,
                      bool allowsStateChanges = true,
                      int appearance = PresentationFactoryUtil::ROLE_VIEW,
                      IPresentationFactory* factory = 0);

    /**
     * Adds a property listener to this stack. The listener will receive a PROP_SELECTION
     * event whenever the result of getSelection changes
     *
     * @param listener
     */
    public: void AddListener(IPropertyChangeListener* listener);

    public: void RemoveListener(IPropertyChangeListener* listener);

    public: int GetAppearance() const;

    public: QString GetID() const;

    protected: bool IsStandalone();

    /**
     * Returns the currently selected IPresentablePart, or null if none
     *
     * @return
     */
    protected: IPresentablePart::Pointer GetSelectedPart();

    protected: IStackPresentationSite::Pointer GetPresentationSite();

    /**
     * Tests the integrity of this object. Throws an exception if the object's state
     * is invalid. For use in test suites.
     */
    public: void TestInvariants();

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.LayoutPart#describeLayout(java.lang.StringBuffer)
     */
    public: void DescribeLayout(QString& buf) const;

    /**
     * See IVisualContainer#add
     */
    public: void Add(LayoutPart::Pointer child);

    /**
     * Add a part at a particular position
     */
    protected: void Add(LayoutPart::Pointer newChild, Object::Pointer cookie);

    public: bool AllowsAdd(LayoutPart::Pointer toAdd);

    /*
     * (non-Javadoc)
     *
     * @see org.blueberry.ui.internal.ILayoutContainer#allowsAutoFocus()
     */
    public: bool AllowsAutoFocus();

    /**
     * @param parts
     */
    protected: void Close(const QList<IPresentablePart::Pointer>& parts);

    /**
     * @param part
     */
    protected: void Close(IPresentablePart::Pointer part);

    protected: IPresentationFactory* GetFactory();

    public: void CreateControl(QWidget* parent);

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.LayoutPart#getDropTarget(java.lang.Object, org.blueberry.swt.graphics.QPoint)
     */
    public: IDropTarget::Pointer GetDropTarget(Object::Pointer draggedObject, const QPoint& position);

    public: void SetActive(bool isActive);

    public: IDropTarget::Pointer CreateDropTarget(PartPane::Pointer pane, StackDropResult::Pointer result);

    /**
     * Saves the current state of the presentation to savedPresentationState, if the
     * presentation exists.
     */
    protected: void SavePresentationState();

    public: ~PartStack();

    /**
     * See LayoutPart#Dispose
     */
    public: void Dispose();

    public: void FindSashes(LayoutPart::Pointer toFind, PartPane::Sashes& sashes);

    /**
     * Gets the presentation bounds.
     */
    public: QRect GetBounds();

    /**
     * See IVisualContainer#getChildren
     */
    public: ChildVector GetChildren() const;

    public: QWidget* GetControl();

    /**
     * Answer the number of children.
     */
    public: ChildVector::size_type GetItemCount();

    /**
     * Returns the LayoutPart for the given IPresentablePart, or null if the given
     * IPresentablePart is not in this stack. Returns null if given a null argument.
     *
     * @param part to locate or null
     * @return
     */
    protected: PartPane::Pointer GetPaneFor(IPresentablePart::Pointer part);

    /**
     * Get the parent control.
     */
    public: QWidget* GetParent();

    /**
     * Returns a list of IPresentablePart
     *
     * @return
     */
    public: PresentableVector GetPresentableParts();

    private: PresentablePart::Pointer GetPresentablePart(LayoutPart::Pointer pane);

    protected: StackPresentation::Pointer GetPresentation();

    /**
     * Returns the visible child.
     * @return the currently visible part, or null if none
     */
    public: PartPane::Pointer GetSelection();

    private: void PresentationSelectionChanged(IPresentablePart::Pointer newSelection);

    /**
     * See IVisualContainer#remove
     */
    public: void Remove(LayoutPart::Pointer child);

    /**
     * Reparent a part. Also reparent visible children...
     */
    public: void Reparent(QWidget* newParent);

    /**
     * See IVisualContainer#replace
     */
    public: void Replace(LayoutPart::Pointer oldChild, LayoutPart::Pointer newChild);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.LayoutPart#computePreferredSize(boolean, int, int, int)
   */
  public: int ComputePreferredSize(bool width, int availableParallel,
      int availablePerpendicular, int preferredParallel);

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.LayoutPart#getSizeFlags(boolean)
     */
    public: int GetSizeFlags(bool horizontal);

    /**
     * @see IPersistable
     */
    public: bool RestoreState(IMemento::Pointer memento);

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.LayoutPart#setVisible(boolean)
     */
    public: void SetVisible(bool makeVisible);

    /**
     * @see IPersistable
     */
    public: bool SaveState(IMemento::Pointer memento);

    protected: WorkbenchPage::Pointer GetPage();

    /**
     * Set the active appearence on the tab folder.
     *
     * @param active
     */
    public: void SetActive(int activeState);

    public: int GetActive() const;

    public: void CreateControl(QWidget* parent, StackPresentation::Pointer presentation);

    /**
     * Sets the presentation bounds.
     */
    public: void SetBounds(const QRect& r);

    public: void SetSelection(LayoutPart::Pointer part);

    /**
     * Updates the enablement state of actions
     */
    protected: virtual void UpdateActions(PresentablePart::Pointer current);

    /* (non-Javadoc)
   * @see org.blueberry.ui.internal.LayoutPart#handleDeferredEvents()
   */
  protected: void HandleDeferredEvents();

    private: void RefreshPresentationSelection();

    public: int GetState();

  /**
   * Sets the minimized state for this stack. The part may call this method to
   * minimize or restore itself. The minimized state only affects the view
   * when unzoomed in the 3.0 presentation (in 3.3 it's handled by the
   * ViewStack directly and works as expected).
   */
//  public: void setMinimized(boolean minimized) {
//    if (minimized != isMinimized) {
//      isMinimized = minimized;
//
//      refreshPresentationState();
//    }
//  }

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.ILayoutContainer#obscuredByZoom(org.blueberry.ui.internal.LayoutPart)
     */
//    public: boolean childObscuredByZoom(LayoutPart toTest) {
//        return isObscuredByZoom();
//    }

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.LayoutPart#requestZoom(org.blueberry.ui.internal.LayoutPart)
     */
//    public: void childRequestZoomIn(LayoutPart toZoom) {
//        super.childRequestZoomIn(toZoom);
//
//        requestZoomIn();
//    }
    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.LayoutPart#requestZoomOut()
     */
//    public: void childRequestZoomOut() {
//        super.childRequestZoomOut();
//
//        requestZoomOut();
//    }

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.ILayoutContainer#isZoomed(org.blueberry.ui.internal.LayoutPart)
     */
//    public: boolean childIsZoomed(LayoutPart toTest) {
//        return isZoomed();
//    }

    /**
     * This is a hack that allows us to preserve the old
     * min/max behavior for the stack containing the IntroPart.
     * This is required to have the initial Intro (Welcome)
     * pane to show correctly but will induce strange
     * effects should a user re-locate the part to
     * stacks other that its initial one...
     *
     * @return true if the stack contains the intro
     * as a ViewPane (not if it's only a placeholder)
     */
//    private: boolean isIntroInStack() {
//      LayoutPart[] kids = getChildren();
//      for (int i = 0; i < kids.length; i++) {
//        if (kids[i] instanceof ViewPane) {
//          ViewPane vp = (ViewPane) kids[i];
//          if (vp.getID().equals(IIntroConstants.INTRO_VIEW_ID))
//            return true;
//        }
//    }
//      return false;
//    }

//    private: void smartZoom() {
//    WorkbenchWindow wbw = (WorkbenchWindow) getPage().getWorkbenchWindow();
//    if (wbw == null || wbw.getShell() == null)
//      return;
//
//    Perspective perspective = getPage().getActivePerspective();
//    FastViewManager fvm = perspective.getFastViewManager();
//
//    fvm.deferUpdates(true);
//
//    // Cache the layout bounds
//    perspective.getPresentation().updateBoundsMap();
//
//    LayoutPart[] children = perspective.getPresentation().getLayout().getChildren();
//    for (int i = 0; i < children.length; i++) {
//      if (children[i] != this) {
//        if (children[i] instanceof ViewStack) {
//          ((ViewStack) children[i]).setMinimized(true);
//          ViewStackTrimToolBar vstb = fvm
//              .getViewStackTrimToolbar(children[i]
//                  .getID());
//          vstb.setRestoreOnUnzoom(true);
//        }
//        else if (children[i] instanceof EditorSashContainer && !(this instanceof EditorStack)) {
//          perspective.setEditorAreaState(IStackPresentationSite.STATE_MINIMIZED);
//          perspective.setEditorAreaRestoreOnUnzoom(true);
//        }
//      }
//    }
//
//    // If the editor area has changed state tell the perspective
//    if (this instanceof EditorStack)
//      perspective.setEditorAreaState(IStackPresentationSite.STATE_MAXIMIZED);
//
//    // Clear the boundsMap
//    perspective.getPresentation().resetBoundsMap();
//
//    // We're done batching...
//    fvm.deferUpdates(false);
//
//    perspective.getPresentation().setMaximizedStack(this);
//    smartZoomed = true;
//    }

//    protected: void smartUnzoom() {
//      // Prevent recursion through 'setMinimized'
//      if (doingUnzoom)
//        return;
//      doingUnzoom = true;
//
//    WorkbenchWindow wbw = (WorkbenchWindow) getPage().getWorkbenchWindow();
//    if (wbw == null || wbw.getShell() == null)
//      return;
//
//    ITrimManager tbm = wbw.getTrimManager();
//    Perspective perspective = getPage().getActivePerspective();
//    FastViewManager fvm = perspective.getFastViewManager();
//
//    ILayoutContainer root = getContainer();
//
//    // We go up one more level when maximizing an editor stack
//    // so that we 'zoom' the editor area
//    boolean restoringEditorArea = false;
//    if (root instanceof EditorSashContainer) {
//      root = ((EditorSashContainer) root).getContainer();
//      restoringEditorArea = true;
//    }
//
//    // This is a compound operation
//    fvm.deferUpdates(true);
//
//    LayoutPart[] children = root.getChildren();
//    for (int i = 0; i < children.length; i++) {
//      if (children[i] != this) {
//        IWindowTrim trim = tbm.getTrim(children[i].getID());
//        if (trim == null)
//          continue;
//
//        if (trim instanceof ViewStackTrimToolBar) {
//          ViewStackTrimToolBar vstb = (ViewStackTrimToolBar) trim;
//          if (vstb.restoreOnUnzoom()
//              && children[i] instanceof ContainerPlaceholder) {
//            // In the current presentation its a
//            // container placeholder
//            ViewStack realStack = (ViewStack) ((ContainerPlaceholder) children[i])
//                .getRealContainer();
//            realStack.setMinimized(false);
//
//            vstb.setRestoreOnUnzoom(false);
//          }
//        } else if (trim instanceof EditorAreaTrimToolBar) {
//          if (perspective.getEditorAreaRestoreOnUnzoom())
//          perspective.setEditorAreaState(IStackPresentationSite.STATE_RESTORED);
//        }
//      }
//    }
//
//    // If the editor area has changed state tell the perspective
//    if (restoringEditorArea)
//      perspective.setEditorAreaState(IStackPresentationSite.STATE_RESTORED);
//
//    perspective.getPresentation().setMaximizedStack(null);
//
//    fvm.deferUpdates(false);
//    smartZoomed = false;
//
//    doingUnzoom = false;
//    }

  protected: void SetState(const int newState);


    /**
     * Called by the workbench page to notify this part that it has been zoomed or unzoomed.
     * The PartStack should not call this method itself -- it must request zoom changes by
     * talking to the WorkbenchPage.
     */
//    public: void setZoomed(boolean isZoomed) {
//
//        super.setZoomed(isZoomed);
//
//        LayoutPart[] children = getChildren();
//
//        for (int i = 0; i < children.length; i++) {
//            LayoutPart next = children[i];
//
//            next.setZoomed(isZoomed);
//        }
//
//        refreshPresentationState();
//    }

//    public: boolean isZoomed() {
//        ILayoutContainer container = getContainer();
//
//        if (container != null) {
//            return container.childIsZoomed(this);
//        }
//
//        return false;
//    }

//    protected: void refreshPresentationState() {
//        if (isZoomed() || smartZoomed) {
//            presentationSite.setPresentationState(IStackPresentationSite.STATE_MAXIMIZED);
//        } else {
//
//            boolean wasMinimized = (presentationSite.getState() == IStackPresentationSite.STATE_MINIMIZED);
//
//            if (isMinimized) {
//                presentationSite.setPresentationState(IStackPresentationSite.STATE_MINIMIZED);
//            } else {
//                presentationSite.setPresentationState(IStackPresentationSite.STATE_RESTORED);
//            }
//
//            if (isMinimized != wasMinimized) {
//                flushLayout();
//
//                if (isMinimized) {
//                  WorkbenchPage page = getPage();
//
//                  if (page != null) {
//                      page.refreshActiveView();
//                  }
//                }
//            }
//        }
//    }

    /**
     * Makes the given part visible in the presentation.
     * @param part the part to add to the stack
     * @param cookie other information
     */
    private: void ShowPart(LayoutPart::Pointer part, Object::Pointer cookie);

    /**
   * Update the container to show the correct visible tab based on the
   * activation list.
   */
    private: void UpdateContainerVisibleTab();

    /**
     *
     */
    public: void ShowSystemMenu();

    public: void ShowPaneMenu();

    public: void ShowPartList();

    public: QList<QWidget*> GetTabList(LayoutPart::Pointer part);

    /**
     *
     * @param beingDragged
     * @param initialLocation
     * @param keyboard
     */
    private: void DragStart(IPresentablePart::Pointer beingDragged, QPoint& initialLocation,
            bool keyboard);

    public: void PaneDragStart(PartPane::Pointer pane, QPoint& initialLocation,
            bool keyboard);

    /**
     * @return Returns the savedPresentationState.
     */
    public: IMemento::Pointer GetSavedPresentationState();

    private: void FireInternalPropertyChange(int id);

    // TrimStack Support

    /**
     * Explicitly sets the presentation state. This is used by the
     * new min/max code to force the CTabFolder to show the proper
     * state without going through the 'setState' code (which causes
     * nasty side-effects.
     * @param newState The state to set the presentation to
     */
//    public: void setPresentationState(int newState) {
//      presentationSite.setPresentationState(newState);
//    }

    //
    // Support for passing perspective layout properties to the presentation


    public: QString GetProperty(const QString& id);

    public: void SetProperty(const QString& id, const QString& value);

    /**
     * Copies all appearance related data from this stack to the given stack.
     */
    public: void CopyAppearanceProperties(PartStack::Pointer copyTo);

    public: void ResizeChild(LayoutPart::Pointer childThatChanged);
};

}

#endif /*BERRYPARTSTACK_H_*/
