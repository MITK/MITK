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

#ifndef CHERRYPARTSASHCONTAINER_H_
#define CHERRYPARTSASHCONTAINER_H_

#include "cherryLayoutPart.h"
#include "cherryILayoutContainer.h"
#include "cherryIStackableContainer.h"

#include "../cherryRectangle.h"

#include "../guitk/cherryGuiTkIControlListener.h"

namespace cherry
{

class WorkbenchPage;
class PartPane;
class LayoutTree;
class PartStack;

/**
 * \ingroup org_opencherry_ui_internal
 *
 * Abstract container that groups various layout
 * parts (possibly other containers) together as
 * a unit. Manages the placement and size of these
 * layout parts based on the location of sashes within
 * the container.
 *
 * GUI specializations must override the following methods
 * (read their documentation for implementation details):
 *
 * <ul>
 * <li>PartSashContainer
 */
class PartSashContainer: public LayoutPart, public ILayoutContainer
{ //, IDragOverListener {

public:
  cherryClassMacro(PartSashContainer);

  friend class LayoutTree;
  friend class LayoutTreeNode;
  friend class PageLayout;

private:
  void* parentWidget;
  //LayoutPart::Pointer zoomedPart;

protected:

  struct ControlListener : public GuiTk::IControlListener
  {
    ControlListener(PartSashContainer* partSashContainer);

    void ControlResized(GuiTk::ControlEvent::Pointer e);

  private: PartSashContainer* partSashContainer;
  };

  void* parent;
  GuiTk::IControlListener::Pointer resizeListener;
  SmartPointer<LayoutTree> root;
  SmartPointer<WorkbenchPage>  page;

  bool active;
  bool layoutDirty;

  /* Array of LayoutPart */
  ILayoutContainer::ChildrenType children;

  //TODO DND
  //private: SashContainerDropTarget dropTarget;

protected:
  struct RelationshipInfo
  {
    LayoutPart::Pointer part;

    LayoutPart::Pointer relative;

    int relationship;

    /**
     * Preferred size for the left child (this would be the size, in pixels of the child
     * at the time the sash was last moved)
     */
    int left;

    /**
     * Preferred size for the right child (this would be the size, in pixels of the child
     * at the time the sash was last moved)
     */
    int right;
  };

  // TODO DND
  //    private: class SashContainerDropTarget extends AbstractDropTarget {
  //        private: int side;
  //
  //        private: int cursor;
  //
  //        private: LayoutPart targetPart;
  //
  //        private: LayoutPart sourcePart;
  //
  //        public: SashContainerDropTarget(LayoutPart sourcePart, int side, int cursor, LayoutPart targetPart) {
  //            this.setTarget(sourcePart, side, cursor, targetPart);
  //        }
  //
  //        public: void setTarget(LayoutPart sourcePart, int side, int cursor, LayoutPart targetPart) {
  //            this.side = side;
  //            this.targetPart = targetPart;
  //            this.sourcePart = sourcePart;
  //            this.cursor = cursor;
  //        }
  //
  //        public: void drop() {
  //            if (side != SWT.NONE) {
  //                LayoutPart visiblePart = sourcePart;
  //
  //                if (sourcePart instanceof PartStack) {
  //                    visiblePart = getVisiblePart((PartStack) sourcePart);
  //                }
  //
  //                dropObject(getVisibleParts(sourcePart), visiblePart,
  //                        targetPart, side);
  //            }
  //        }
  //
  //        public: Cursor getCursor() {
  //            return DragCursors.getCursor(DragCursors
  //                    .positionToDragCursor(cursor));
  //        }
  //
  //        public: Rectangle getSnapRectangle() {
  //            Rectangle targetBounds;
  //
  //            if (targetPart != null) {
  //                targetBounds = DragUtil.getDisplayBounds(targetPart
  //                        .getControl());
  //            } else {
  //                targetBounds = DragUtil.getDisplayBounds(getParent());
  //            }
  //
  //            if (side == SWT.CENTER || side == SWT.NONE) {
  //                return targetBounds;
  //            }
  //
  //            int distance = Geometry.getDimension(targetBounds, !Geometry
  //                    .isHorizontal(side));
  //
  //            return Geometry.getExtrudedEdge(targetBounds,
  //                    (int) (distance * getDockingRatio(sourcePart, targetPart)),
  //                    side);
  //        }
  //    }

public:

