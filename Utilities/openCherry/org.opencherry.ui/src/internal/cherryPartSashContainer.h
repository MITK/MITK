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

namespace cherry
{

/**
 * Abstract container that groups various layout
 * parts (possibly other containers) together as
 * a unit. Manages the placement and size of these
 * layout part based on the location of sashes within
 * the container.
 */
class PartSashContainer : public LayoutPart, public ILayoutContainer { //, IDragOverListener {

    protected: void* parent;

    protected: ControlListener resizeListener;

    protected: LayoutTree root;
    
    private: Composite parentWidget;

    //private: LayoutPart::Pointer zoomedPart;

    protected: WorkbenchPage::Pointer page;

    boolean active = false;
    boolean layoutDirty = false;

    /* Array of LayoutPart */
    protected: ArrayList children = new ArrayList();
    
    private: SashContainerDropTarget dropTarget;

    protected: static class RelationshipInfo {
        protected: LayoutPart part;

        protected: LayoutPart relative;

        protected: int relationship;

        /**
         * Preferred size for the left child (this would be the size, in pixels of the child
         * at the time the sash was last moved)
         */
        protected: int left;

        /**
         * Preferred size for the right child (this would be the size, in pixels of the child
         * at the time the sash was last moved)
         */
        protected: int right;

        /**
         * Computes the "ratio" for this container. That is, the ratio of the left side over
         * the sum of left + right. This is only used for serializing PartSashContainers in 
         * a form that can be read by old versions of Eclipse. This can be removed if this
         * is no longer required. 
         * 
         * @return the pre-Eclipse 3.0 sash ratio
         */
        public: float getRatio() {
            int total = left + right;
            if (total > 0) {
                return (float) left / (float) total;
            }
            
            return 0.5f;
        }
    }

    private: class SashContainerDropTarget extends AbstractDropTarget {
        private: int side;

        private: int cursor;

        private: LayoutPart targetPart;

        private: LayoutPart sourcePart;

        public: SashContainerDropTarget(LayoutPart sourcePart, int side, int cursor, LayoutPart targetPart) {
            this.setTarget(sourcePart, side, cursor, targetPart);
        }
        
        public: void setTarget(LayoutPart sourcePart, int side, int cursor, LayoutPart targetPart) {
            this.side = side;
            this.targetPart = targetPart;
            this.sourcePart = sourcePart;
            this.cursor = cursor;
        }

        public: void drop() {
            if (side != SWT.NONE) {
                LayoutPart visiblePart = sourcePart;

                if (sourcePart instanceof PartStack) {
                    visiblePart = getVisiblePart((PartStack) sourcePart);
                }

                dropObject(getVisibleParts(sourcePart), visiblePart,
                        targetPart, side);
            }
        }

        public: Cursor getCursor() {
            return DragCursors.getCursor(DragCursors
                    .positionToDragCursor(cursor));
        }

        public: Rectangle getSnapRectangle() {
            Rectangle targetBounds;

            if (targetPart != null) {
                targetBounds = DragUtil.getDisplayBounds(targetPart
                        .getControl());
            } else {
                targetBounds = DragUtil.getDisplayBounds(getParent());
            }

            if (side == SWT.CENTER || side == SWT.NONE) {
                return targetBounds;
            }

            int distance = Geometry.getDimension(targetBounds, !Geometry
                    .isHorizontal(side));

            return Geometry.getExtrudedEdge(targetBounds,
                    (int) (distance * getDockingRatio(sourcePart, targetPart)),
                    side);
        }
    }

    public: PartSashContainer(String id, final WorkbenchPage page, Composite parentWidget) {
        super(id);
        this.page = page;
        this.parentWidget = parentWidget;
        resizeListener = new ControlAdapter() {
            public: void controlResized(ControlEvent e) {
                resizeSashes();
            }
        };
    }

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.ILayoutContainer#obscuredByZoom(org.eclipse.ui.internal.LayoutPart)
     */
    public: boolean childObscuredByZoom(LayoutPart toTest) {
        LayoutPart zoomPart = getZoomedPart();
        
        if (zoomPart != null && toTest != zoomPart) {
            return true;
        }
        return isObscuredByZoom();
    }
    
    /**
     * Given an object associated with a drag (a PartPane or PartStack), this returns
     * the actual PartPanes being dragged.
     * 
     * @param pane
     * @return
     */
    private: PartPane[] getVisibleParts(LayoutPart pane) {
        if (pane instanceof PartPane) {
            return new PartPane[] { (PartPane) pane };
        } else if (pane instanceof PartStack) {
            PartStack stack = (PartStack) pane;

            LayoutPart[] children = stack.getChildren();
            ArrayList result = new ArrayList(children.length);
            for (int idx = 0; idx < children.length; idx++) {
                LayoutPart next = children[idx];
                if (next instanceof PartPane) {
                    result.add(next);
                }
            }

            return (PartPane[]) result.toArray(new PartPane[result.size()]);
        }

        return new PartPane[0];
    }

