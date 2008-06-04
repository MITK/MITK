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

#ifndef CHERRYLAYOUTTREE_H_
#define CHERRYLAYOUTTREE_H_

#include "cherryRectangle.h"

namespace cherry {

class LayoutTreeNode;

/**
 * \ingroup org_opencherry_ui_internal
 * 
 * Implementation of a tree where the node is allways a sash
 * and it allways has two chidren. If a children is removed
 * the sash, ie the node, is removed as well and its other children
 * placed on its parent.
 */
class LayoutTree { //implements ISizeProvider {
  
public:
  
    /* The parent of this tree or null if it is the root */
    LayoutTreeNode* parent;

    /* Any LayoutPart if this is a leaf or a LayoutSashPart if it is a node */
    LayoutPart::Pointer part;
    
private:
  
    // Cached information
      int cachedMinimumWidthHint = SWT.DEFAULT;
      int cachedMinimumWidth = SWT.DEFAULT;
      int cachedMinimumHeightHint = SWT.DEFAULT;
      int cachedMinimumHeight = SWT.DEFAULT;
      int cachedMaximumWidthHint = SWT.DEFAULT;
      int cachedMaximumWidth = SWT.DEFAULT;
      int cachedMaximumHeightHint = SWT.DEFAULT;
      int cachedMaximumHeight = SWT.DEFAULT;
      
      bool forceLayout = true;
      Rectangle currentBounds;
    
    // Cached size flags
      bool sizeFlagsDirty = true;
      int widthSizeFlags = 0;
      int heightSizeFlags = 0;
    
public:
  
    // Cache statistics. For use in benchmarks and test suites only!
      static int minCacheHits;
      static int minCacheMisses;
      static int maxCacheHits;
      static int maxCacheMisses;
    
    
    
    /**
     * Initialize this tree with its part.
     */
      LayoutTree(LayoutPart::Pointer part) {
        this.part = part;
    }

    /**
     * Add the relation ship between the children in the list
     * and returns the left children.
     */
     virtual LayoutPart::Pointer ComputeRelation(std::list<PartSashContainer::RelationshipInfo> relations) {
        return part;
    }

    /**
     * Locates the part that intersects the given point
     * 
     * @param toFind
     * @return
     */
     virtual LayoutPart::Pointer FindPart(int x, int y) {
        return part;
    }
    
    /**
     * Dispose all Sashs in this tree
     */
     virtual void DisposeSashes() {
    }
    
    /**
     * Find a LayoutPart in the tree and return its sub-tree. Returns
     * null if the child is not found.
     */
     virtual LayoutTree* Find(LayoutPart::Pointer child) {
        if (part != child) {
      return null;
    }
        return this;
    }

    /**
     * Find the Left,Right,Top and Botton 
     * sashes around this tree and set them
     * in <code>sashes</code>
     */
     virtual void FindSashes(PartPane::Sashes sashes) {
        if (getParent() == null) {
      return;
    }
        getParent().findSashes(this, sashes);
    }

    /**
     * Find the part that is in the bottom rigth possition.
     */
     virtual LayoutPart::Pointer FindBottomRight() {
        return part;
    }
    
    /**
     * Find a sash in the tree and return its sub-tree. Returns
     * null if the sash is not found.
     */
     virtual LayoutTreeNode* FindSash(LayoutPartSash::Pointer sash) {
        return null;
    }

    /**
     * Return the bounds of this tree which is the rectangle that
     * contains all Controls in this tree.
     */
      Rectangle GetBounds() {
        return Geometry.copy(currentBounds);
    }
    
    /**
     * Subtracts two integers. If a is INFINITE, this is treated as
     * positive infinity. 
     * 
     * @param a a positive integer or INFINITE indicating positive infinity
     * @param b a positive integer (may not be INFINITE)
     * @return a - b, or INFINITE if a == INFINITE
     * @since 3.1
     */
      static int Subtract(int a, int b) {
        Assert.isTrue(b >= 0 && b < INFINITE);
        
      return add(a, -b);
    }
    
    /**
     * Adds two positive integers. Treates INFINITE as positive infinity.
     * 
     * @param a a positive integer
     * @param b a positive integer
     * @return a + b, or INFINITE if a or b are positive infinity
     * @since 3.1
     */
      static int Add(int a, int b) {
      if (a == INFINITE || b == INFINITE) {
        return INFINITE;
      }
      
      return a + b;
    }
    
    /**
     * Asserts that toCheck is a positive integer less than INFINITE / 2 or equal
     * to INFINITE. Many of the methods of this class use positive integers as sizes,
     * with INFINITE indicating positive infinity. This picks up accidental addition or
     * subtraction from infinity. 
     * 
     * @param toCheck integer to validate
     * @since 3.1
     */
      static void AssertValidSize(int toCheck) {
      Assert.isTrue(toCheck >= 0 && (toCheck == INFINITE || toCheck < INFINITE / 2));
    }
    