  /**
   * Constructs a PartSashContainer with the given id under the given page
   * and parentWidget.
   *
   * GUI specializations must hook
   */
  PartSashContainer(const std::string& id, SmartPointer<WorkbenchPage>  page,
      void* parentWidget);

  ~PartSashContainer();

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.ILayoutContainer#obscuredByZoom(org.opencherry.ui.internal.LayoutPart)
   */
  //    public: bool childObscuredByZoom(LayoutPart toTest) {
  //        LayoutPart zoomPart = getZoomedPart();
  //
  //        if (zoomPart != null && toTest != zoomPart) {
  //            return true;
  //        }
  //        return isObscuredByZoom();
  //    }

  /**
   * Given an object associated with a drag (a PartPane or PartStack), this returns
   * the actual PartPanes being dragged.
   *
   * @param pane
   * @return
   */
private:
  std::vector<SmartPointer<PartPane> > GetVisibleParts(LayoutPart::Pointer pane);

  /**
   * Find the sashs around the specified part.
   */
public:
  void FindSashes(LayoutPart::Pointer pane, PartPane::Sashes& sashes);


public:

  /**
   * Add a part.
   */
  virtual void Add(LayoutPart::Pointer child);

  virtual void AddPart(StackablePart::Pointer child);

  /**
   * Add a part relative to another. For compatibility only. New code should use
   * addEnhanced, above.
   *
   * @param child the new part to add
   * @param relationship one of PageLayout.TOP, PageLayout.BOTTOM, PageLayout.LEFT, or PageLayout.RIGHT
   * @param ratio a value between 0.0 and 1.0, indicating how much space will be allocated to the UPPER-LEFT pane
   * @param relative part where the new part will be attached
   */
  virtual void Add(LayoutPart::Pointer child, int relationship, float ratio,
          LayoutPart::Pointer relative);

  /**
   * Add a new part relative to another. This should be used in place of <code>add</code>.
   * It differs as follows:
   * <ul>
   * <li>relationships are specified using SWT direction constants</li>
   * <li>the ratio applies to the newly added child -- not the upper-left child</li>
   * </ul>
   *
   * @param child new part to add to the layout
   * @param swtDirectionConstant one of SWT.TOP, SWT.BOTTOM, SWT.LEFT, or SWT.RIGHT
   * @param ratioForNewPart a value between 0.0 and 1.0 specifying how much space will be allocated for the newly added part
   * @param relative existing part indicating where the new child should be attached
   * @since 3.0
   */
protected:
  virtual void AddEnhanced(LayoutPart::Pointer child, int swtDirectionConstant,
      float ratioForNewPart, LayoutPart::Pointer relative);

protected:
  static int MeasureTree(const Rectangle& outerBounds,
      SmartPointer<const LayoutTree> toMeasure, bool horizontal);

protected:
  virtual void AddChild(const RelationshipInfo& info);

  /**
   * Adds the child using ratio and position attributes
   * from the specified placeholder without replacing
   * the placeholder
   *
   * FIXME: I believe there is a bug in computeRelation()
   * when a part is positioned relative to the editorarea.
   * We end up with a null relative and 0.0 for a ratio.
   */
protected:
  virtual void AddChildForPlaceholder(LayoutPart::Pointer child,
      LayoutPart::Pointer placeholder);

  /**
   * See ILayoutContainer#allowBorder
   */
public:
  virtual bool AllowsBorder();

  /**
   * Notification that a child layout part has been
   * added to the container. Subclasses may override
   * this method to perform any container specific
   * work.
   */
protected:
  virtual void ChildAdded(LayoutPart::Pointer child);

  /**
   * Notification that a child layout part has been
   * removed from the container. Subclasses may override
   * this method to perform any container specific
   * work.
   */
protected:
  virtual void ChildRemoved(LayoutPart::Pointer child);

  /**
   * Returns an array with all the relation ship between the
   * parts.
   */
public:
  virtual std::vector<RelationshipInfo> ComputeRelation();

public:
  virtual void SetActive(bool isActive);

  /**
   * @see LayoutPart#getControl
   */
public:
  void CreateControl(void* parentWidget);

  /**
   * Subclasses override this method to specify
   * the composite to use to parent all children
   * layout parts it contains.
   */
protected:
  virtual void* CreateParent(void* parentWidget) = 0;

  /**
   * @see LayoutPart#dispose
   */
public:
  virtual void Dispose();