    /**
     * Find the sashs around the specified part.
     */
    public: void findSashes(LayoutPart pane, PartPane.Sashes sashes) {
        if (root == null) {
            return;
        }
        LayoutTree part = root.find(pane);
        if (part == null) {
      return;
    }
        part.findSashes(sashes);
    }

    /**
     * Add a part.
     */
    public: void add(LayoutPart child) {
        if (child == null) {
      return;
    }

        addEnhanced(child, SWT.RIGHT, 0.5f, findBottomRight());
    }

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
    void addEnhanced(LayoutPart child, int swtDirectionConstant,
            float ratioForNewPart, LayoutPart relative) {
        int relativePosition = PageLayout
                .swtConstantToLayoutPosition(swtDirectionConstant);

        float ratioForUpperLeftPart;

        if (relativePosition == IPageLayout.RIGHT
                || relativePosition == IPageLayout.BOTTOM) {
            ratioForUpperLeftPart = 1.0f - ratioForNewPart;
        } else {
            ratioForUpperLeftPart = ratioForNewPart;
        }

        add(child, relativePosition, ratioForUpperLeftPart, relative);
    }

    /**
     * Add a part relative to another. For compatibility only. New code should use
     * addEnhanced, above.
     * 
     * @param child the new part to add
     * @param relationship one of PageLayout.TOP, PageLayout.BOTTOM, PageLayout.LEFT, or PageLayout.RIGHT
     * @param ratio a value between 0.0 and 1.0, indicating how much space will be allocated to the UPPER-LEFT pane
     * @param relative part where the new part will be attached
     */
    public: void add(LayoutPart child, int relationship, float ratio,
            LayoutPart relative) {
        boolean isHorizontal = (relationship == IPageLayout.LEFT || relationship == IPageLayout.RIGHT);

        LayoutTree node = null;
        if (root != null && relative != null) {
            node = root.find(relative);
        }

        Rectangle bounds;
        if (getParent() == null) {
            Control control = getPage().getClientComposite();
            if (control != null && !control.isDisposed()) {
                bounds = control.getBounds();
            } else {
                bounds = new Rectangle(0, 0, 800, 600);
            }
            bounds.x = 0;
            bounds.y = 0;
        } else {
            bounds = getBounds();
        }

        int totalSize = measureTree(bounds, node, isHorizontal);

        int left = (int) (totalSize * ratio);
        int right = totalSize - left;

        add(child, relationship, left, right, relative);
    }

    static int measureTree(Rectangle outerBounds, LayoutTree toMeasure,
            boolean horizontal) {
        if (toMeasure == null) {
            return Geometry.getDimension(outerBounds, horizontal);
        }

        LayoutTreeNode parent = toMeasure.getParent();
        if (parent == null) {
            return Geometry.getDimension(outerBounds, horizontal);
        }

        if (parent.getSash().isHorizontal() == horizontal) {
            return measureTree(outerBounds, parent, horizontal);
        }

        boolean isLeft = parent.isLeftChild(toMeasure);

        LayoutTree otherChild = parent.getChild(!isLeft);
        if (otherChild.isVisible()) {
            int left = parent.getSash().getLeft();
            int right = parent.getSash().getRight();
            int childSize = isLeft ? left : right;

            int bias = parent.getCompressionBias();

            // Normalize bias: 1 = we're fixed, -1 = other child is fixed
            if (isLeft) {
                bias = -bias;
            }

            if (bias == 1) {
                // If we're fixed, return the fixed size
                return childSize;
            } else if (bias == -1) {

                // If the other child is fixed, return the size of the parent minus the fixed size of the
                // other child
                return measureTree(outerBounds, parent, horizontal)
                        - (left + right - childSize);
            }

            // Else return the size of the parent, scaled appropriately
            return measureTree(outerBounds, parent, horizontal) * childSize
                    / (left + right);
        }

        return measureTree(outerBounds, parent, horizontal);
    }