    /**
     * Computes the preferred size for this object. The interpretation of the result depends on the flags returned
     * by getSizeFlags(). If the caller is looking for a maximum or minimum size, this delegates to computeMinimumSize
     * or computeMaximumSize in order to benefit from caching optimizations. Otherwise, it delegates to 
     * doComputePreferredSize. Subclasses should overload one of doComputeMinimumSize, doComputeMaximumSize, or
     * doComputePreferredSize to specialize the return value. 
     * 
     * @see LayoutPart#computePreferredSize(boolean, int, int, int)
     */
      int ComputePreferredSize(bool width, int availableParallel, int availablePerpendicular, int preferredParallel) {
      assertValidSize(availableParallel);
      assertValidSize(availablePerpendicular);
      assertValidSize(preferredParallel);
      
      if (!isVisible()) {
        return 0;
      }

      if (availableParallel == 0) {
        return 0;
      }

      if (preferredParallel == 0) {
        return Math.min(availableParallel, computeMinimumSize(width, availablePerpendicular));
      } else if (preferredParallel == INFINITE && availableParallel == INFINITE) {
        return computeMaximumSize(width, availablePerpendicular);
      }
      
      // Optimization: if this subtree doesn't have any size preferences beyond its minimum and maximum
      // size, simply return the preferred size
      if (!hasSizeFlag(width, SWT.FILL)) {
          return preferredParallel;
      }
      
      int result = doComputePreferredSize(width, availableParallel, availablePerpendicular, preferredParallel);

      return result;
    }
    
protected:
  
    /**
     * Returns the size flags for this tree. 
     * 
     * @see org.eclipse.ui.presentations.StackPresentation#getSizeFlags(boolean)
     * 
   * @param b indicates whether the caller wants the flags for computing widths (=true) or heights (=false)
   * @return a bitwise combiniation of flags with the same meaning as StackPresentation.getSizeFlags(boolean)
   */
  virtual int DoGetSizeFlags(bool width) {
    return part.getSizeFlags(width);
  }

  /**
   * Subclasses should overload this method instead of computePreferredSize(boolean, int, int, int)
   * 
   * @see org.eclipse.ui.presentations.StackPresentation#computePreferredSize(boolean, int, int, int)
   * 
   * @since 3.1
   */
  virtual int DoComputePreferredSize(bool width, int availableParallel, int availablePerpendicular, int preferredParallel) {
      int result = Math.min(availableParallel, 
          part.computePreferredSize(width, availableParallel, availablePerpendicular, preferredParallel));

      assertValidSize(result);
      return result;      
    }
  
public:
  
  /**
   * Returns the minimum size for this subtree. Equivalent to calling 
   * computePreferredSize(width, INFINITE, availablePerpendicular, 0).
   * Returns a cached value if possible or defers to doComputeMinimumSize otherwise.
   * Subclasses should overload doComputeMinimumSize if they want to specialize the
   * return value.
   * 
   * @param width true iff computing the minimum width, false iff computing the minimum height
   * @param availablePerpendicular available space (pixels) perpendicular to the dimension 
   * being computed. This is a height when computing a width, or a width when computing a height.
   * 
   * @see LayoutPart#computePreferredSize(boolean, int, int, int)
   */
    virtual int ComputeMinimumSize(bool width, int availablePerpendicular) {
      assertValidSize(availablePerpendicular);
      
      // Optimization: if this subtree has no minimum size, then always return 0 as its
      // minimum size.
        if (!hasSizeFlag(width, SWT.MIN)) {
            return 0;
        }
      
      // If this subtree doesn't contain any wrapping controls (ie: they don't care
      // about their perpendicular size) then force the perpendicular
      // size to be INFINITE. This ensures that we will get a cache hit
      // every time for non-wrapping controls.
      if (!hasSizeFlag(width, SWT.WRAP)) {
        availablePerpendicular = INFINITE;
      }
      
      if (width) {
          // Check if we have a cached width measurement (we can only return a cached
          // value if we computed it for the same height)
        if (cachedMinimumWidthHint == availablePerpendicular) {
          minCacheHits++;
          return cachedMinimumWidth;
        }
        
        // Recompute the minimum width and store it in the cache
        
        minCacheMisses++;
         
        int result = doComputeMinimumSize(width, availablePerpendicular);
        cachedMinimumWidth = result;
        cachedMinimumWidthHint = availablePerpendicular;
        return result;
        
      } else {
          // Check if we have a cached height measurement (we can only return a cached
          // value if we computed it for the same width)
        if (cachedMinimumHeightHint == availablePerpendicular) {
          minCacheHits++;
          return cachedMinimumHeight;
        }
        
        // Recompute the minimum width and store it in the cache
        minCacheMisses++;
        
        int result = doComputeMinimumSize(width, availablePerpendicular);
        cachedMinimumHeight = result;
        cachedMinimumHeightHint = availablePerpendicular;
        return result;
      }
    }
    