  /**
   * Subclasses override this method to dispose
   * of any swt resources created during createParent.
   */
protected:
  virtual void DisposeParent() = 0;

  /**
   * Dispose all sashs used in this perspective.
   */
public:
 virtual  void DisposeSashes();

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.LayoutPart#setVisible(boolean)
   */
public:
  void SetVisible(bool makeVisible);

  /**
   * Return the most bottom right part or null if none.
   */
public:
  virtual LayoutPart::Pointer FindBottomRight();

  /**
   * @see LayoutPart#getBounds
   */
public:
  Rectangle GetBounds();

  /**
   * @see ILayoutContainer#getChildren
   */
public:
  ChildrenType GetChildren();

  /**
   * @see LayoutPart#getControl
   */
public:
  void* GetControl();

public:
  virtual SmartPointer<LayoutTree> GetLayoutTree();

  /**
   * For themes.
   *
   * @return the current WorkbenchPage.
   */
public:
  virtual SmartPointer<WorkbenchPage>  GetPage();

  /**
   * Returns the composite used to parent all the
   * layout parts contained within.
   */
public:
  virtual void* GetParent();

protected:
  virtual bool IsChild(LayoutPart::Pointer part);

  /**
   * Returns whether this container is zoomed.
   */
  //    public: bool IsZoomed() {
  //        return (zoomedPart != null);
  //    }

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.LayoutPart#forceLayout(org.opencherry.ui.internal.LayoutPart)
   */
public:
  void ResizeChild(LayoutPart::Pointer childThatChanged);

  /**
   * Remove a part.
   */
public:
  void Remove(LayoutPart::Pointer child);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.LayoutPart#forceLayout()
   */
public:
  void FlushLayout();

  /**
   * Replace one part with another.
   */
public:
  void Replace(LayoutPart::Pointer oldChild, LayoutPart::Pointer newChild);

private:
  void ResizeSashes();

  /**
   * Returns the maximum size that can be utilized by this part if the given width and
   * height are available. Parts can overload this if they have a quantized set of preferred
   * sizes.
   *
   * @param width available horizontal space (pixels)
   * @return returns a new point where point.x is <= availableWidth and point.y is <= availableHeight
   */
public:
  virtual int ComputePreferredSize(bool width, int availableParallel,
      int availablePerpendicular, int preferredParallel);

public:
  int GetSizeFlags(bool width);

  /**
   * @see LayoutPart#setBounds
   */
public:
  void SetBounds(const Rectangle& r);

  /**
   * Zoom in on a particular layout part.
   *
   * The implementation of zoom is quite simple.  When zoom occurs we create
   * a zoom root which only contains the zoom part.  We store the old
   * root in unzoomRoot and then active the zoom root.  When unzoom occurs
   * we restore the unzoomRoot and dispose the zoom root.
   *
   * Note: Method assumes we are active.
   */
  //    private: void zoomIn(LayoutPart part) {
  //        // Sanity check.
  //        if (isZoomed()) {
  //      return;
  //    }
  //
  //        // Hide the sashes
  //      root.disposeSashes();
  //
  //        // Make all parts invisible except for the zoomed part
  //        LayoutPart[] children = getChildren();
  //        for (int i = 0; i < children.length; i++) {
  //            LayoutPart child = children[i];
  //            child.setVisible(child == part);
  //        }
  //
  //        zoomedPart = part;
  //
  //        // Notify the part that it has been zoomed
  //      part.setZoomed(true);
  //
  //        // Remember that we need to trigger a layout
  //        layoutDirty = true;
  //    }

  /**
   * Returns the currently zoomed part or null if none
   *
   * @return the currently zoomed part or null if none
   * @since 3.1
   */
  //    public: LayoutPart getZoomedPart() {
  //        return zoomedPart;
  //    }

  //    public: void childRequestZoomIn(LayoutPart toZoom) {
  //        if (!SwtUtil.isDisposed(this.parent)) {
  //            this.parent.setRedraw(false);
  //        }
  //        try {
  //          zoomIn(toZoom);
  //
  //          requestZoomIn();
  //
  //          if (layoutDirty) {
  //              resizeSashes();
  //          }
  //        } finally {
  //            if (!SwtUtil.isDisposed(this.parent)) {
  //                this.parent.setRedraw(true);
  //            }
  //        }
  //    }