    protected: void addChild(RelationshipInfo info) {
        LayoutPart child = info.part;

        children.add(child);

        if (root == null) {
            root = new LayoutTree(child);
        } else {
            //Add the part to the tree.
            int vertical = (info.relationship == IPageLayout.LEFT || info.relationship == IPageLayout.RIGHT) ? SWT.VERTICAL
                    : SWT.HORIZONTAL;
            boolean left = info.relationship == IPageLayout.LEFT
                    || info.relationship == IPageLayout.TOP;
            LayoutPartSash sash = new LayoutPartSash(this, vertical);
            sash.setSizes(info.left, info.right);
            if ((parent != null) && !(child instanceof PartPlaceholder)) {
        sash.createControl(parent);
      }
            root = root.insert(child, left, sash, info.relative);
        }

        childAdded(child);

        if (active) {
            child.createControl(parent);
            child.setVisible(true);
            child.setContainer(this);
            resizeChild(child);
        }

    }

    /**
     * Adds the child using ratio and position attributes
     * from the specified placeholder without replacing
     * the placeholder
     * 
     * FIXME: I believe there is a bug in computeRelation()
     * when a part is positioned relative to the editorarea.
     * We end up with a null relative and 0.0 for a ratio.
     */
    void addChildForPlaceholder(LayoutPart child, LayoutPart placeholder) {
        RelationshipInfo newRelationshipInfo = new RelationshipInfo();
        newRelationshipInfo.part = child;
        if (root != null) {
            newRelationshipInfo.relationship = IPageLayout.RIGHT;
            newRelationshipInfo.relative = root.findBottomRight();
            newRelationshipInfo.left = 200;
            newRelationshipInfo.right = 200;
        }

        // find the relationship info for the placeholder
        RelationshipInfo[] relationships = computeRelation();
        for (int i = 0; i < relationships.length; i++) {
            RelationshipInfo info = relationships[i];
            if (info.part == placeholder) {
                newRelationshipInfo.left = info.left;
                newRelationshipInfo.right = info.right;
                newRelationshipInfo.relationship = info.relationship;
                newRelationshipInfo.relative = info.relative;
            }
        }

        addChild(newRelationshipInfo);
        flushLayout();
    }

    /**
     * See ILayoutContainer#allowBorder
     */
    public: boolean allowsBorder() {
        return true;
    }

    /**
     * Notification that a child layout part has been
     * added to the container. Subclasses may override
     * this method to perform any container specific
     * work.
     */
    protected: void childAdded(LayoutPart child) {
      if (isDeferred()) {
        child.deferUpdates(true);
      }
    }

    /**
     * Notification that a child layout part has been
     * removed from the container. Subclasses may override
     * this method to perform any container specific
     * work.
     */
    protected: void childRemoved(LayoutPart child) {
      if (isDeferred()) {
        child.deferUpdates(false);
      }
    }

    /**
     * Returns an array with all the relation ship between the
     * parts.
     */
    public: RelationshipInfo[] computeRelation() {
        LayoutTree treeRoot = root;
        ArrayList list = new ArrayList();
        if (treeRoot == null) {
      return new RelationshipInfo[0];
    }
        RelationshipInfo r = new RelationshipInfo();
        r.part = treeRoot.computeRelation(list);
        list.add(0, r);
        RelationshipInfo[] result = new RelationshipInfo[list.size()];
        list.toArray(result);
        return result;
    }

    public: void setActive(boolean isActive) {
        if (isActive == active) {
            return;
        }
        
        active = isActive;
        
        ArrayList children = (ArrayList) this.children.clone();
        for (int i = 0, length = children.size(); i < length; i++) {
            LayoutPart child = (LayoutPart) children.get(i);
            
            if (child instanceof PartStack) {
                PartStack stack = (PartStack) child;
                stack.setActive(isActive);
            }
        }
        
        if (isActive) {
            
            createControl(parentWidget);
            
            parent.addControlListener(resizeListener);

            DragUtil.addDragTarget(parent, this);
            DragUtil.addDragTarget(parent.getShell(), this);
            
            //ArrayList children = (ArrayList) this.children.clone();
            for (int i = 0, length = children.size(); i < length; i++) {
                LayoutPart child = (LayoutPart) children.get(i);
                child.setContainer(this);
                child.setVisible(zoomedPart == null || child == zoomedPart);
                if (!(child instanceof PartStack)) {
                    if (root != null) {
                        LayoutTree node = root.find(child);
                        if (node != null) {
                            node.flushCache();
                        }
                    }
                }
            }
            
            if (root != null) {
                //root.flushChildren();
                if (!isZoomed()) {
                    root.createControl(parent);
                }
            }
            
            resizeSashes();
        } else {
            DragUtil.removeDragTarget(parent, this);
            DragUtil.removeDragTarget(parent.getShell(), this);

            // remove all Listeners
            if (resizeListener != null && parent != null) {
                parent.removeControlListener(resizeListener);
            }

            if (children != null) {
                for (int i = 0, length = children.size(); i < length; i++) {
                    LayoutPart child = (LayoutPart) children.get(i);
                    child.setContainer(null);
                    if (child instanceof PartStack) {
                        child.setVisible(false);
                    }
                }
            }
            
            disposeSashes();
            
            //dispose();
        }
    }
    