    /**
     * For use in benchmarks and test suites only. Displays cache utilization statistics for all
     * LayoutTree instances.
     * 
     * @since 3.1
     */
      static void PrintCacheStatistics() {
      System.out.println("minimize cache " + minCacheHits + " / " + (minCacheHits + minCacheMisses) + " hits " + //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
          minCacheHits * 100 / (minCacheHits + minCacheMisses) + "%"); //$NON-NLS-1$
      System.out.println("maximize cache " + maxCacheHits + " / " + (maxCacheHits + maxCacheMisses) + " hits" + //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
          maxCacheHits * 100 / (maxCacheHits + maxCacheMisses) + "%"); //$NON-NLS-1$
    }
    
   virtual int DoComputeMinimumSize(boolean width, int availablePerpendicular) {
    int result = doComputePreferredSize(width, INFINITE, availablePerpendicular, 0);
    assertValidSize(result);
    return result;
  }

      int ComputeMaximumSize(boolean width, int availablePerpendicular) {
      assertValidSize(availablePerpendicular);
      
      // Optimization: if this subtree has no maximum size, then always return INFINITE as its
      // maximum size.
        if (!hasSizeFlag(width, SWT.MAX)) {
            return INFINITE;
        }
      
      // If this subtree doesn't contain any wrapping controls (ie: they don't care
      // about their perpendicular size) then force the perpendicular
      // size to be INFINITE. This ensures that we will get a cache hit
      // every time.
      if (!hasSizeFlag(width, SWT.WRAP)) {
        availablePerpendicular = INFINITE;
      }
      
      if (width) {          
          // Check if we have a cached width measurement (we can only return a cached
          // value if we computed it for the same height)
        if (cachedMaximumWidthHint == availablePerpendicular) {
          maxCacheHits++;
          return cachedMaximumWidth;
        }
        
        maxCacheMisses++;
        
        // Recompute the maximum width and store it in the cache
        int result = doComputeMaximumSize(width, availablePerpendicular);
        cachedMaximumWidth = result;
        cachedMaximumWidthHint = availablePerpendicular;
        return result;
        
      } else {
          // Check if we have a cached height measurement
        if (cachedMaximumHeightHint == availablePerpendicular) {
          maxCacheHits++;
          return cachedMaximumHeight;
        }
        
        maxCacheMisses++;
        
        // Recompute the maximum height and store it in the cache
        int result = doComputeMaximumSize(width, availablePerpendicular);
        cachedMaximumHeight = result;
        cachedMaximumHeightHint = availablePerpendicular;
        return result;
      }
    }
    
protected:
  virtual int DoComputeMaximumSize(bool width, int availablePerpendicular) {        
      return doComputePreferredSize(width, INFINITE, availablePerpendicular, INFINITE);
    }
   
public:
  
    /**
     * Called to flush any cached information in this tree and its parents.
     */
    virtual void FlushNode() {
        
        // Clear cached sizes
        cachedMinimumWidthHint = SWT.DEFAULT;
        cachedMinimumWidth = SWT.DEFAULT;
        cachedMinimumHeightHint = SWT.DEFAULT;
        cachedMinimumHeight = SWT.DEFAULT;
        cachedMaximumWidthHint = SWT.DEFAULT;
        cachedMaximumWidth = SWT.DEFAULT;
        cachedMaximumHeightHint = SWT.DEFAULT;
        cachedMaximumHeight = SWT.DEFAULT;
        
        // Flags may have changed. Ensure that they are recomputed the next time around
        sizeFlagsDirty = true;
        
        // The next setBounds call should trigger a layout even if set to the same bounds since
        // one of the children has changed.
        forceLayout = true;
    }
    
    /**
     * Flushes all cached information about this node and all of its children.
     * This should be called if something may have caused all children to become
     * out of synch with their cached information (for example, if a lot of changes
     * may have happened without calling flushCache after each change)
     * 
     * @since 3.1
     */
     virtual void FlushChildren() {
        flushNode();
    }
    
    /**
     * Flushes all cached information about this node and all of its ancestors.
     * This should be called when a single child changes.
     * 
     * @since 3.1
     */
      void FlushCache() {
        flushNode();
        
      if (parent != null) {
        parent.flushCache();
      }        
    }
    
      int GetSizeFlags(bool width) {
        if (sizeFlagsDirty) {
            widthSizeFlags = doGetSizeFlags(true);
            heightSizeFlags = doGetSizeFlags(false);
            sizeFlagsDirty = false;
        }
        
        return width ? widthSizeFlags : heightSizeFlags;
    }
        
