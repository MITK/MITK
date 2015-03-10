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

#include "berryLayoutTree.h"

#include "berryLayoutTreeNode.h"
#include "berryLayoutPartSash.h"

#include "berryConstants.h"
#include "berryLog.h"

namespace berry
{

int LayoutTree::minCacheHits = 0;
int LayoutTree::minCacheMisses = 0;
int LayoutTree::maxCacheHits = 0;
int LayoutTree::maxCacheMisses = 0;

LayoutTree::LayoutTree(LayoutPart::Pointer part)
 : parent(0),
 cachedMinimumWidthHint(Constants::DEFAULT),
 cachedMinimumWidth(Constants::DEFAULT),
 cachedMinimumHeightHint(Constants::DEFAULT),
 cachedMinimumHeight(Constants::DEFAULT),
 cachedMaximumWidthHint(Constants::DEFAULT),
 cachedMaximumWidth(Constants::DEFAULT),
 cachedMaximumHeightHint(Constants::DEFAULT),
 cachedMaximumHeight(Constants::DEFAULT),

 forceLayout(true),
 sizeFlagsDirty(true),
 widthSizeFlags(0),
 heightSizeFlags(0)
{
  this->part = part;
}

LayoutPart::Pointer LayoutTree::ComputeRelation(
    QList<PartSashContainer::RelationshipInfo>&  /*relations*/)
{
  return part;
}

LayoutPart::Pointer LayoutTree::FindPart(const QPoint&  /*toFind*/)
{
  return part;
}

void LayoutTree::DisposeSashes()
{
}

LayoutTree::Pointer LayoutTree::Find(LayoutPart::Pointer child)
{
  if (part != child)
  {
    return LayoutTree::Pointer(0);
  }
  return LayoutTree::Pointer(this);
}

void LayoutTree::FindSashes(PartPane::Sashes sashes)
{
  if (this->GetParent() == 0)
  {
    return;
  }
  this->GetParent()->FindSashes(LayoutTree::Pointer(this), sashes);
}

LayoutPart::Pointer LayoutTree::FindBottomRight()
{
  return part;
}

LayoutTreeNode::Pointer LayoutTree::FindSash(LayoutPartSash::Pointer  /*sash*/)
{
  return LayoutTreeNode::Pointer(0);
}

QRect LayoutTree::GetBounds()
{
  return currentBounds;
}

int LayoutTree::Subtract(int a, int b)
{
  poco_assert(b >= 0 && b < INF);

  return Add(a, -b);
}

int LayoutTree::Add(int a, int b)
{
  if (a == INF || b == INF)
  {
    return INF;
  }

  return a + b;
}

void LayoutTree::AssertValidSize(int toCheck)
{
  poco_assert(toCheck >= 0 && (toCheck == INF || toCheck < INF / 2));
}

int LayoutTree::ComputePreferredSize(bool width, int availableParallel,
    int availablePerpendicular, int preferredParallel)
{
  this->AssertValidSize(availableParallel);
  this->AssertValidSize(availablePerpendicular);
  this->AssertValidSize(preferredParallel);

  if (!this->IsVisible())
  {
    return 0;
  }

  if (availableParallel == 0)
  {
    return 0;
  }

  if (preferredParallel == 0)
  {
    return std::min<int>(availableParallel, this->ComputeMinimumSize(width,
        availablePerpendicular));
  }
  else if (preferredParallel == INF && availableParallel == INF)
  {
    return this->ComputeMaximumSize(width, availablePerpendicular);
  }

  // Optimization: if this subtree doesn't have any size preferences beyond its minimum and maximum
  // size, simply return the preferred size
  if (!this->HasSizeFlag(width, Constants::FILL))
  {
    return preferredParallel;
  }

  int result = this->DoComputePreferredSize(width, availableParallel,
      availablePerpendicular, preferredParallel);

  return result;
}

int LayoutTree::DoGetSizeFlags(bool width)
{
  return part->GetSizeFlags(width);
}

int LayoutTree::DoComputePreferredSize(bool width, int availableParallel,
    int availablePerpendicular, int preferredParallel)
{
  int result = std::min<int>(availableParallel, part->ComputePreferredSize(width,
      availableParallel, availablePerpendicular, preferredParallel));

  this->AssertValidSize(result);
  return result;
}

int LayoutTree::ComputeMinimumSize(bool width, int availablePerpendicular)
{
  this->AssertValidSize(availablePerpendicular);

  // Optimization: if this subtree has no minimum size, then always return 0 as its
  // minimum size.
  if (!this->HasSizeFlag(width, Constants::MIN))
  {
    return 0;
  }

  // If this subtree doesn't contain any wrapping controls (ie: they don't care
  // about their perpendicular size) then force the perpendicular
  // size to be INF. This ensures that we will get a cache hit
  // every time for non-wrapping controls.
  if (!this->HasSizeFlag(width, Constants::WRAP))
  {
    availablePerpendicular = INF;
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

    int result = this->DoComputeMinimumSize(width, availablePerpendicular);
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

    int result = this->DoComputeMinimumSize(width, availablePerpendicular);
    cachedMinimumHeight = result;
    cachedMinimumHeightHint = availablePerpendicular;
    return result;
  }
}
void LayoutTree::PrintCacheStatistics()
{
  BERRY_INFO << "minimize cache " << minCacheHits << " / " << (minCacheHits
      + minCacheMisses) << " hits " <<
      (minCacheHits * 100 / (minCacheHits + minCacheMisses)) << "%\n";
  BERRY_INFO  << "maximize cache " << maxCacheHits << " / " << (maxCacheHits
      + maxCacheMisses) << " hits" <<
      (maxCacheHits * 100 / (maxCacheHits + maxCacheMisses)) << "%\n";
}

int LayoutTree::DoComputeMinimumSize(bool width, int availablePerpendicular)
{
  int result = this->DoComputePreferredSize(width, INF, availablePerpendicular,
      0);
  this->AssertValidSize(result);
  return result;
}

int LayoutTree::ComputeMaximumSize(bool width, int availablePerpendicular)
{
  this->AssertValidSize(availablePerpendicular);

  // Optimization: if this subtree has no maximum size, then always return INF as its
  // maximum size.
  if (!this->HasSizeFlag(width, Constants::MAX))
  {
    return INF;
  }

  // If this subtree doesn't contain any wrapping controls (ie: they don't care
  // about their perpendicular size) then force the perpendicular
  // size to be INF. This ensures that we will get a cache hit
  // every time.
  if (!this->HasSizeFlag(width, Constants::WRAP))
  {
    availablePerpendicular = INF;
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
    int result = this->DoComputeMaximumSize(width, availablePerpendicular);
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
    int result = this->DoComputeMaximumSize(width, availablePerpendicular);
    cachedMaximumHeight = result;
    cachedMaximumHeightHint = availablePerpendicular;
    return result;
  }
}

int LayoutTree::DoComputeMaximumSize(bool width, int availablePerpendicular)
{
  return this->DoComputePreferredSize(width, INF, availablePerpendicular,
      INF);
}

void LayoutTree::FlushNode()
{

  // Clear cached sizes
  cachedMinimumWidthHint = Constants::DEFAULT;
  cachedMinimumWidth = Constants::DEFAULT;
  cachedMinimumHeightHint = Constants::DEFAULT;
  cachedMinimumHeight = Constants::DEFAULT;
  cachedMaximumWidthHint = Constants::DEFAULT;
  cachedMaximumWidth = Constants::DEFAULT;
  cachedMaximumHeightHint = Constants::DEFAULT;
  cachedMaximumHeight = Constants::DEFAULT;

  // Flags may have changed. Ensure that they are recomputed the next time around
  sizeFlagsDirty = true;

  // The next setBounds call should trigger a layout even if set to the same bounds since
  // one of the children has changed.
  forceLayout = true;
}

void LayoutTree::FlushChildren()
{
  this->FlushNode();
}

void LayoutTree::FlushCache()
{
  this->FlushNode();

  if (parent != 0)
  {
    parent->FlushCache();
  }
}

int LayoutTree::GetSizeFlags(bool width)
{
  if (sizeFlagsDirty)
  {
    widthSizeFlags = this->DoGetSizeFlags(true);
    heightSizeFlags = this->DoGetSizeFlags(false);
    sizeFlagsDirty = false;
  }

  return width ? widthSizeFlags : heightSizeFlags;
}

LayoutTreeNode* LayoutTree::GetParent() const
{
  return parent;
}

LayoutTree::Pointer LayoutTree::Insert(LayoutPart::Pointer child, bool left,
    LayoutPartSash::Pointer sash, LayoutPart::Pointer relative)
{
  LayoutTree::Pointer relativeChild = this->Find(relative);
  LayoutTreeNode::Pointer node(new LayoutTreeNode(sash));
  if (relativeChild == 0)
  {
    //Did not find the relative part. Insert beside the root.
    node->SetChild(left, child);
    node->SetChild(!left, LayoutTree::Pointer(this));
    return node;
  }
  else
  {
    LayoutTreeNode* oldParent = relativeChild->GetParent();
    node->SetChild(left, child);
    node->SetChild(!left, relativeChild);
    if (oldParent == 0)
    {
      //It was the root. Return a new root.
      return node;
    }
    oldParent->ReplaceChild(relativeChild, node);
    return LayoutTree::Pointer(this);
  }
}

bool LayoutTree::IsCompressible()
{
  //Added for bug 19524
  return part->IsCompressible();
}

bool LayoutTree::IsVisible()
{
  return !part->IsPlaceHolder();
}

void LayoutTree::RecomputeRatio()
{
}

LayoutTree::Pointer LayoutTree::Remove(LayoutPart::Pointer child)
{
  LayoutTree::Pointer tree = this->Find(child);
  if (tree == 0)
  {
    return LayoutTree::Pointer(this);
  }
  LayoutTreeNode::Pointer oldParent(tree->GetParent());
  if (oldParent == 0)
  {
    //It was the root and the only child of this tree
    return LayoutTree::Pointer(0);
  }
  if (oldParent->GetParent() == 0)
  {
    return oldParent->Remove(tree);
  }

  oldParent->Remove(tree);
  return LayoutTree::Pointer(this);
}

void LayoutTree::SetBounds(const QRect& bounds)
{
  if (!(bounds == currentBounds) || forceLayout)
  {
    currentBounds = bounds;

    this->DoSetBounds(currentBounds);
    forceLayout = false;
  }
}

void LayoutTree::DoSetBounds(const QRect& bounds)
{
  part->SetBounds(bounds);
}

void LayoutTree::SetParent(LayoutTreeNode* parent)
{
  this->parent = parent;
}

void LayoutTree::SetPart(LayoutPart::Pointer part)
{
  this->part = part;
  this->FlushCache();
}

QString LayoutTree::ToString() const
{
  return "(" + part->ToString() + ")";
}

void LayoutTree::CreateControl(QWidget*  /*parent*/)
{
}

void LayoutTree::DescribeLayout(QString& buf) const
{
  part->DescribeLayout(buf);
}

bool LayoutTree::HasSizeFlag(bool width, int flag)
{
  return (this->GetSizeFlags(width) & flag) != 0;
}

}