    /**
     * @see LayoutPart#getControl
     */
    public: void createControl(Composite parentWidget) {
        if (this.parent != null) {
      return;
    }

        parent = createParent(parentWidget);

        ArrayList children = (ArrayList) this.children.clone();
        for (int i = 0, length = children.size(); i < length; i++) {
            LayoutPart child = (LayoutPart) children.get(i);
            child.createControl(parent);
        }

    }

    /**
     * Subclasses override this method to specify
     * the composite to use to parent all children
     * layout parts it contains.
     */
    protected: abstract Composite createParent(Composite parentWidget);

    /**
     * @see LayoutPart#dispose
     */
    public: void dispose() {
        if (parent == null) {
      return;
    }

        if (children != null) {
            for (int i = 0, length = children.size(); i < length; i++) {
                LayoutPart child = (LayoutPart) children.get(i);

                // In PartSashContainer dispose really means deactivate, so we
                // only dispose PartTabFolders.
                if (child instanceof PartStack) {
          child.dispose();
        }
            }
        }
        disposeParent();
        this.parent = null;
    }

    /**
     * Subclasses override this method to dispose
     * of any swt resources created during createParent.
     */
    protected: abstract void disposeParent();

    /**
     * Dispose all sashs used in this perspective.
     */
    public: void disposeSashes() {
        if (root != null) {
            root.disposeSashes();
        }
    }

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.LayoutPart#setVisible(boolean)
     */
    public: void setVisible(boolean makeVisible) {
        
        if (makeVisible == getVisible()) {
            return;
        }
        
        if (!SwtUtil.isDisposed(this.parent)) {
            this.parent.setEnabled(makeVisible);
        }
        super.setVisible(makeVisible);
        
        ArrayList children = (ArrayList) this.children.clone();
        for (int i = 0, length = children.size(); i < length; i++) {
            LayoutPart child = (LayoutPart) children.get(i);
            child.setVisible(makeVisible && (zoomedPart == null || child == zoomedPart));
        }
    }
    
    /**
     * Return the most bottom right part or null if none.
     */
    public: LayoutPart findBottomRight() {
        if (root == null) {
      return null;
    }
        return root.findBottomRight();
    }

    /**
     * @see LayoutPart#getBounds
     */
    public: Rectangle getBounds() {
        return this.parent.getBounds();
    }

    /**
     * @see ILayoutContainer#getChildren
     */
    public: LayoutPart[] getChildren() {
        LayoutPart[] result = new LayoutPart[children.size()];
        children.toArray(result);
        return result;
    }

    /**
     * @see LayoutPart#getControl
     */
    public: Control getControl() {
        return this.parent;
    }

    public: LayoutTree getLayoutTree() {
        return root;
    }

    /**
     * For themes.
     * 
     * @return the current WorkbenchPage.
     */
    public: WorkbenchPage getPage() {
        return page;
    }

    /**
     * Returns the composite used to parent all the
     * layout parts contained within.
     */
    public: Composite getParent() {
        return parent;
    }

    protected: boolean isChild(LayoutPart part) {
        return children.indexOf(part) >= 0;
    }

    /**
     * Returns whether this container is zoomed.
     */
    public: boolean isZoomed() {
        return (zoomedPart != null);
    }

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.LayoutPart#forceLayout(org.eclipse.ui.internal.LayoutPart)
     */
    public: void resizeChild(LayoutPart childThatChanged) {
      if (root != null) {
        LayoutTree tree = root.find(childThatChanged);
        
        if (tree != null) {
          tree.flushCache();
        }
      }
      
        flushLayout();

    }

    /**
     * Remove a part.
     */
    public: void remove(LayoutPart child) {
        if (child == getZoomedPart()) {
      childRequestZoomOut();
    }

        if (!isChild(child)) {
      return;
    }

        children.remove(child);
        if (root != null) {
            root = root.remove(child);
        }
        childRemoved(child);

        if (active) {
            child.setVisible(false);
            child.setContainer(null);
            flushLayout();
        }
    }

  /* (non-Javadoc)
   * @see org.eclipse.ui.internal.LayoutPart#forceLayout()
   */
  public: void flushLayout() {
    layoutDirty = true;
    super.flushLayout();
    
    if (layoutDirty) {
      resizeSashes();
    }
  }
  
