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

#include "cherryLayoutTree.h"

#include "cherryLayoutTreeNode.h"

namespace cherry
{

LayoutTree::LayoutTree(LayoutPart::Pointer part)
{
  this.part = part;
}

LayoutPart::Pointer LayoutTree::ComputeRelation(
    std::list<PartSashContainer::RelationshipInfo> relations)
{
  return part;
}

LayoutPart::Pointer LayoutTree::FindPart(int x, int y)
{
  return part;
}

void LayoutTree::DisposeSashes()
{
}

LayoutTree* LayoutTree::Find(LayoutPart::Pointer child)
{
  if (part != child)
  {
    return null;
  }
  return this;
}

void LayoutTree::FindSashes(PartPane::Sashes sashes)
{
  if (getParent() == null)
  {
    return;
  }
  getParent().findSashes(this, sashes);
}

LayoutPart::Pointer LayoutTree::FindBottomRight()
{
  return part;
}

LayoutTreeNode* LayoutTree::FindSash(LayoutPartSash::Pointer sash)
{
  return null;
}

Rectangle LayoutTree::GetBounds()
{
  return Geometry.copy(currentBounds);
}

static int LayoutTree::Subtract(int a, int b)
{
  Assert.isTrue(b >= 0 && b < INFINITE);

  return add(a, -b);
}

static int LayoutTree::Add(int a, int b)
{
  if (a == INFINITE || b == INFINITE)
  {
    return INFINITE;
  }

  return a + b;
}

static void LayoutTree::AssertValidSize(int toCheck)
{
  Assert.isTrue(toCheck >= 0 && (toCheck == INFINITE || toCheck < INFINITE / 2));
}

int LayoutTree::ComputePreferredSize(bool width, int availableParallel,
    int availablePerpendicular, int preferredParallel)
{
  assertValidSize(availableParallel);
  assertValidSize(availablePerpendicular);
  assertValidSize(preferredParallel);

  if (!isVisible())
  {
    return 0;
  }

  if (availableParallel == 0)
  {
    return 0;
  }

  if (preferredParallel == 0)
  {
    return Math.min(availableParallel, computeMinimumSize(width,
        availablePerpendicular));
  }
  else if (preferredParallel == INFINITE && availableParallel == INFINITE)
  {
    return computeMaximumSize(width, availablePerpendicular);
  }

  // Optimization: if this subtree doesn't have any size preferences beyond its minimum and maximum
  // size, simply return the preferred size
  if (!hasSizeFlag(width, SWT.FILL))
  {
    return preferredParallel;
  }

  int result = doComputePreferredSize(width, availableParallel,
      availablePerpendicular, preferredParallel);

  return result;
}

int LayoutTree::DoGetSizeFlags(bool width)
{
  return part.getSizeFlags(width);
}

int LayoutTree::DoComputePreferredSize(bool width, int availableParallel,
    int availablePerpendicular, int preferredParallel)
{
  int result = Math.min(availableParallel, part.computePreferredSize(width,
      availableParallel, availablePerpendicular, preferredParallel));

  assertValidSize(result);
  return result;
}

int LayoutTree::ComputeMinimumSize(bool width, int availablePerpendicular)
{
  assertValidSize(availablePerpendicular);

  // Optimization: if this subtree has no minimum size, then always return 0 as its
  // minimum size.
  if (!hasSizeFlag(width, SWT.MIN))
  {
    return 0;
  }

  // If this subtree doesn't contain any wrapping controls (ie: they don't care
  // about their perpendicular size) then force the perpendicular
  // size to be INFINITE. This ensures that we will get a cache hit
  // every time for non-wrapping controls.
  if (!hasSizeFlag(width, SWT.WRAP))
  {
    availablePerpendicular = INFINITE;
  }

  if (width)
  {
    // Check if we have a cached width measurement (we can only return a cached
    // value if we computed it for the same height)
    if (cachedMinimumWidthHint == availablePerpendicular)
    {
      minCacheHits++;
      return cachedMinimumWidth;
    }

    // Recompute the minimum width and store it in the cache

    minCacheMisses++;

    int result = doComputeMinimumSize(width, availablePerpendicular);
    cachedMinimumWidth = result;
    cachedMinimumWidthHint = availablePerpendicular;
    return result;

  }
  else
  {
    // Check if we have a cached height measurement (we can only return a cached
    // value if we computed it for the same width)
    if (cachedMinimumHeightHint == availablePerpendicular)
    {
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
void LayoutTree::PrintCacheStatistics()
{
  System.out.println("minimize cache " + minCacheHits + " / " + (minCacheHits
      + minCacheMisses) + " hits " + //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
      minCacheHits * 100 / (minCacheHits + minCacheMisses) + "%"); //$NON-NLS-1$
  System.out.println("maximize cache " + maxCacheHits + " / " + (maxCacheHits
      + maxCacheMisses) + " hits" + //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
      maxCacheHits * 100 / (maxCacheHits + maxCacheMisses) + "%"); //$NON-NLS-1$
}

int LayoutTree::DoComputeMinimumSize(boolean width, int availablePerpendicular)
{
  int result = doComputePreferredSize(width, INFINITE, availablePerpendicular,
      0);
  assertValidSize(result);
  return result;
}

int LayoutTree::ComputeMaximumSize(boolean width, int availablePerpendicular)
{
  assertValidSize(availablePerpendicular);

  // Optimization: if this subtree has no maximum size, then always return INFINITE as its
  // maximum size.
  if (!hasSizeFlag(width, SWT.MAX))
  {
    return INFINITE;
  }

  // If this subtree doesn't contain any wrapping controls (ie: they don't care
  // about their perpendicular size) then force the perpendicular
  // size to be INFINITE. This ensures that we will get a cache hit
  // every time.
  if (!hasSizeFlag(width, SWT.WRAP))
  {
    availablePerpendicular = INFINITE;
  }

  if (width)
  {
    // Check if we have a cached width measurement (we can only return a cached
    // value if we computed it for the same height)
    if (cachedMaximumWidthHint == availablePerpendicular)
    {
      maxCacheHits++;
      return cachedMaximumWidth;
    }

    maxCacheMisses++;

    // Recompute the maximum width and store it in the cache
    int result = doComputeMaximumSize(width, availablePerpendicular);
    cachedMaximumWidth = result;
    cachedMaximumWidthHint = availablePerpendicular;
    return result;

  }
  else
  {
    // Check if we have a cached height measurement
    if (cachedMaximumHeightHint == availablePerpendicular)
    {
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

int LayoutTree::DoComputeMaximumSize(bool width, int availablePerpendicular)
{
  return doComputePreferredSize(width, INFINITE, availablePerpendicular,
      INFINITE);
}

void LayoutTree::FlushNode()
{

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

void LayoutTree::FlushChildren()
{
  flushNode();
}

void LayoutTree::FlushCache()
{
  flushNode();

  if (parent != null)
  {
    parent.flushCache();
  }
}

int LayoutTree::GetSizeFlags(bool width)
{
  if (sizeFlagsDirty)
  {
    widthSizeFlags = doGetSizeFlags(true);
    heightSizeFlags = doGetSizeFlags(false);
    sizeFlagsDirty = false;
  }

  return width ? widthSizeFlags : heightSizeFlags;
}

LayoutTreeNode* LayoutTree::GetParent()
{
  return parent;
}

LayoutTree* LayoutTree::Insert(LayoutPart::Pointer child, bool left,
    LayoutPartSash::Pointer sash, LayoutPart::Pointer relative)
{
  LayoutTree relativeChild = find(relative);
  LayoutTreeNode node = new LayoutTreeNode(sash);
  if (relativeChild == null)
  {
    //Did not find the relative part. Insert beside the root.
    node.setChild(left, child);
    node.setChild(!left, this);
    return node;
  }
  else
  {
    LayoutTreeNode oldParent = relativeChild.getParent();
    node.setChild(left, child);
    node.setChild(!left, relativeChild);
    if (oldParent == null)
    {
      //It was the root. Return a new root.
      return node;
    }
    oldParent.replaceChild(relativeChild, node);
    return this;
  }
}

bool LayoutTree::IsCompressible()
{
  //Added for bug 19524
  return part.isCompressible();
}

bool LayoutTree::IsVisible()
{
  return part.Cast<PartPlaceholder>().IsNull();
}

void LayoutTree::RecomputeRatio()
{
}

LayoutTree* LayoutTree::Remove(LayoutPart::Pointer child)
{
  LayoutTree tree = find(child);
  if (tree == null)
  {
    return this;
  }
  LayoutTreeNode oldParent = tree.getParent();
  if (oldParent == null)
  {
    //It was the root and the only child of this tree
    return null;
  }
  if (oldParent.getParent() == null)
  {
    return oldParent.remove(tree);
  }

  oldParent.remove(tree);
  return this;
}

void LayoutTree::SetBounds(const Rectangle& bounds)
{
  if (!bounds.equals(currentBounds) || forceLayout)
  {
    currentBounds = Geometry.copy(bounds);

    doSetBounds(currentBounds);
    forceLayout = false;
  }
}

void LayoutTree::DoSetBounds(const Rectangle& bounds)
{
  part.setBounds(bounds);
}

void LayoutTree::SetParent(LayoutTreeNode* parent)
{
  this.parent = parent;
}

void LayoutTree::SetPart(LayoutPart::Pointer part)
{
  this.part = part;
  flushCache();
}

std::string LayoutTree::ToString()
{
  return "(" + part.toString() + ")";//$NON-NLS-2$//$NON-NLS-1$
}

void LayoutTree::CreateControl(void* parent)
{
}

void LayoutTree::DescribeLayout(std::string& buf)
{
  part.describeLayout(buf);
}

bool LayoutTree::HasSizeFlag(bool width, int flag)
{
  return (getSizeFlags(width) & flag) != 0;
}

}
