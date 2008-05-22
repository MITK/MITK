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

#ifndef CHERRYPARTSTACK_H_
#define CHERRYPARTSTACK_H_

#include "cherryLayoutPart.h"
#include "cherryILayoutContainer.h"
#include "cherryWorkbenchPage.h"

#include "../cherryIWorkbenchPart.h"
#include "../cherryIPartPane.h"
#include "../cherryIMemento.h"

#include <vector>
#include <map>

namespace cherry {

/**
 * \ingroup org_opencherry_ui_internal
 * 
 * Implements the common behavior for stacks of Panes (ie: EditorStack and ViewStack)
 * This layout container has PartPanes as children and belongs to a PartSashContainer.
 * 
 * @since 3.0
 */
class PartStack : public LayoutPart, public ILayoutContainer {

    public: static const int PROP_SELECTION; // = 0x42;
    
    private: std::vector<LayoutPart::Pointer> children;
    private: bool isActive;
    //private: ArrayList presentableParts = new ArrayList();
    
    private: std::map<std::string, std::string> properties;
    
    //protected: int appearance = PresentationFactoryUtil.ROLE_VIEW;
    
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
    //private: PresentablePart presentationCurrent;

    private: bool ignoreSelectionChanges;

    //protected: IMemento savedPresentationState = null;

//    protected: DefaultStackPresentationSite presentationSite = new DefaultStackPresentationSite() {
//
//        public: void close(IPresentablePart part) {
//            PartStack.this.close(part);
//        }
//
//        public: void close(IPresentablePart[] parts) {
//            PartStack.this.close(parts);
//        }
//
//        public: void dragStart(IPresentablePart beingDragged,
//                Point initialLocation, boolean keyboard) {
//            PartStack.this.dragStart(beingDragged, initialLocation, keyboard);
//        }
//
//        public: void dragStart(Point initialLocation, boolean keyboard) {
//            PartStack.this.dragStart(null, initialLocation, keyboard);
//        }
//
//        public: boolean isPartMoveable(IPresentablePart part) {
//            return PartStack.this.isMoveable(part);
//        }
//
//        public: void selectPart(IPresentablePart toSelect) {
//            PartStack.this.presentationSelectionChanged(toSelect);
//        }
//
//        public: boolean supportsState(int state) {
//            return PartStack.this.supportsState(state);
//        }
//
//        public: void setState(int newState) {
//            PartStack.this.setState(newState);
//        }
//
//        public: IPresentablePart getSelectedPart() {
//            return PartStack.this.getSelectedPart();
//        }
//
//        public: void addSystemActions(IMenuManager menuManager) {
//            PartStack.this.addSystemActions(menuManager);
//        }
//
//        public: boolean isStackMoveable() {
//            return canMoveFolder();
//        }
//        
//        public: void flushLayout() {
//          PartStack.this.flushLayout();
//        }
//
//        public: IPresentablePart[] getPartList() {
//            List parts = getPresentableParts();
//            
//            return (IPresentablePart[]) parts.toArray(new IPresentablePart[parts.size()]);
//        }
//
//        public: String getProperty(String id) {            
//            return PartStack.this.getProperty(id);
//        }
//    };

//    private: static final class PartStackDropResult extends AbstractDropTarget {
//        private: PartPane pane;
//        
//        // Result of the presentation's dragOver method or null if we are stacking over the
//        // client area of the pane.
//        private: StackDropResult dropResult;
//        private: PartStack stack;
//        
//        /**
//         * Resets the target of this drop result (allows the same drop result object to be
//         * reused)
//         * 
//         * @param stack
//         * @param pane
//         * @param result result of the presentation's dragOver method, or null if we are
//         * simply stacking anywhere.
//         * @since 3.1
//         */
//        public: void setTarget(PartStack stack, PartPane pane, StackDropResult result) {
//            this.pane = pane;
//            this.dropResult = result;
//            this.stack = stack;
//        }
//        
//        public: void drop() {
//            // If we're dragging a pane over itself do nothing
//            //if (dropResult.getInsertionPoint() == pane.getPresentablePart()) { return; };
//
//            Object cookie = null;
//            if (dropResult != null) {
//                cookie = dropResult.getCookie();
//            }
//
//            // Handle cross window drops by opening a new editor
//            if (pane instanceof EditorPane) {
//              if (pane.getWorkbenchWindow() != stack.getWorkbenchWindow()) {
//                EditorPane editor = (EditorPane) pane;
//                try {
//            IEditorInput input = editor.getEditorReference().getEditorInput();
//            
//            // Close the old editor and capture the actual closed state incase of a 'cancel'
//            boolean editorClosed = editor.getPage().closeEditor(editor.getEditorReference(), true);
//            
//            // Only open open the new editor if the old one closed
//            if (editorClosed)
//              stack.getPage().openEditor(input, editor.getEditorReference().getId());
//            return;
//          } catch (PartInitException e) {
//            e.printStackTrace();
//          }
//                
//              }
//            }
//            
//            if (pane.getContainer() != stack) {
//                // Moving from another stack
//                stack.derefPart(pane);
//                pane.reparent(stack.getParent());
//                stack.add(pane, cookie);
//                stack.setSelection(pane);
//                pane.setFocus();
//            } else if (cookie != null) {
//                // Rearranging within this stack
//                stack.getPresentation().movePart(stack.getPresentablePart(pane), cookie);
//            }
//        }
//
//        public: Cursor getCursor() {
//            return DragCursors.getCursor(DragCursors.CENTER);
//        }
//
//        public: Rectangle getSnapRectangle() {
//            if (dropResult == null) {
//                return DragUtil.getDisplayBounds(stack.getControl());
//            }
//            return dropResult.getSnapRectangle();
//        }
//    }

//    private: static final PartStackDropResult dropResult = new PartStackDropResult();

//    protected: bool isMinimized;

//    private: ListenerList listeners = new ListenerList();

