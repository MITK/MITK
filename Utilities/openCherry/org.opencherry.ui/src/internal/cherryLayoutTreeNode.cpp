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

#include "cherryLayoutTreeNode.h"

namespace cherry
{

LayoutTreeNode::LayoutTreeNode(LayoutPartSash::Pointer sash)
{
  super(sash);
}

void LayoutTreeNode::FlushChildren()
{
  super.flushChildren();

  children[0].flushChildren();
  children[1].flushChildren();
}

LayoutPart::Pointer LayoutTreeNode::FindPart(int x, int y)
{
  if (!children[0].isVisible())
  {
    if (!children[1].isVisible())
    {
      return null;
    }

    return children[1].findPart(toFind);
  }
  else
  {
    if (!children[1].isVisible())
    {
      return children[0].findPart(toFind);
    }
  }

  LayoutPartSash sash = getSash();

  Rectangle bounds = sash.getBounds();

  if (sash.isVertical())
  {
    if (toFind.x < bounds.x + (bounds.width / 2))
    {
      return children[0].findPart(toFind);
    }
    return children[1].findPart(toFind);
  }
  else
  {
    if (toFind.y < bounds.y + (bounds.height / 2))
    {
      return children[0].findPart(toFind);
    }
    return children[1].findPart(toFind);
  }
}

LayoutPart::Pointer LayoutTreeNode::ComputeRelation(
    std::list<PartSashContainer::RelationshipInfo>& relations)
{
  PartSashContainer::RelationshipInfo r =
      new PartSashContainer::RelationshipInfo();
  r.relative = children[0].computeRelation(relations);
  r.part = children[1].computeRelation(relations);
  r.left = getSash().getLeft();
  r.right = getSash().getRight();
  r.relationship = getSash().isVertical() ? IPageLayout.RIGHT : IPageLayout.BOTTOM;
  relations.add(0, r);
  return r.relative;
}

void LayoutTreeNode::DisposeSashes()
{
  children[0].disposeSashes();
  children[1].disposeSashes();
  getSash().dispose();
}

LayoutTree* LayoutTreeNode::Find(LayoutPart::Pointer child)
{
  LayoutTree node = children[0].find(child);
  if (node != null)
  {
    return node;
  }
  node = children[1].find(child);
  return node;
}

LayoutPart::Pointer LayoutTreeNode::FindBottomRight()
{
  if (children[1].isVisible())
  {
    return children[1].findBottomRight();
  }
  return children[0].findBottomRight();
}

LayoutTreeNode* LayoutTreeNode::FindCommonParent(LayoutPart::Pointer child1,
    LayoutPart::Pointer child2, bool foundChild1 = false,
    bool foundChild2 = false)
{
  if (!foundChild1)
  {
    foundChild1 = find(child1) != null;
  }
  if (!foundChild2)
  {
    foundChild2 = find(child2) != null;
  }
  if (foundChild1 && foundChild2)
  {
    return this;
  }
  if (parent == null)
  {
    return null;
  }
  return parent
  .findCommonParent(child1, child2, foundChild1, foundChild2);
}

LayoutTreeNode* LayoutTreeNode::FindSash(LayoutPartSash::Pointer sash)
{
  if (this.getSash() == sash)
  {
    return this;
  }
  LayoutTreeNode node = children[0].findSash(sash);
  if (node != null)
  {
    return node;
  }
  node = children[1].findSash(sash);
  if (node != null)
  {
    return node;
  }
  return null;
}

void LayoutTreeNode::FindSashes(LayoutTree* child, PartPane::Sashes sashes)
{
  Sash sash = (Sash) getSash().getControl();
  boolean leftOrTop = children[0] == child;
  if (sash != null)
  {
    LayoutPartSash partSash = getSash();
    //If the child is in the left, the sash 
    //is in the rigth and so on.
    if (leftOrTop)
    {
      if (partSash.isVertical())
      {
        if (sashes.right == null)
        {
          sashes.right = sash;
        }
      }
      else
      {
        if (sashes.bottom == null)
        {
          sashes.bottom = sash;
        }
      }
    }
    else
    {
      if (partSash.isVertical())
      {
        if (sashes.left == null)
        {
          sashes.left = sash;
        }
      }
      else
      {
        if (sashes.top == null)
        {
          sashes.top = sash;
        }
      }
    }
  }
  if (getParent() != null)
  {
    getParent().findSashes(this, sashes);
  }
}

LayoutPartSash::Poiter LayoutTreeNode::GetSash()
{
  return (LayoutPartSash) part;
}

int LayoutTreeNode::GetSashSize()
{
  return getSash().getSashSize();
}

bool LayoutTreeNode::IsVisible()
{
  return children[0].isVisible() || children[1].isVisible();
}

LayoutTree* LayoutTreeNode::Remove(LayoutTree* child)
{
  getSash().dispose();
  if (parent == null)
  {
    //This is the root. Return the other child to be the new root.
    if (children[0] == child)
    {
      children[1].setParent(null);
      return children[1];
    }
    children[0].setParent(null);
    return children[0];
  }

  LayoutTreeNode oldParent = parent;
  if (children[0] == child)
  {
    oldParent.replaceChild(this, children[1]);
  }
  else
  {
    oldParent.replaceChild(this, children[0]);
  }
  return oldParent;
}

void LayoutTreeNode::ReplaceChild(LayoutTree* oldChild, LayoutTree* newChild)
{
  if (children[0] == oldChild)
  {
    children[0] = newChild;
  }
  else if (children[1] == oldChild)
  {
    children[1] = newChild;
  }
  newChild.setParent(this);
  if (!children[0].isVisible() || !children[0].isVisible())
  {
    getSash().dispose();
  }

  flushCache();
}

bool LayoutTreeNode::SameDirection(bool isVertical, LayoutTreeNode* subTree)
{
  boolean treeVertical = getSash().isVertical();
  if (treeVertical != isVertical)
  {
    return false;
  }
  while (subTree != null)
  {
    if (this == subTree)
    {
      return true;
    }
    if (subTree.children[0].isVisible() && subTree.children[1].isVisible())
    {
      if (subTree.getSash().isVertical() != isVertical)
      {
        return false;
      }
    }
    subTree = subTree.getParent();
  }
  return true;
}

int LayoutTreeNode::DoComputePreferredSize(bool width, int availableParallel,
    int availablePerpendicular, int preferredParallel)
{
  assertValidSize(availablePerpendicular);
  assertValidSize(availableParallel);
  assertValidSize(preferredParallel);

  // If one child is invisible, defer to the other child
  if (!children[0].isVisible())
  {
    return children[1].computePreferredSize(width, availableParallel,
        availablePerpendicular, preferredParallel);
  }

  if (!children[1].isVisible())
  {
    return children[0].computePreferredSize(width, availableParallel,
        availablePerpendicular, preferredParallel);
  }

  if (availableParallel == 0)
  {
    return 0;
  }

  // If computing the dimension perpendicular to our sash
  if (width == getSash().isVertical())
  {
    // Compute the child sizes
    ChildSizes sizes = computeChildSizes(availableParallel,
        availablePerpendicular, getSash().getLeft(), getSash().getRight(), preferredParallel);

    // Return the sum of the child sizes plus the sash size
    return add(sizes.left, add(sizes.right, getSashSize()));
  }
  else
  {
    // Computing the dimension parallel to the sash. We will compute and return the preferred size
    // of whichever child is closest to the ideal size.

    ChildSizes sizes;
    // First compute the dimension of the child sizes perpendicular to the sash
    sizes = computeChildSizes(availablePerpendicular, availableParallel,
        getSash().getLeft(), getSash().getRight(), availablePerpendicular);

    // Use this information to compute the dimension of the child sizes parallel to the sash.
    // Return the preferred size of whichever child is largest
    int leftSize = children[0].computePreferredSize(width, availableParallel,
        sizes.left, preferredParallel);

    // Compute the preferred size of the right child
    int rightSize = children[1].computePreferredSize(width, availableParallel,
        sizes.right, preferredParallel);

    // Return leftSize or rightSize: whichever one is largest
    int result = rightSize;
    if (leftSize > rightSize)
    {
      result = leftSize;
    }

    assertValidSize(result);

    return result;
  }
}

ChildSizes LayoutTreeNode::ComputeChildSizes(int width, int height, int left,
    int right, int preferredWidth)
{
  Assert.isTrue(children[0].isVisible());
  Assert.isTrue(children[1].isVisible());
  assertValidSize(width);
  assertValidSize(height);
  assertValidSize(preferredWidth);
  Assert.isTrue(left >= 0);
  Assert.isTrue(right >= 0);
  Assert.isTrue(preferredWidth >= 0);
  Assert.isTrue(preferredWidth <= width);
  boolean vertical = getSash().isVertical();

  if (width <= getSashSize())
  {
    return new ChildSizes(0,0, false);
  }

  if (width == INFINITE)
  {
    if (preferredWidth == INFINITE)
    {
      return new ChildSizes(children[0].computeMaximumSize(vertical, height),
          children[1].computeMaximumSize(vertical, height), false);
    }

    if (preferredWidth == 0)
    {
      return new ChildSizes(children[0].computeMinimumSize(vertical, height),
          children[1].computeMinimumSize(vertical, height), false);
    }
  }

  int total = left + right;

  // Use all-or-none weighting
  double wLeft = left, wRight = right;
  switch (getCompressionBias())
  {
  case -1:
    wLeft = 0.0;
    break;
  case 1:
    wRight = 0.0;
    break;
  default:
    break;
  }
  double wTotal = wLeft + wRight;

  // Subtract the SASH_WIDTH from preferredWidth and width. From here on, we'll deal with the
  // width available to the controls and neglect the space used by the sash.
  preferredWidth = Math.max(0, subtract(preferredWidth, getSashSize()));
  width = Math.max(0, subtract(width, getSashSize()));

  int redistribute = subtract(preferredWidth, total);

  // Compute the minimum and maximum sizes for each child
  int leftMinimum = children[0].computeMinimumSize(vertical, height);
  int rightMinimum = children[1].computeMinimumSize(vertical, height);
  int leftMaximum = children[0].computeMaximumSize(vertical, height);
  int rightMaximum = children[1].computeMaximumSize(vertical, height);

  // Keep track of the available space for each child, given the minimum size of the other child
  int leftAvailable = Math.min(leftMaximum, Math.max(0, subtract(width,
      rightMinimum)));
  int rightAvailable = Math.min(rightMaximum, Math.max(0, subtract(width,
      leftMinimum)));

  // Figure out the ideal size of the left child
  int idealLeft = Math.max(leftMinimum, Math.min(preferredWidth, left
      + (int) Math.round(redistribute * wLeft / wTotal)));

  // If the right child can't use all its available space, let the left child fill it in
  idealLeft = Math.max(idealLeft, preferredWidth - rightAvailable);
  // Ensure the left child doesn't get larger than its available space
  idealLeft = Math.min(idealLeft, leftAvailable);

  // Check if the left child would prefer to be a different size 
  idealLeft = children[0].computePreferredSize(vertical, leftAvailable, height,
      idealLeft);

  // Ensure that the left child is larger than its minimum size
  idealLeft = Math.max(idealLeft, leftMinimum);
  idealLeft = Math.min(idealLeft, leftAvailable);

  // Compute the right child width
  int idealRight = Math.max(rightMinimum, preferredWidth - idealLeft);

  rightAvailable = Math.max(0, Math.min(rightAvailable, subtract(width,
      idealLeft)));
  idealRight = Math.min(idealRight, rightAvailable);
  idealRight = children[1].computePreferredSize(vertical, rightAvailable,
      height, idealRight);
  idealRight = Math.max(idealRight, rightMinimum);

  return new ChildSizes(idealLeft, idealRight, leftMaximum> leftMinimum
      && rightMaximum> rightMinimum
      && leftMinimum + rightMinimum < width);
}

int LayoutTreeNode::DoGetSizeFlags(bool width)
{
  if (!children[0].isVisible())
  {
    return children[1].getSizeFlags(width);
  }

  if (!children[1].isVisible())
  {
    return children[0].getSizeFlags(width);
  }

  int leftFlags = children[0].getSizeFlags(width);
  int rightFlags = children[1].getSizeFlags(width);

  return ((leftFlags | rightFlags) & ~SWT.MAX) | (leftFlags & rightFlags
      & SWT.MAX);
}

void LayoutTreeNode::DoSetBounds(const Rectangle& bounds)
{
  if (!children[0].isVisible())
  {
    children[1].setBounds(bounds);
    getSash().setVisible(false);
    return;
  }
  if (!children[1].isVisible())
  {
    children[0].setBounds(bounds);
    getSash().setVisible(false);
    return;
  }

  bounds = Geometry.copy(bounds);

  boolean vertical = getSash().isVertical();

  // If this is a horizontal sash, flip coordinate systems so 
  // that we can eliminate special cases
  if (!vertical)
  {
    Geometry.flipXY(bounds);
  }

  ChildSizes childSizes = computeChildSizes(bounds.width, bounds.height,
      getSash().getLeft(), getSash().getRight(), bounds.width);

  getSash().setVisible(true);
  getSash().setEnabled(childSizes.resizable);

  Rectangle leftBounds = new Rectangle(bounds.x, bounds.y, childSizes.left, bounds.height);
  Rectangle sashBounds = new Rectangle(leftBounds.x + leftBounds.width, bounds.y, this.getSashSize(), bounds.height);
  Rectangle
      rightBounds =
          new Rectangle(sashBounds.x + sashBounds.width, bounds.y, childSizes.right, bounds.height);

  if (!vertical)
  {
    Geometry.flipXY(leftBounds);
    Geometry.flipXY(sashBounds);
    Geometry.flipXY(rightBounds);
  }

  getSash().setBounds(sashBounds);
  children[0].setBounds(leftBounds);
  children[1].setBounds(rightBounds);
}

void LayoutTreeNode::CreateControl(void* parent)
{
  children[0].createControl(parent);
  children[1].createControl(parent);
  getSash().createControl(parent);

  super.createControl(parent);
}

bool LayoutTreeNode::IsCompressible()
{
  return children[0].isCompressible() || children[1].isCompressible();
}

int LayoutTreeNode::GetCompressionBias()
{
  boolean left = children[0].isCompressible();
  boolean right = children[1].isCompressible();
  if (left == right)
  {
    return 0;
  }
  if (right)
  {
    return -1;
  }
  return 1;
}

bool LayoutTreeNode::IsLeftChild(LayoutTree* toTest)
{
  return children[0] == toTest;
}

LayoutTree* LayoutTreeNode::GetChild(bool left)
{
  int index = left ? 0 : 1;
  return (children[index]);
}

void LayoutTreeNode::SetChild(bool left, LayoutPart::Pointer part)
{
  LayoutTree child = new LayoutTree(part);
  setChild(left, child);
  flushCache();
}

void LayoutTreeNode::SetChild(bool left, LayoutTree* child)
{
  int index = left ? 0 : 1;
  children[index] = child;
  child.setParent(this);
  flushCache();
}

std::string LayoutTreeNode::ToString()
{
  String s = "<null>\n";//$NON-NLS-1$
  if (part.getControl() != null)
  {
    s = "<@" + part.getControl().hashCode() + ">\n";//$NON-NLS-2$//$NON-NLS-1$
  }
  String result = "["; //$NON-NLS-1$
  if (children[0].getParent() != this)
  {
    result = result + "{" + children[0] + "}" + s;//$NON-NLS-2$//$NON-NLS-1$
  }
  else
  {
    result = result + children[0] + s;
  }

  if (children[1].getParent() != this)
  {
    result = result + "{" + children[1] + "}]";//$NON-NLS-2$//$NON-NLS-1$
  }
  else
  {
    result = result + children[1] + "]";//$NON-NLS-1$
  }
  return result;
}

void LayoutTreeNode::DescribeLayout(std::string& buf)
{
  if (!(children[0].isVisible()))
  {
    if (!children[1].isVisible())
    {
      return;
    }

    children[1].describeLayout(buf);
    return;
  }

  if (!children[1].isVisible())
  {
    children[0].describeLayout(buf);
    return;
  }

  buf.append("("); //$NON-NLS-1$
  children[0].describeLayout(buf);

  buf.append(getSash().isVertical() ? "|" : "-"); //$NON-NLS-1$ //$NON-NLS-2$

  children[1].describeLayout(buf);
  buf.append(")"); //$NON-NLS-1$
}

}
