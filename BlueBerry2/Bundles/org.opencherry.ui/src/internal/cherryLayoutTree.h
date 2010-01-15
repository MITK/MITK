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

#include "../cherryISizeProvider.h"
#include "../cherryRectangle.h"
#include "../cherryPoint.h"

#include "cherryPartSashContainer.h"

#include <list>

namespace cherry
{

class LayoutTreeNode;
class LayoutPartSash;

/**
 * \ingroup org_opencherry_ui_internal
 *
 * Implementation of a tree where the node is allways a sash
 * and it allways has two chidren. If a children is removed
 * the sash, ie the node, is removed as well and its other children
 * placed on its parent.
 */
class LayoutTree : public Object, public ISizeProvider
{ //implements ISizeProvider {

public:

  osgiObjectMacro(LayoutTree);

  /* The parent of this tree or null if it is the root */
  LayoutTreeNode* parent;

  /* Any LayoutPart if this is a leaf or a LayoutSashPart if it is a node */
  LayoutPart::Pointer part;

private:

  // Cached information
  int cachedMinimumWidthHint;
  int cachedMinimumWidth;
  int cachedMinimumHeightHint;
  int cachedMinimumHeight;
  int cachedMaximumWidthHint;
  int cachedMaximumWidth;
  int cachedMaximumHeightHint;
  int cachedMaximumHeight;

  bool forceLayout;
  Rectangle currentBounds;

  // Cached size flags
  bool sizeFlagsDirty;
  int widthSizeFlags;
  int heightSizeFlags;

public:

  // Cache statistics. For use in benchmarks and test suites only!
  static int minCacheHits;
  static int minCacheMisses;
  static int maxCacheHits;
  static int maxCacheMisses;

  /**
   * Initialize this tree with its part.
   */
  LayoutTree(LayoutPart::Pointer part);

  /**
   * Add the relation ship between the children in the list
   * and returns the left children.
   */
  virtual LayoutPart::Pointer ComputeRelation(
      std::list<PartSashContainer::RelationshipInfo>& relations);

  /**
   * Locates the part that intersects the given point
   *
   * @param toFind
   * @return
   */
  virtual LayoutPart::Pointer FindPart(const Point& toFind);

  /**
   * Dispose all Sashs in this tree
   */
  virtual void DisposeSashes();

  /**
   * Find a LayoutPart in the tree and return its sub-tree. Returns
   * null if the child is not found.
   */
  virtual SmartPointer<LayoutTree> Find(LayoutPart::Pointer child);

  /**
   * Find the Left,Right,Top and Bottom
   * sashes around this tree and set them
   * in <code>sashes</code>
   */
  virtual void FindSashes(PartPane::Sashes sashes);

  /**
   * Find the part that is in the bottom right position.
   */
  virtual LayoutPart::Pointer FindBottomRight();

  /**
   * Find a sash in the tree and return its sub-tree. Returns
   * null if the sash is not found.
   */
  virtual SmartPointer<LayoutTreeNode> FindSash(SmartPointer<LayoutPartSash> sash);

  /**
   * Return the bounds of this tree which is the rectangle that
   * contains all Controls in this tree.
   */
  Rectangle GetBounds();

  /**
   * Subtracts two integers. If a is INF, this is treated as
   * positive infinity.
   *
   * @param a a positive integer or INF indicating positive infinity
   * @param b a positive integer (may not be INF)
   * @return a - b, or INF if a == INF
   * @since 3.1
   */
  static int Subtract(int a, int b);

  /**
   * Adds two positive integers. Treates INF as positive infinity.
   *
   * @param a a positive integer
   * @param b a positive integer
   * @return a + b, or INF if a or b are positive infinity
   * @since 3.1
   */
  static int Add(int a, int b);

  /**
   * Asserts that toCheck is a positive integer less than INF / 2 or equal
   * to INF. Many of the methods of this class use positive integers as sizes,
   * with INF indicating positive infinity. This picks up accidental addition or
   * subtraction from infinity.
   *
   * @param toCheck integer to validate
   * @since 3.1
   */
  static void AssertValidSize(int toCheck);

  /**
   * Computes the preferred size for this object. The interpretation of the result depends on the flags returned
   * by getSizeFlags(). If the caller is looking for a maximum or minimum size, this delegates to computeMinimumSize
   * or computeMaximumSize in order to benefit from caching optimizations. Otherwise, it delegates to
   * doComputePreferredSize. Subclasses should overload one of doComputeMinimumSize, doComputeMaximumSize, or
   * doComputePreferredSize to specialize the return value.
   *
   * @see LayoutPart#computePreferredSize(boolean, int, int, int)
   */
  int ComputePreferredSize(bool width, int availableParallel,
      int availablePerpendicular, int preferredParallel);

protected:

