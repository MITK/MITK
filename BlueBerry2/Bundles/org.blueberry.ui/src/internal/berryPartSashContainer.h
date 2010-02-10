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

#ifndef BERRYPARTSASHCONTAINER_H_
#define BERRYPARTSASHCONTAINER_H_

#include "berryLayoutPart.h"
#include "berryILayoutContainer.h"
#include "berryIStackableContainer.h"
#include "berryIDragOverListener.h"
#include "berryAbstractDropTarget.h"

#include "../tweaklets/berryDnDTweaklet.h"
#include "../berryRectangle.h"

#include "../guitk/berryGuiTkIControlListener.h"

namespace berry
{

class WorkbenchPage;
class PartPane;
class LayoutTree;
class PartStack;

/**
 * \ingroup org_blueberry_ui_internal
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
class PartSashContainer: public LayoutPart, public ILayoutContainer, public IDragOverListener {

public:
  osgiObjectMacro(PartSashContainer);

  friend class LayoutTree;
  friend class LayoutTreeNode;
  friend class PageLayout;

private:
  void* parentWidget;
  //LayoutPart::Pointer zoomedPart;

  /* Indicates if the children of a sash container should be aligned from left to right
   * or the other way around. This is important if one child does
   * not occupy all of the available space. Then the empty space
   * is either on the left, or on the right side.
   */
  bool static leftToRight;

protected:

  struct ControlListener : public GuiTk::IControlListener
  {
    ControlListener(PartSashContainer* partSashContainer);

    Events::Types GetEventTypes() const;

    void ControlResized(GuiTk::ControlEvent::Pointer e);

  private: PartSashContainer* partSashContainer;
  };

  void* parent;
  GuiTk::IControlListener::Pointer resizeListener;
  SmartPointer<LayoutTree> root;
  WorkbenchPage*  page;

  bool active;
  bool layoutDirty;

  /* Array of LayoutPart */
  ILayoutContainer::ChildrenType children;

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

private:

  class SashContainerDropTarget : public AbstractDropTarget {
    private:

      int side;
    int cursor;

    // This is a IStackablePart or IStackableContainer
    Object::Pointer targetPart;

    // This is a IStackablePart or IStackableContainer
    Object::Pointer sourcePart;

    PartSashContainer* partSashContainer;

    public:

      osgiObjectMacro(SashContainerDropTarget);

      SashContainerDropTarget(PartSashContainer* partSashContainer, Object::Pointer sourcePart,
          int side, int cursor, Object::Pointer targetPart);

    void SetTarget(Object::Pointer sourcePart, int side, int cursor, Object::Pointer targetPart);

     void Drop();

    DnDTweaklet::CursorType GetCursor();

    Rectangle GetSnapRectangle();
};


  SashContainerDropTarget::Pointer dropTarget;

public:

  /**
   * Constructs a PartSashContainer with the given id under the given page
   * and parentWidget.
   *
   * GUI specializations must hook
   */
  PartSashContainer(const std::string& id, WorkbenchPage*  page,
      void* parentWidget);

  ~PartSashContainer();

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.ILayoutContainer#obscuredByZoom(org.blueberry.ui.internal.LayoutPart)
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
  std::vector<SmartPointer<PartPane> > GetVisibleParts(Object::Pointer pane);

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


protected:
  virtual void DropObject(const std::vector<PartPane::Pointer>& toDrop,
        StackablePart::Pointer visiblePart,
            Object::Pointer targetPart, int side);
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
   * @see org.blueberry.ui.internal.LayoutPart#setVisible(boolean)
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
   * @see org.blueberry.ui.internal.LayoutPart#forceLayout(org.blueberry.ui.internal.LayoutPart)
   */
public:
  void ResizeChild(LayoutPart::Pointer childThatChanged);

  /**
   * Remove a part.
   */
public:
  void Remove(LayoutPart::Pointer child);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.LayoutPart#forceLayout()
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
   * @see org.blueberry.ui.internal.LayoutPart#setZoomed(boolean)
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
   * @see org.blueberry.ui.internal.dnd.IDragOverListener#drag(org.blueberry.swt.widgets.Control, java.lang.Object, org.blueberry.swt.graphics.Point, org.blueberry.swt.graphics.Rectangle)
   */
public:
  IDropTarget::Pointer Drag(void* currentControl, Object::Pointer draggedObject,
             const Point& position, const Rectangle& dragRectangle);

  /**
   * @param sourcePart
   * @param targetPart
   * @param side
   * @param cursor
   * @return
   * @since 3.1
   */
private:
  SashContainerDropTarget::Pointer CreateDropTarget(Object::Pointer sourcePart, int side, int cursor, Object::Pointer targetPart);

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
  virtual std::size_t GetVisibleChildrenCount(IStackableContainer::Pointer container);
  virtual std::size_t GetVisibleChildrenCount(ILayoutContainer::Pointer container);

protected:
  virtual float
      GetDockingRatio(Object::Pointer dragged, IStackableContainer::Pointer target);

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
   * @see org.blueberry.ui.internal.ILayoutContainer#isZoomed(org.blueberry.ui.internal.LayoutPart)
   */
  //    public: bool childIsZoomed(LayoutPart toTest) {
  //        return toTest == getZoomedPart();
  //    }

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.ILayoutContainer#allowsAutoFocus()
   */
public:
  bool AllowsAutoFocus();

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.LayoutPart#startDeferringEvents()
   */
protected:
  void StartDeferringEvents();

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.LayoutPart#handleDeferredEvents()
   */
protected:
  void HandleDeferredEvents();

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.LayoutPart#testInvariants()
   */
public:
  void TestInvariants();
};

}

#endif /*BERRYPARTSASHCONTAINER_H_*/