  //    public: void childRequestZoomOut() {
  //        if (!SwtUtil.isDisposed(this.parent)) {
  //            this.parent.setRedraw(false);
  //        }
  //        try {
  //          zoomOut();
  //
  //          requestZoomOut();
  //
  //          if (layoutDirty) {
  //              resizeSashes();
  //          }
  //        } finally {
  //            if (!SwtUtil.isDisposed(this.parent)) {
  //                this.parent.setRedraw(true);
  //            }
  //        }
  //    }

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.LayoutPart#setZoomed(boolean)
   */
  //    public: void setZoomed(boolean isZoomed) {
  //        if (!isZoomed) {
  //            zoomOut();
  //        } else {
  //            if (!isZoomed()) {
  //                LayoutPart toZoom = pickPartToZoom();
  //
  //                if (toZoom != null) {
  //                    zoomIn(toZoom);
  //                }
  //            }
  //        }
  //        super.setZoomed(isZoomed);
  //    }

  //    public: LayoutPart pickPartToZoom() {
  //        return findBottomRight();
  //    }

  /**
   * Zoom out.
   *
   * See zoomIn for implementation details.
   *
   * Note: Method assumes we are active.
   */
  //    private: void zoomOut() {
  //        // Sanity check.
  //        if (!isZoomed()) {
  //      return;
  //    }
  //
  //        LayoutPart zoomedPart = this.zoomedPart;
  //        this.zoomedPart = null;
  //        // Inform the part that it is no longer zoomed
  //        zoomedPart.setZoomed(false);
  //
  //        // Make all children visible
  //        LayoutPart[] children = getChildren();
  //        for (int i = 0; i < children.length; i++) {
  //            LayoutPart child = children[i];
  //
  //            child.setVisible(true);
  //        }
  //
  //        // Recreate the sashes
  //        root.createControl(getParent());
  //
  //        // Ensure that the part being un-zoomed will have its size refreshed.
  //        LayoutTree node = root.find(zoomedPart);
  //        node.flushCache();
  //
  //        layoutDirty = true;
  //    }

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.dnd.IDragOverListener#drag(org.opencherry.swt.widgets.Control, java.lang.Object, org.opencherry.swt.graphics.Point, org.opencherry.swt.graphics.Rectangle)
   */
  //TODO DND
  //    public: IDropTarget drag(Control currentControl, Object draggedObject,
  //            Point position, Rectangle dragRectangle) {
  //        if (!(draggedObject instanceof LayoutPart)) {
  //            return null;
  //        }
  //
  //        final LayoutPart sourcePart = (LayoutPart) draggedObject;
  //
  //        if (!isStackType(sourcePart) && !isPaneType(sourcePart)) {
  //            return null;
  //        }
  //
  //        boolean differentWindows = sourcePart.getWorkbenchWindow() != getWorkbenchWindow();
  //        boolean editorDropOK = ((sourcePart instanceof EditorPane) &&
  //                      sourcePart.getWorkbenchWindow().getWorkbench() ==
  //                      getWorkbenchWindow().getWorkbench());
  //        if (differentWindows && !editorDropOK) {
  //            return null;
  //        }
  //
  //        Rectangle containerBounds = DragUtil.getDisplayBounds(parent);
  //        LayoutPart targetPart = null;
  //        ILayoutContainer sourceContainer = isStackType(sourcePart) ? (ILayoutContainer) sourcePart
  //                : sourcePart.getContainer();
  //
  //        // If this container has no visible children
  //        if (getVisibleChildrenCount(this) == 0) {
  //            return createDropTarget(sourcePart, SWT.CENTER, SWT.CENTER, null);
  //        }
  //
  //        if (containerBounds.contains(position)) {
  //
  //            if (root != null) {
  //                targetPart = root.findPart(parent.toControl(position));
  //            }
  //
  //            if (targetPart != null) {
  //                final Control targetControl = targetPart.getControl();
  //
  //                final Rectangle targetBounds = DragUtil
  //                        .getDisplayBounds(targetControl);
  //
  //                int side = Geometry.getClosestSide(targetBounds, position);
  //                int distance = Geometry.getDistanceFromEdge(targetBounds, position, side);
  //
  //                // is the source coming from a standalone part
  //                boolean standalone = (isStackType(sourcePart)
  //                    && ((PartStack) sourcePart).isStandalone())
  //                  || (isPaneType(sourcePart)
  //                      && ((PartPane) sourcePart).getStack()!=null
  //                      && ((PartPane) sourcePart).getStack().isStandalone());
  //
  //                // Only allow dropping onto an existing stack from different windows
  //                if (differentWindows && targetPart instanceof EditorStack) {
  //                    IDropTarget target = targetPart.getDropTarget(draggedObject, position);
  //                    return target;
  //                }
  //
  //                // Reserve the 5 pixels around the edge of the part for the drop-on-edge cursor
  //                if (distance >= 5 && !standalone) {
  //                    // Otherwise, ask the part if it has any special meaning for this drop location
  //                    IDropTarget target = targetPart.getDropTarget(draggedObject, position);
  //                    if (target != null) {
  //                        return target;
  //                    }
  //                }
  //
  //                if (distance > 30 && isStackType(targetPart) && !standalone) {
  //                    if (targetPart instanceof ILayoutContainer) {
  //                        ILayoutContainer targetContainer = (ILayoutContainer)targetPart;
  //                        if (targetContainer.allowsAdd(sourcePart)) {
  //                            side = SWT.CENTER;
  //                        }
  //                    }
  //                }
  //
  //                // If the part doesn't want to override this drop location then drop on the edge
  //
  //                // A "pointless drop" would be one that will put the dragged object back where it started.
  //                // Note that it should be perfectly valid to drag an object back to where it came from -- however,
  //                // the drop should be ignored.
  //
  //                boolean pointlessDrop = isZoomed();
  //
  //                if (sourcePart == targetPart) {
  //                    pointlessDrop = true;
  //                }
  //
  //                if ((sourceContainer != null)
  //                        && (sourceContainer == targetPart)
  //                        && getVisibleChildrenCount(sourceContainer) <= 1) {
  //                    pointlessDrop = true;
  //                }
  //
  //                if (side == SWT.CENTER
  //                        && sourcePart.getContainer() == targetPart) {
  //                    pointlessDrop = true;
  //                }
  //
  //                int cursor = side;
  //
  //                if (pointlessDrop) {
  //                    side = SWT.NONE;
  //                    cursor = SWT.CENTER;
  //                }
  //
  //                return createDropTarget(sourcePart, side, cursor, targetPart);
  //            }
  //        } else {
  //          // We only allow dropping into a stack, not creating one
  //          if (differentWindows)
  //            return null;
  //
  //            int side = Geometry.getClosestSide(containerBounds, position);
  //
  //            boolean pointlessDrop = isZoomed();
  //
  //            if ((isStackType(sourcePart) && sourcePart.getContainer() == this)
  //                    || (sourcePart.getContainer() != null
  //                       && isPaneType(sourcePart)
  //                       && getVisibleChildrenCount(sourcePart.getContainer()) <= 1)
  //                       && ((LayoutPart)sourcePart.getContainer()).getContainer() == this) {
  //                if (root == null || getVisibleChildrenCount(this) <= 1) {
  //                    pointlessDrop = true;
  //                }
  //            }
  //
  //            int cursor = Geometry.getOppositeSide(side);
  //
  //            if (pointlessDrop) {
  //                side = SWT.NONE;
  //            }
  //
  //            return createDropTarget(sourcePart, side, cursor, null);
  //        }
  //
  //        return null;
  //    }