    /**
     * Custom presentation factory to use for this stack, or null to
     * use the default
     */
//    private: AbstractPresentationFactory factory;

//  private: boolean smartZoomed = false;
//  private: boolean doingUnzoom = false;
            
    protected: virtual bool IsMoveable(IWorkbenchPart::Pointer part) = 0;

//    protected: abstract void addSystemActions(IMenuManager menuManager);

    protected: virtual bool SupportsState(int newState) = 0;

    protected: virtual bool CanMoveFolder() = 0;

    protected: virtual void DerefPart(LayoutPart::Pointer toDeref) = 0;

    protected: virtual bool AllowsDrop(IPartPane::Pointer part) = 0;

//    protected: static void appendToGroupIfPossible(IMenuManager m,
//            String groupId, ContributionItem item) {
//        try {
//            m.appendToGroup(groupId, item);
//        } catch (IllegalArgumentException e) {
//            m.add(item);
//        }
//    }
    
    /**
     * Creates a new PartStack, given a constant determining which presentation to use
     * 
     * @param appearance one of the PresentationFactoryUtil.ROLE_* constants
     */
    public: PartStack();
    

    /**
     * Adds a property listener to this stack. The listener will receive a PROP_SELECTION
     * event whenever the result of getSelection changes
     * 
     * @param listener
     */
//    public: void addListener(IPropertyListener listener) {
//        listeners.add(listener);
//    }
    
//    public: void removeListener(IPropertyListener listener) {
//        listeners.remove(listener);
//    }
    
    protected: bool IsStandalone();
    
    /**
     * Returns the currently selected IPresentablePart, or null if none
     * 
     * @return
     */
    protected: IWorkbenchPart::Pointer GetSelectedPart();

//    protected: IStackPresentationSite getPresentationSite() {
//        return presentationSite;
//    }