    /**
     * Returns the parent of this tree or null if it is the root.
     */
     virtual LayoutTreeNode* GetParent() {
        return parent;
    }

    /**
     * Inserts a new child on the tree. The child will be placed beside 
     * the <code>relative</code> child. Returns the new root of the tree.
     */
     virtual LayoutTree* Insert(LayoutPart::Pointer child, bool left,
            LayoutPartSash::Pointer sash, LayoutPart::Pointer relative) {
        LayoutTree relativeChild = find(relative);
        LayoutTreeNode node = new LayoutTreeNode(sash);
        if (relativeChild == null) {
            //Did not find the relative part. Insert beside the root.
            node.setChild(left, child);
            node.setChild(!left, this);
            return node;
        } else {
            LayoutTreeNode oldParent = relativeChild.getParent();
            node.setChild(left, child);
            node.setChild(!left, relativeChild);
            if (oldParent == null) {
                //It was the root. Return a new root.
                return node;
            }
            oldParent.replaceChild(relativeChild, node);
            return this;
        }
    }

    /**
     * Returns true if this tree can be compressed and expanded.
     * @return true if springy
     */
     virtual bool IsCompressible() {
        //Added for bug 19524
        return part.isCompressible();
    }

    /**
     * Returns true if this tree has visible parts otherwise returns false.
     */
     virtual bool IsVisible() {
        return !(part instanceof PartPlaceholder);
    }

    /**
     * Recompute the ratios in this tree.
     */
     virtual void RecomputeRatio() {
    }

    /**
     * Find a child in the tree and remove it and its parent.
     * The other child of its parent is placed on the parent's parent.
     * Returns the new root of the tree.
     */
     virtual LayoutTree* Remove(LayoutPart::Pointer child) {
        LayoutTree tree = find(child);
        if (tree == null) {
      return this;
    }
        LayoutTreeNode oldParent = tree.getParent();
        if (oldParent == null) {
            //It was the root and the only child of this tree
            return null;
        }
        if (oldParent.getParent() == null) {
      return oldParent.remove(tree);
    }

        oldParent.remove(tree);
        return this;
    }

    /**
     * Sets the bounds of this node. If the bounds have changed or any children have
     * changed then the children will be recursively layed out. This implementation
     * filters out redundant calls and delegates to doSetBounds to layout the children. 
     * Subclasses should overload doSetBounds to lay out their children.  
     * 
     * @param bounds new bounds of the tree
     */
      void SetBounds(const Rectangle& bounds) {
        if (!bounds.equals(currentBounds) || forceLayout) {
            currentBounds = Geometry.copy(bounds);
            
            doSetBounds(currentBounds);
            forceLayout = false;
        } 
    }
    
protected:
  
    /**
     * Resize the parts on this tree to fit in <code>bounds</code>.
     */
    virtual void DoSetBounds(const Rectangle& bounds) {
        part.setBounds(bounds);
    }

public:
  
    /**
     * Set the parent of this tree.
     */
    virtual void SetParent(LayoutTreeNode* parent) {
        this.parent = parent;
    }

    /**
     * Set the part of this leaf
     */
    virtual void SetPart(LayoutPart::Pointer part) {
        this.part = part;
        flushCache();
    }

    /**
     * Returns a string representation of this object.
     */
     virtual std::string ToString() {
        return "(" + part.toString() + ")";//$NON-NLS-2$//$NON-NLS-1$
    }

    /**
     * Creates SWT controls owned by the LayoutTree (ie: the sashes). Does not affect the 
     * LayoutParts that are being arranged by the LayoutTree. 
     * 
     * @param parent
     * @since 3.1
     */
     virtual void CreateControl(void* parent) {        
    }
        
    /**
     * Writes a description of the layout to the given string buffer.
     * This is used for drag-drop test suites to determine if two layouts are the
     * same. Like a hash code, the description should compare as equal iff the
     * layouts are the same. However, it should be user-readable in order to
     * help debug failed tests. Although these are english readable strings,
     * they should not be translated or equality tests will fail.
     * <p>
     * This is only intended for use by test suites.
     * </p>
     * 
     * @param buf
     */
     virtual void DescribeLayout(std::string& buf) {
        part.describeLayout(buf);
    }

    /**
     * This is a shorthand method that checks if the tree contains the
     * given size flag. For example, hasSizeFlag(false, SWT.MIN) returns
     * true iff the receiver enforces a minimum height, or 
     * hasSizeFlag(true, SWT.WRAP) returns true iff the receiver needs to
     * know its height when computing its preferred width.
     * 
     * @param vertical 
     * @return
     * @since 3.1
     */
      boolan HasSizeFlag(bool width, int flag) {        
        return (getSizeFlags(width) & flag) != 0;
    }

};

}

#endif /*CHERRYLAYOUTTREE_H_*/