  /**
   * @param sourcePart
   * @param targetPart
   * @param side
   * @param cursor
   * @return
   * @since 3.1
   */
  // TODO DND
  //    private: SashContainerDropTarget createDropTarget(final LayoutPart sourcePart, int side, int cursor, LayoutPart targetPart) {
  //        if (dropTarget == null) {
  //            dropTarget = new SashContainerDropTarget(sourcePart, side, cursor,
  //                targetPart);
  //        } else {
  //            dropTarget.setTarget(sourcePart, side, cursor, targetPart);
  //        }
  //        return dropTarget;
  //    }

  /**
   * Returns true iff this PartSashContainer allows its parts to be stacked onto the given
   * container.
   *
   * @param container
   * @return
   */
public:
  virtual bool IsStackType(IStackableContainer::Pointer toTest) = 0;

public:
  virtual bool IsPaneType(StackablePart::Pointer toTest) = 0;

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.PartSashContainer#dropObject(org.opencherry.ui.internal.LayoutPart, org.opencherry.ui.internal.LayoutPart, int)
   */
  //TODO DND
  //    protected: void dropObject(PartPane[] toDrop, LayoutPart visiblePart,
  //            LayoutPart targetPart, int side) {
  //        getControl().setRedraw(false);
  //
  //        // Targetpart is null if there isn't a part under the cursor (all the parts are
  //        // hidden or the container is empty). In this case, the actual side doesn't really
  //        // since we'll be the only visible container and will fill the entire space. However,
  //        // we can't leave it as SWT.CENTER since we can't stack if we don't have something
  //        // to stack on. In this case, we pick SWT.BOTTOM -- this will insert the new pane
  //        // below any currently-hidden parts.
  //        if (targetPart == null && side == SWT.CENTER) {
  //            side = SWT.BOTTOM;
  //        }
  //
  //        if (side == SWT.CENTER) {
  //            if (isStackType(targetPart)) {
  //                PartStack stack = (PartStack) targetPart;
  //                for (int idx = 0; idx < toDrop.length; idx++) {
  //                    PartPane next = toDrop[idx];
  //                    stack(next, stack);
  //                }
  //            }
  //        } else {
  //            PartStack newPart = createStack();
  //
  //            // if the toDrop array has 1 item propogate the stack
  //            // appearance
  //            if (toDrop.length == 1 && toDrop[0].getStack()!=null) {
  //                toDrop[0].getStack().copyAppearanceProperties(newPart);
  //            }
  //
  //            for (int idx = 0; idx < toDrop.length; idx++) {
  //                PartPane next = toDrop[idx];
  //                stack(next, newPart);
  //            }
  //
  //            addEnhanced(newPart, side, getDockingRatio(newPart, targetPart),
  //                    targetPart);
  //        }
  //
  //        if (visiblePart != null) {
  //            setVisiblePart(visiblePart.getContainer(), visiblePart);
  //        }
  //
  //        getControl().setRedraw(true);
  //
  //        if (visiblePart != null) {
  //            visiblePart.setFocus();
  //        }
  //    }

protected:
  virtual SmartPointer<PartStack> CreateStack() = 0;

public:
  virtual void Stack(StackablePart::Pointer newPart, SmartPointer<IStackableContainer> container);