    /**
     * Tests the integrity of this object. Throws an exception if the object's state
     * is invalid. For use in test suites.
     */
    public: void TestInvariants();

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.LayoutPart#describeLayout(java.lang.StringBuffer)
     */
    public: void DescribeLayout(std::string& buf) ;

    /**
     * See IVisualContainer#add
     */
    public: void Add(LayoutPart::Pointer child);

    /**
     * Add a part at a particular position
     */
    protected: void Add(LayoutPart::Pointer newChild, Object* cookie);

    public: bool AllowsAdd(LayoutPart::Pointer toAdd);
    
    /*
     * (non-Javadoc)
     * 
     * @see org.eclipse.ui.internal.ILayoutContainer#allowsAutoFocus()
     */
    public: bool AllowsAutoFocus();

    /**
     * @param parts
     */
    protected: void Close(const std::vector<IWorkbenchPart::Pointer>& parts);

    /**
     * @param part
     */
    protected: void Close(IWorkbenchPart::Pointer part);

//    protected: AbstractPresentationFactory getFactory() {
//        
//        if (factory != null) {
//            return factory;
//        }
//        
//        return ((WorkbenchWindow) getPage()
//                .getWorkbenchWindow()).getWindowConfigurer()
//                .getPresentationFactory();
//    }
    
    public: void* CreateControl(void* parent);

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.LayoutPart#getDropTarget(java.lang.Object, org.eclipse.swt.graphics.Point)
     */
//    public: IDropTarget getDropTarget(Object draggedObject, Point position) {
//
//        if (!(draggedObject instanceof PartPane)) {
//            return null;
//        }
//
//        final PartPane pane = (PartPane) draggedObject;
//        if (isStandalone() 
//                || !allowsDrop(pane)) {
//            return null;
//        }
//
//        // Don't allow views to be dragged between windows
//        boolean differentWindows = pane.getWorkbenchWindow() != getWorkbenchWindow();
//        boolean editorDropOK = ((pane instanceof EditorPane) && 
//            pane.getWorkbenchWindow().getWorkbench() == 
//              getWorkbenchWindow().getWorkbench());
//        if (differentWindows && !editorDropOK) {
//            return null;
//        }
//
//        StackDropResult dropResult = getPresentation().dragOver(
//                getControl(), position);
//        
//        if (dropResult == null) {
//          return null;
//        }
//        
//        return createDropTarget(pane, dropResult); 
//    }
    
    public: void SetActive(bool isActive);
    

//    public: IDropTarget createDropTarget(PartPane pane, StackDropResult result) {
//        dropResult.setTarget(this, pane, result);
//        return dropResult;
//    }
    
    /**
     * Saves the current state of the presentation to savedPresentationState, if the
     * presentation exists.
     */
//    protected: void savePresentationState() {
//        if (isDisposed()) {
//            return;
//        }
//
//        {// Save the presentation's state before disposing it
//            XMLMemento memento = XMLMemento
//                    .createWriteRoot(IWorkbenchConstants.TAG_PRESENTATION);
//            memento.putString(IWorkbenchConstants.TAG_ID, getFactory().getId());
//
//            PresentationSerializer serializer = new PresentationSerializer(
//                    getPresentableParts());
//
//            getPresentation().saveState(serializer, memento);
//
//            // Store the memento in savedPresentationState
//            savedPresentationState = memento;
//        }
//    }

    /**
     * See LayoutPart#dispose
     */
    public: ~PartStack();

//    public: void findSashes(LayoutPart part, PartPane.Sashes sashes) {
//        ILayoutContainer container = getContainer();
//
//        if (container != null) {
//            container.findSashes(this, sashes);
//        }
//    }

    /**
     * Gets the presentation bounds.
     */
//    public: Rectangle getBounds() {
//        if (getPresentation() == null) {
//            return new Rectangle(0, 0, 0, 0);
//        }
//
//        return getPresentation().getControl().getBounds();
//    }