    /**
     * Replace one part with another.
     */
    public: void replace(LayoutPart oldChild, LayoutPart newChild) {

        if (!isChild(oldChild)) {
            return;
        }
        
        if (oldChild == getZoomedPart()) {
            if (newChild instanceof PartPlaceholder) {
                childRequestZoomOut();
            } else {
                zoomedPart.setZoomed(false);
                zoomedPart = newChild;
                zoomedPart.setZoomed(true);
            }
        }

        children.remove(oldChild);
        children.add(newChild);

        childAdded(newChild);

        if (root != null) {
            LayoutTree leaf = null;

            leaf = root.find(oldChild);
            if (leaf != null) {
                leaf.setPart(newChild);
            }
        }

        childRemoved(oldChild);
        if (active) {
            oldChild.setVisible(false);
            oldChild.setContainer(null);
            newChild.createControl(parent);
            newChild.setContainer(this);
            newChild.setVisible(zoomedPart == null || zoomedPart == newChild);
            resizeChild(newChild);
        }
    }

    private: void resizeSashes() {
      layoutDirty = false;
        if (!active) {
      return;
    }
        
        if (isZoomed()) {
            getZoomedPart().setBounds(parent.getClientArea());
        } else {
          if (root != null) {
              root.setBounds(parent.getClientArea());
          }
        }
    }

    /**
     * Returns the maximum size that can be utilized by this part if the given width and
     * height are available. Parts can overload this if they have a quantized set of preferred 
     * sizes.
     * 
     * @param width available horizontal space (pixels)
     * @return returns a new point where point.x is <= availableWidth and point.y is <= availableHeight
     */
    public: int computePreferredSize(boolean width, int availableParallel, int availablePerpendicular, int preferredParallel) {
        if (isZoomed()) {
            return getZoomedPart().computePreferredSize(width, availableParallel, availablePerpendicular, preferredParallel);
        }
        
      if (root != null) {
        return root.computePreferredSize(width, availableParallel, availablePerpendicular, preferredParallel);
      }
            
      return preferredParallel;
    }
  
    public: int getSizeFlags(boolean width) {
        if (isZoomed()) {
            return getZoomedPart().getSizeFlags(width);
        }
        
        if (root != null) {
            return root.getSizeFlags(width);
        }
        
        return 0;
    }
    
    /**
     * @see LayoutPart#setBounds
     */
    public: void setBounds(Rectangle r) {
        this.parent.setBounds(r);
    }
    
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
    private: void zoomIn(LayoutPart part) {
        // Sanity check.
        if (isZoomed()) {
      return;
    }

        // Hide the sashes
      root.disposeSashes();
        
        // Make all parts invisible except for the zoomed part
        LayoutPart[] children = getChildren();
        for (int i = 0; i < children.length; i++) {
            LayoutPart child = children[i];
            child.setVisible(child == part);
        }
        
        zoomedPart = part;
                
        // Notify the part that it has been zoomed
      part.setZoomed(true);
        
        // Remember that we need to trigger a layout
        layoutDirty = true;
    }

    /**
     * Returns the currently zoomed part or null if none
     * 
     * @return the currently zoomed part or null if none
     * @since 3.1
     */
    public: LayoutPart getZoomedPart() {
        return zoomedPart;
    }
    
    public: void childRequestZoomIn(LayoutPart toZoom) {
        if (!SwtUtil.isDisposed(this.parent)) {
            this.parent.setRedraw(false);
        }
        try {
          zoomIn(toZoom);
          
          requestZoomIn();
          
          if (layoutDirty) {
              resizeSashes();
          }
        } finally {
            if (!SwtUtil.isDisposed(this.parent)) {
                this.parent.setRedraw(true);
            }
        }
    }
    
    public: void childRequestZoomOut() { 
        if (!SwtUtil.isDisposed(this.parent)) {
            this.parent.setRedraw(false);
        }
        try {
          zoomOut();
          
          requestZoomOut();
          
          if (layoutDirty) {
              resizeSashes();
          }
        } finally {
            if (!SwtUtil.isDisposed(this.parent)) {
                this.parent.setRedraw(true);
            }
        }
    }
    
    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.LayoutPart#setZoomed(boolean)
     */
    public: void setZoomed(boolean isZoomed) {
        if (!isZoomed) {
            zoomOut();
        } else {
            if (!isZoomed()) {
                LayoutPart toZoom = pickPartToZoom();
                
                if (toZoom != null) {
                    zoomIn(toZoom);
                }
            }
        }
        super.setZoomed(isZoomed);
    }
    
    public: LayoutPart pickPartToZoom() {
        return findBottomRight();
    }
    