  /**
   * Returns the size flags for this tree.
   *
   * @see org.opencherry.ui.presentations.StackPresentation#getSizeFlags(boolean)
   *
   * @param b indicates whether the caller wants the flags for computing widths (=true) or heights (=false)
   * @return a bitwise combiniation of flags with the same meaning as StackPresentation.getSizeFlags(boolean)
   */
  virtual int DoGetSizeFlags(bool width);

  /**
   * Subclasses should overload this method instead of computePreferredSize(boolean, int, int, int)
   *
   * @see org.opencherry.ui.presentations.StackPresentation#computePreferredSize(boolean, int, int, int)
   *
   * @since 3.1
   */
  virtual int DoComputePreferredSize(bool width, int availableParallel,
      int availablePerpendicular, int preferredParallel);

public:

  /**
   * Returns the minimum size for this subtree. Equivalent to calling
   * computePreferredSize(width, INF, availablePerpendicular, 0).
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
  int ComputeMinimumSize(bool width, int availablePerpendicular);

  /**
   * For use in benchmarks and test suites only. Displays cache utilization statistics for all
   * LayoutTree instances.
   *
   * @since 3.1
   */
  static void PrintCacheStatistics();

  virtual int DoComputeMinimumSize(bool width, int availablePerpendicular);

  int ComputeMaximumSize(bool width, int availablePerpendicular);

protected:
  virtual int DoComputeMaximumSize(bool width, int availablePerpendicular);

public:

  /**
   * Called to flush any cached information in this tree and its parents.
   */
  virtual void FlushNode();

  /**
   * Flushes all cached information about this node and all of its children.
   * This should be called if something may have caused all children to become
   * out of synch with their cached information (for example, if a lot of changes
   * may have happened without calling flushCache after each change)
   *
   * @since 3.1
   */
  virtual void FlushChildren();

  /**
   * Flushes all cached information about this node and all of its ancestors.
   * This should be called when a single child changes.
   *
   * @since 3.1
   */
  void FlushCache();

  int GetSizeFlags(bool width);

  /**
   * Returns the parent of this tree or null if it is the root.
   */
  virtual LayoutTreeNode* GetParent() const;

  /**
   * Inserts a new child on the tree. The child will be placed beside
   * the <code>relative</code> child. Returns the new root of the tree.
   */
  virtual SmartPointer<LayoutTree> Insert(LayoutPart::Pointer child, bool left,
      SmartPointer<LayoutPartSash> sash, SmartPointer<LayoutPart> relative);

  /**
   * Returns true if this tree can be compressed and expanded.
   * @return true if springy
   */
  virtual bool IsCompressible();

  /**
   * Returns true if this tree has visible parts otherwise returns false.
   */
  virtual bool IsVisible();

  /**
   * Recompute the ratios in this tree.
   */
  virtual void RecomputeRatio();

  /**
   * Find a child in the tree and remove it and its parent.
   * The other child of its parent is placed on the parent's parent.
   * Returns the new root of the tree.
   */
  virtual SmartPointer<LayoutTree> Remove(LayoutPart::Pointer child);

  /**
   * Sets the bounds of this node. If the bounds have changed or any children have
   * changed then the children will be recursively layed out. This implementation
   * filters out redundant calls and delegates to doSetBounds to layout the children.
   * Subclasses should overload doSetBounds to lay out their children.
   *
   * @param bounds new bounds of the tree
   */
  void SetBounds(const Rectangle& bounds);

protected:

  /**
   * Resize the parts on this tree to fit in <code>bounds</code>.
   */
  virtual void DoSetBounds(const Rectangle& bounds);

public:

  /**
   * Set the parent of this tree.
   */
  virtual void SetParent(LayoutTreeNode* parent);

  /**
   * Set the part of this leaf
   */
  virtual void SetPart(LayoutPart::Pointer part);

  /**
   * Returns a string representation of this object.
   */
  virtual std::string ToString();

  /**
   * Creates SWT controls owned by the LayoutTree (ie: the sashes). Does not affect the
   * LayoutParts that are being arranged by the LayoutTree.
   *
   * @param parent
   * @since 3.1
   */
  virtual void CreateControl(void* parent);

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
  virtual void DescribeLayout(std::string& buf) const;

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
  bool HasSizeFlag(bool width, int flag);

};

}

#endif /*CHERRYLAYOUTTREE_H_*/