    /**
     * See IVisualContainer#getChildren
     */
    public: std::vector<LayoutPart::Pointer> GetChildren();

    public: void* GetControl();

    /**
     * Answer the number of children.
     */
    public: int GetItemCount();
    
    /**
     * Returns the LayoutPart for the given IPresentablePart, or null if the given
     * IPresentablePart is not in this stack. Returns null if given a null argument.
     * 
     * @param part to locate or null
     * @return
     */
    protected: IPartPane::Pointer GetPaneFor(IWorkbenchPart::Pointer part);

    /**
     * Get the parent control.
     */
    public: void* GetParent();

    /**
     * Returns a list of IPresentablePart
     * 
     * @return
     */
//    public: List getPresentableParts() {
//        return presentableParts;
//    }

//    private: PresentablePart getPresentablePart(LayoutPart pane) {
//        for (Iterator iter = presentableParts.iterator(); iter.hasNext();) {
//            PresentablePart part = (PresentablePart) iter.next();
//            
//            if (part.getPane() == pane) {
//                return part;
//            }
//        }
//        
//        return null;
//    }
    
//    protected: StackPresentation getPresentation() {
//        return presentationSite.getPresentation();
//    }

    /**
     * Returns the visible child.
     * @return the currently visible part, or null if none
     */
    public: IPartPane::Pointer GetSelection();

    private: void PresentationSelectionChanged(IWorkbenchPart::Pointer newSelection);

    /**
     * See IVisualContainer#remove
     */
    public: void Remove(LayoutPart::Pointer child);

    /**
     * Reparent a part. Also reparent visible children...
     */
    public: void Reparent(void* newParent);

    /**
     * See IVisualContainer#replace
     */
    public: void Replace(LayoutPart::Pointer oldChild, LayoutPart::Pointer newChild);
    
  /* (non-Javadoc)
   * @see org.eclipse.ui.internal.LayoutPart#computePreferredSize(boolean, int, int, int)
   */
//  public: int computePreferredSize(boolean width, int availableParallel,
//      int availablePerpendicular, int preferredParallel) {
//    
//    return getPresentation().computePreferredSize(width, availableParallel, 
//        availablePerpendicular, preferredParallel);
//  }
    
    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.LayoutPart#getSizeFlags(boolean)
     */
//    public: int getSizeFlags(boolean horizontal) {
//        StackPresentation presentation = getPresentation();
//        
//        if (presentation != null) {
//            return presentation.getSizeFlags(horizontal);
//        } 
//        
//        return 0;
//    }
    
    /**
     * @see IPersistable
     */
    public: bool RestoreState(IMemento::Pointer memento);

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.LayoutPart#setVisible(boolean)
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

    public: int GetActive();

    /**
     * Sets the presentation bounds.
     */
//    public: void SetBounds(Rectangle r) {
//      
//        if (getPresentation() != null) {
//            getPresentation().setBounds(r);
//        }
//    }

    public: void SetSelection(LayoutPart::Pointer part);

    /**
     * Subclasses should override this method to update the enablement state of their
     * actions
     */
    protected: virtual void UpdateActions(IWorkbenchPart::Pointer current) = 0;

    /* (non-Javadoc)
   * @see org.eclipse.ui.internal.LayoutPart#handleDeferredEvents()
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
     * @see org.eclipse.ui.internal.ILayoutContainer#obscuredByZoom(org.eclipse.ui.internal.LayoutPart)
     */
//    public: boolean childObscuredByZoom(LayoutPart toTest) {
//        return isObscuredByZoom();
//    }
    
    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.LayoutPart#requestZoom(org.eclipse.ui.internal.LayoutPart)
     */
//    public: void childRequestZoomIn(LayoutPart toZoom) {
//        super.childRequestZoomIn(toZoom);
//        
//        requestZoomIn();
//    }
    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.LayoutPart#requestZoomOut()
     */
//    public: void childRequestZoomOut() {
//        super.childRequestZoomOut();
//        
//        requestZoomOut();
//    }

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.ILayoutContainer#isZoomed(org.eclipse.ui.internal.LayoutPart)
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
    private: void ShowPart(LayoutPart::Pointer part, Object* cookie);

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
    