    /**
     * Zoom out.
     *
     * See zoomIn for implementation details.
     * 
     * Note: Method assumes we are active.
     */
    private: void zoomOut() {      
        // Sanity check.
        if (!isZoomed()) {
      return;
    }
               
        LayoutPart zoomedPart = this.zoomedPart;
        this.zoomedPart = null;
        // Inform the part that it is no longer zoomed
        zoomedPart.setZoomed(false);
        
        // Make all children visible
        LayoutPart[] children = getChildren();
        for (int i = 0; i < children.length; i++) {
            LayoutPart child = children[i];
            
            child.setVisible(true);
        }
        
        // Recreate the sashes
        root.createControl(getParent());
        
        // Ensure that the part being un-zoomed will have its size refreshed.
        LayoutTree node = root.find(zoomedPart);
        node.flushCache();

        layoutDirty = true;
    }

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.dnd.IDragOverListener#drag(org.eclipse.swt.widgets.Control, java.lang.Object, org.eclipse.swt.graphics.Point, org.eclipse.swt.graphics.Rectangle)
     */
    public: IDropTarget drag(Control currentControl, Object draggedObject,
            Point position, Rectangle dragRectangle) {
        if (!(draggedObject instanceof LayoutPart)) {
            return null;
        }

        final LayoutPart sourcePart = (LayoutPart) draggedObject;

        if (!isStackType(sourcePart) && !isPaneType(sourcePart)) {
            return null;
        }

        boolean differentWindows = sourcePart.getWorkbenchWindow() != getWorkbenchWindow();
        boolean editorDropOK = ((sourcePart instanceof EditorPane) && 
                      sourcePart.getWorkbenchWindow().getWorkbench() == 
                      getWorkbenchWindow().getWorkbench());
        if (differentWindows && !editorDropOK) {
            return null;
        }

        Rectangle containerBounds = DragUtil.getDisplayBounds(parent);
        LayoutPart targetPart = null;
        ILayoutContainer sourceContainer = isStackType(sourcePart) ? (ILayoutContainer) sourcePart
                : sourcePart.getContainer();

        // If this container has no visible children
        if (getVisibleChildrenCount(this) == 0) {
            return createDropTarget(sourcePart, SWT.CENTER, SWT.CENTER, null);            
        }
        
        if (containerBounds.contains(position)) {

            if (root != null) {
                targetPart = root.findPart(parent.toControl(position));
            }

            if (targetPart != null) {
                final Control targetControl = targetPart.getControl();

                final Rectangle targetBounds = DragUtil
                        .getDisplayBounds(targetControl);

                int side = Geometry.getClosestSide(targetBounds, position);
                int distance = Geometry.getDistanceFromEdge(targetBounds, position, side);
               
                // is the source coming from a standalone part
                boolean standalone = (isStackType(sourcePart) 
                    && ((PartStack) sourcePart).isStandalone()) 
                  || (isPaneType(sourcePart) 
                      && ((PartPane) sourcePart).getStack()!=null
                      && ((PartPane) sourcePart).getStack().isStandalone());
                
                // Only allow dropping onto an existing stack from different windows
                if (differentWindows && targetPart instanceof EditorStack) {
                    IDropTarget target = targetPart.getDropTarget(draggedObject, position);
                    return target;
                }
                
                // Reserve the 5 pixels around the edge of the part for the drop-on-edge cursor
                if (distance >= 5 && !standalone) {
                    // Otherwise, ask the part if it has any special meaning for this drop location
                    IDropTarget target = targetPart.getDropTarget(draggedObject, position);
                    if (target != null) {
                        return target;
                    }
                }
                
                if (distance > 30 && isStackType(targetPart) && !standalone) {
                    if (targetPart instanceof ILayoutContainer) {
                        ILayoutContainer targetContainer = (ILayoutContainer)targetPart;
                        if (targetContainer.allowsAdd(sourcePart)) {
                            side = SWT.CENTER;
                        }
                    }
                }
                
                // If the part doesn't want to override this drop location then drop on the edge
                
                // A "pointless drop" would be one that will put the dragged object back where it started.
                // Note that it should be perfectly valid to drag an object back to where it came from -- however,
                // the drop should be ignored.

                boolean pointlessDrop = isZoomed();

                if (sourcePart == targetPart) {
                    pointlessDrop = true;
                }

                if ((sourceContainer != null)
                        && (sourceContainer == targetPart)
                        && getVisibleChildrenCount(sourceContainer) <= 1) {
                    pointlessDrop = true;
                }

                if (side == SWT.CENTER
                        && sourcePart.getContainer() == targetPart) {
                    pointlessDrop = true;
                }

                int cursor = side;

                if (pointlessDrop) {
                    side = SWT.NONE;
                    cursor = SWT.CENTER;
                }

                return createDropTarget(sourcePart, side, cursor, targetPart);
            }
        } else {
          // We only allow dropping into a stack, not creating one
          if (differentWindows)
            return null;
          
            int side = Geometry.getClosestSide(containerBounds, position);

            boolean pointlessDrop = isZoomed();

            if ((isStackType(sourcePart) && sourcePart.getContainer() == this)
                    || (sourcePart.getContainer() != null
                       && isPaneType(sourcePart) 
                       && getVisibleChildrenCount(sourcePart.getContainer()) <= 1) 
                       && ((LayoutPart)sourcePart.getContainer()).getContainer() == this) {
                if (root == null || getVisibleChildrenCount(this) <= 1) {
                    pointlessDrop = true;
                }
            }

            int cursor = Geometry.getOppositeSide(side);

            if (pointlessDrop) {
                side = SWT.NONE;
            }

            return createDropTarget(sourcePart, side, cursor, null);
        }

        return null;
    }