  /**
   * @param container
   * @param visiblePart
   */
protected:
  virtual void SetVisiblePart(IStackableContainer::Pointer container,
      SmartPointer<PartPane>  visiblePart) = 0;

  /**
   * @param container
   * @return
   */
protected:
  virtual StackablePart::Pointer GetVisiblePart(
      IStackableContainer::Pointer container) = 0;

  /**
   * @param sourcePart
   */
protected:
  virtual void DerefPart(StackablePart::Pointer sourcePart);

protected:
  virtual int GetVisibleChildrenCount(IStackableContainer::Pointer container);

protected:
  virtual float
      GetDockingRatio(StackablePart::Pointer dragged, IStackableContainer::Pointer target);

  /**
   * Writes a description of the layout to the given string buffer.
   * This is used for drag-drop test suites to determine if two layouts are the
   * same. Like a hash code, the description should compare as equal iff the
   * layouts are the same. However, it should be user-readable in order to
   * help debug failed tests. Although these are english readable strings,
   * they should not be translated or equality tests will fail.
   *
   * @param buf
   */
public:
  void DescribeLayout(std::string& buf) const;

  /**
   * Adds a new child to the container relative to some part
   *
   * @param child
   * @param relationship
   * @param left preferred pixel size of the left/top child
   * @param right preferred pixel size of the right/bottom child
   * @param relative relative part
   */
protected:
  virtual void Add(LayoutPart::Pointer child, int relationship, int left, int right,
      LayoutPart::Pointer relative);

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.ILayoutContainer#isZoomed(org.opencherry.ui.internal.LayoutPart)
   */
  //    public: bool childIsZoomed(LayoutPart toTest) {
  //        return toTest == getZoomedPart();
  //    }

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.ILayoutContainer#allowsAutoFocus()
   */
public:
  bool AllowsAutoFocus();

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.LayoutPart#startDeferringEvents()
   */
protected:
  void StartDeferringEvents();

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.LayoutPart#handleDeferredEvents()
   */
protected:
  void HandleDeferredEvents();

  /* (non-Javadoc)
   * @see org.opencherry.ui.internal.LayoutPart#testInvariants()
   */
public:
  void TestInvariants();
};

}

#endif /*CHERRYPARTSASHCONTAINER_H_*/