    public: std::vector<void*> GetTabList(LayoutPart::Pointer part);

    /**
     * 
     * @param beingDragged
     * @param initialLocation
     * @param keyboard
     */
//    private: void dragStart(IPresentablePart beingDragged, Point initialLocation,
//            boolean keyboard) {
//        if (beingDragged == null) {
//            paneDragStart((LayoutPart)null, initialLocation, keyboard);
//        } else {
//            if (presentationSite.isPartMoveable(beingDragged)) {
//                LayoutPart pane = getPaneFor(beingDragged);
//
//                if (pane != null) {
//                    paneDragStart(pane, initialLocation, keyboard);
//                }
//            }
//        }
//    }
    
//    public: void paneDragStart(LayoutPart pane, Point initialLocation,
//            boolean keyboard) {
//        if (pane == null) {
//            if (canMoveFolder()) {              
//                if (presentationSite.getState() == IStackPresentationSite.STATE_MAXIMIZED) {
//                  // Calculate where the initial location was BEFORE the 'restore'...as a percentage
//                  Rectangle bounds = Geometry.toDisplay(getParent(), getPresentation().getControl().getBounds());
//                  float xpct = (initialLocation.x - bounds.x) / (float)(bounds.width);
//                  float ypct = (initialLocation.y - bounds.y) / (float)(bounds.height);
//
//                  // Only restore if we're dragging views/view stacks
//                  if (this instanceof ViewStack)
//                    setState(IStackPresentationSite.STATE_RESTORED);
//
//                  // Now, adjust the initial location to be within the bounds of the restored rect
//                  bounds = Geometry.toDisplay(getParent(), getPresentation().getControl().getBounds());
//                  initialLocation.x = (int) (bounds.x + (xpct * bounds.width));
//                  initialLocation.y = (int) (bounds.y + (ypct * bounds.height));
//                }
//    
//                DragUtil.performDrag(PartStack.this, Geometry
//                        .toDisplay(getParent(), getPresentation().getControl()
//                                .getBounds()), initialLocation, !keyboard);
//            }
//        } else {
//            if (presentationSite.getState() == IStackPresentationSite.STATE_MAXIMIZED) {
//              // Calculate where the initial location was BEFORE the 'restore'...as a percentage
//              Rectangle bounds = Geometry.toDisplay(getParent(), getPresentation().getControl().getBounds());
//              float xpct = (initialLocation.x - bounds.x) / (float)(bounds.width);
//              float ypct = (initialLocation.y - bounds.y) / (float)(bounds.height);
//              
//              // Only restore if we're dragging views/view stacks
//              if (this instanceof ViewStack)
//                setState(IStackPresentationSite.STATE_RESTORED);
//
//              // Now, adjust the initial location to be within the bounds of the restored rect
//              // See bug 100908
//              bounds = Geometry.toDisplay(getParent(), getPresentation().getControl().getBounds());
//              initialLocation.x = (int) (bounds.x + (xpct * bounds.width));
//              initialLocation.y = (int) (bounds.y + (ypct * bounds.height));
//            }
//    
//            DragUtil.performDrag(pane, Geometry.toDisplay(getParent(),
//                    getPresentation().getControl().getBounds()),
//                    initialLocation, !keyboard);
//        }
//    }

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

    
    public: std::string GetProperty(const std::string& id);
    
    public: void SetProperty(const std::string& id, const std::string& value);
    
    /**
     * Copies all appearance related data from this stack to the given stack.
     */
    public: void CopyAppearanceProperties(PartStack::Pointer copyTo);
};

}

#endif /*CHERRYPARTSTACK_H_*/