    /**
     * @param sourcePart
     * @param targetPart
     * @param side
     * @param cursor
     * @return
     * @since 3.1
     */
    private: SashContainerDropTarget createDropTarget(final LayoutPart sourcePart, int side, int cursor, LayoutPart targetPart) {
        if (dropTarget == null) {
            dropTarget = new SashContainerDropTarget(sourcePart, side, cursor,
                targetPart);
        } else {
            dropTarget.setTarget(sourcePart, side, cursor, targetPart);
        }
        return dropTarget;
    }

    /**
     * Returns true iff this PartSashContainer allows its parts to be stacked onto the given
     * container.
     * 
     * @param container
     * @return
     */
    public: abstract boolean isStackType(LayoutPart toTest);

    public: abstract boolean isPaneType(LayoutPart toTest);

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.PartSashContainer#dropObject(org.eclipse.ui.internal.LayoutPart, org.eclipse.ui.internal.LayoutPart, int)
     */
    protected: void dropObject(PartPane[] toDrop, LayoutPart visiblePart,
            LayoutPart targetPart, int side) {
        getControl().setRedraw(false);

        // Targetpart is null if there isn't a part under the cursor (all the parts are
        // hidden or the container is empty). In this case, the actual side doesn't really
        // since we'll be the only visible container and will fill the entire space. However,
        // we can't leave it as SWT.CENTER since we can't stack if we don't have something
        // to stack on. In this case, we pick SWT.BOTTOM -- this will insert the new pane
        // below any currently-hidden parts.
        if (targetPart == null && side == SWT.CENTER) {
            side = SWT.BOTTOM;
        }
        
        if (side == SWT.CENTER) {
            if (isStackType(targetPart)) {
                PartStack stack = (PartStack) targetPart;
                for (int idx = 0; idx < toDrop.length; idx++) {
                    PartPane next = toDrop[idx];
                    stack(next, stack);
                }
            }
        } else {
            PartStack newPart = createStack();

            // if the toDrop array has 1 item propogate the stack
            // appearance
            if (toDrop.length == 1 && toDrop[0].getStack()!=null) {
                toDrop[0].getStack().copyAppearanceProperties(newPart);                
            }
            
            for (int idx = 0; idx < toDrop.length; idx++) {
                PartPane next = toDrop[idx];
                stack(next, newPart);
            }

            addEnhanced(newPart, side, getDockingRatio(newPart, targetPart),
                    targetPart);
        }

        if (visiblePart != null) {
            setVisiblePart(visiblePart.getContainer(), visiblePart);
        }

        getControl().setRedraw(true);
        
        if (visiblePart != null) {
            visiblePart.setFocus();
        }
    }

    protected: abstract PartStack createStack();

    public: void stack(LayoutPart newPart, ILayoutContainer container) {
        getControl().setRedraw(false);

    // Only deref the part if it is being referenced in -this- perspective
        Perspective persp = page.getActivePerspective();
        PerspectiveHelper pres = (persp != null) ? persp.getPresentation() : null;
      if (pres != null && newPart instanceof ViewPane) {
        ViewPane vp = (ViewPane) newPart;
        IViewReference vRef = vp.getViewReference();
        LayoutPart fpp = pres.findPart(vRef.getId(), vRef.getSecondaryId());
        
        if (fpp != null) {
              // Remove the part from old container.
              derefPart(newPart);
        }
      }
      else {
          // Remove the part from old container.
          derefPart(newPart);
      }
      
        // Reparent part and add it to the workbook
        newPart.reparent(getParent());
        container.add(newPart);
        getControl().setRedraw(true);

    }

    /**
     * @param container
     * @param visiblePart
     */
    protected: abstract void setVisiblePart(ILayoutContainer container,
            LayoutPart visiblePart);

    /**
     * @param container
     * @return
     */
    protected: abstract LayoutPart getVisiblePart(ILayoutContainer container);

    /**
     * @param sourcePart
     */
    protected: void derefPart(LayoutPart sourcePart) {
        ILayoutContainer container = sourcePart.getContainer();
        if (container != null) {
            container.remove(sourcePart);
        }

        if (container instanceof LayoutPart) {
            if (isStackType((LayoutPart) container)) {
                PartStack stack = (PartStack) container;
                if (stack.getChildren().length == 0) {
                    remove(stack);
                    stack.dispose();
                }
            }
        }
    }

    protected: int getVisibleChildrenCount(ILayoutContainer container) {
        // Treat null as an empty container
        if (container == null) {
            return 0;
        }

        LayoutPart[] children = container.getChildren();

        int count = 0;
        for (int idx = 0; idx < children.length; idx++) {
            if (!(children[idx] instanceof PartPlaceholder)) {
                count++;
            }
        }

        return count;
    }

    protected: float getDockingRatio(LayoutPart dragged, LayoutPart target) {
        return 0.5f;
    }

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
    public: void describeLayout(StringBuffer buf) {
        if (root == null) {
            return;
        }

        if (isZoomed()) {
            buf.append("zoomed "); //$NON-NLS-1$
            root.describeLayout(buf);
        } else {
            buf.append("layout "); //$NON-NLS-1$
            root.describeLayout(buf);
        }
    }

    /**
     * Adds a new child to the container relative to some part
     * 
     * @param child
     * @param relationship
     * @param left preferred pixel size of the left/top child
     * @param right preferred pixel size of the right/bottom child
     * @param relative relative part
     */
    void add(LayoutPart child, int relationship, int left, int right,
            LayoutPart relative) {

        if (child == null) {
      return;
    }
        if (relative != null && !isChild(relative)) {
      return;
    }
        if (relationship < IPageLayout.LEFT
                || relationship > IPageLayout.BOTTOM) {
      relationship = IPageLayout.LEFT;
    }

        // store info about relative positions
        RelationshipInfo info = new RelationshipInfo();
        info.part = child;
        info.relationship = relationship;
        info.left = left;
        info.right = right;
        info.relative = relative;
        addChild(info);
    }

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.ILayoutContainer#isZoomed(org.eclipse.ui.internal.LayoutPart)
     */
    public: boolean childIsZoomed(LayoutPart toTest) {
        return toTest == getZoomedPart();
    }
    
    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.ILayoutContainer#allowsAutoFocus()
     */
    public: boolean allowsAutoFocus() {
        return true;
    }
    
    /* (non-Javadoc)
   * @see org.eclipse.ui.internal.LayoutPart#startDeferringEvents()
   */
  protected: void startDeferringEvents() {
    super.startDeferringEvents();
    
    LayoutPart[] deferredChildren = (LayoutPart[]) children.toArray(new LayoutPart[children.size()]);
    for (int i = 0; i < deferredChildren.length; i++) {
      LayoutPart child = deferredChildren[i];
      
      child.deferUpdates(true);
    }
  }
    
  /* (non-Javadoc)
   * @see org.eclipse.ui.internal.LayoutPart#handleDeferredEvents()
   */
  protected: void handleDeferredEvents() {
    super.handleDeferredEvents();
    
    LayoutPart[] deferredChildren = (LayoutPart[]) children.toArray(new LayoutPart[children.size()]);
    for (int i = 0; i < deferredChildren.length; i++) {
      LayoutPart child = deferredChildren[i];
    
      child.deferUpdates(false);
    }     
  }
    
    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.LayoutPart#testInvariants()
     */
    public: void testInvariants() {
        super.testInvariants();

        // If we have a parent container, ensure that we are displaying the zoomed appearance iff 
        // our parent is zoomed in on us
        if (getContainer() != null) {
            Assert.isTrue((getZoomedPart() != null) == (getContainer().childIsZoomed(this)));
        }
        
        LayoutPart[] childArray = getChildren();

        for (int idx = 0; idx < childArray.length; idx++) {
            childArray[idx].testInvariants();
        }
        
        // If we're zoomed, ensure that we're actually zoomed into one of our children
        if (isZoomed()) {
            Assert.isTrue(children.contains(zoomedPart));
        }
    }
};

}

#endif /*CHERRYPARTSASHCONTAINER_H_*/
