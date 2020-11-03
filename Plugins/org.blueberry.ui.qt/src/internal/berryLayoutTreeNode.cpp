/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryLayoutTreeNode.h"

#include "berryConstants.h"
#include "berryIPageLayout.h"

#include <sstream>

namespace berry
{

LayoutTreeNode::ChildSizes::ChildSizes (int l, int r, bool resize)
{
  left = l;
  right = r;
  resizable = resize;
}

LayoutTreeNode::LayoutTreeNode(LayoutPartSash::Pointer sash)
 : LayoutTree(sash)
{
  children[0] = nullptr;
  children[1] = nullptr;
}

LayoutTreeNode::~LayoutTreeNode()
{

}

void LayoutTreeNode::FlushChildren()
{
  LayoutTree::FlushChildren();

  children[0]->FlushChildren();
  children[1]->FlushChildren();
}

LayoutPart::Pointer LayoutTreeNode::FindPart(const QPoint& toFind)
{
  if (!children[0]->IsVisible())
  {
    if (!children[1]->IsVisible())
    {
      return LayoutPart::Pointer(nullptr);
    }

    return children[1]->FindPart(toFind);
  }
  else
  {
    if (!children[1]->IsVisible())
    {
      return children[0]->FindPart(toFind);
    }
  }

  LayoutPartSash::Pointer sash = this->GetSash();

  QRect bounds = sash->GetBounds();

  if (sash->IsVertical())
  {
    if (toFind.x() < bounds.x() + (bounds.width() / 2))
    {
      return children[0]->FindPart(toFind);
    }
    return children[1]->FindPart(toFind);
  }
  else
  {
    if (toFind.y() < bounds.y() + (bounds.height() / 2))
    {
      return children[0]->FindPart(toFind);
    }
    return children[1]->FindPart(toFind);
  }
}

LayoutPart::Pointer LayoutTreeNode::ComputeRelation(
    QList<PartSashContainer::RelationshipInfo>& relations)
{
  PartSashContainer::RelationshipInfo r =
      PartSashContainer::RelationshipInfo();
  r.relative = children[0]->ComputeRelation(relations);
  r.part = children[1]->ComputeRelation(relations);
  r.left = this->GetSash()->GetLeft();
  r.right = this->GetSash()->GetRight();
  r.relationship = this->GetSash()->IsVertical() ? IPageLayout::RIGHT : IPageLayout::BOTTOM;
  relations.push_front(r);
  return r.relative;
}

void LayoutTreeNode::DisposeSashes()
{
  children[0]->DisposeSashes();
  children[1]->DisposeSashes();
  this->GetSash()->Dispose();
}

LayoutTree::Pointer LayoutTreeNode::Find(LayoutPart::Pointer child)
{
  LayoutTree::Pointer node = children[0]->Find(child);
  if (node != 0)
  {
    return node;
  }
  node = children[1]->Find(child);
  return node;
}

LayoutPart::Pointer LayoutTreeNode::FindBottomRight()
{
  if (children[1]->IsVisible())
  {
    return children[1]->FindBottomRight();
  }
  return children[0]->FindBottomRight();
}

LayoutTreeNode* LayoutTreeNode::FindCommonParent(LayoutPart::Pointer child1,
    LayoutPart::Pointer child2, bool foundChild1,
    bool foundChild2)
{
  if (!foundChild1)
  {
    foundChild1 = this->Find(child1) != 0;
  }
  if (!foundChild2)
  {
    foundChild2 = this->Find(child2) != 0;
  }
  if (foundChild1 && foundChild2)
  {
    return this;
  }
  if (parent == nullptr)
  {
    return nullptr;
  }
  return parent
  ->FindCommonParent(child1, child2, foundChild1, foundChild2);
}

LayoutTreeNode::Pointer LayoutTreeNode::FindSash(LayoutPartSash::Pointer sash)
{
  if (this->GetSash() == sash)
  {
    return LayoutTreeNode::Pointer(this);
  }
  LayoutTreeNode::Pointer node = children[0]->FindSash(sash);
  if (node != 0)
  {
    return node;
  }
  node = children[1]->FindSash(sash);
  if (node != 0)
  {
    return node;
  }
  return LayoutTreeNode::Pointer(nullptr);
}

void LayoutTreeNode::FindSashes(LayoutTree::Pointer child, PartPane::Sashes sashes)
{
  QWidget* sash = this->GetSash()->GetControl();
  bool leftOrTop = children[0] == child;
  if (sash != nullptr)
  {
    LayoutPartSash::Pointer partSash = this->GetSash();
    //If the child is in the left, the sash
    //is in the rigth and so on.
    if (leftOrTop)
    {
      if (partSash->IsVertical())
      {
        if (sashes.right == nullptr)
        {
          sashes.right = sash;
        }
      }
      else
      {
        if (sashes.bottom == nullptr)
        {
          sashes.bottom = sash;
        }
      }
    }
    else
    {
      if (partSash->IsVertical())
      {
        if (sashes.left == nullptr)
        {
          sashes.left = sash;
        }
      }
      else
      {
        if (sashes.top == nullptr)
        {
          sashes.top = sash;
        }
      }
    }
  }
  if (this->GetParent() != nullptr)
  {
    this->GetParent()->FindSashes(LayoutTree::Pointer(this), sashes);
  }
}

LayoutPartSash::Pointer LayoutTreeNode::GetSash() const
{
  return part.Cast<LayoutPartSash>();
}

int LayoutTreeNode::GetSashSize() const
{
  return this->GetSash()->GetSashSize();
}

bool LayoutTreeNode::IsVisible()
{
  return children[0]->IsVisible() || children[1]->IsVisible();
}

LayoutTree::Pointer LayoutTreeNode::Remove(LayoutTree::Pointer child)
{
  this->GetSash()->Dispose();
  if (parent == nullptr)
  {
    //This is the root. Return the other child to be the new root.
    if (children[0] == child)
    {
      children[1]->SetParent(nullptr);
      return children[1];
    }
    children[0]->SetParent(nullptr);
    return children[0];
  }

  LayoutTreeNode::Pointer oldParent(parent);
  if (children[0] == child)
  {
    oldParent->ReplaceChild(LayoutTree::Pointer(this), children[1]);
  }
  else
  {
    oldParent->ReplaceChild(LayoutTree::Pointer(this), children[0]);
  }
  return oldParent;
}

void LayoutTreeNode::ReplaceChild(LayoutTree::Pointer oldChild, LayoutTree::Pointer newChild)
{
  if (children[0] == oldChild)
  {
    children[0] = newChild;
  }
  else if (children[1] == oldChild)
  {
    children[1] = newChild;
  }
  newChild->SetParent(this);
  if (!children[0]->IsVisible() || !children[0]->IsVisible())
  {
    this->GetSash()->Dispose();
  }

  this->FlushCache();
}

bool LayoutTreeNode::SameDirection(bool isVertical, LayoutTreeNode::Pointer subTree)
{
  bool treeVertical = this->GetSash()->IsVertical();
  if (treeVertical != isVertical)
  {
    return false;
  }
  while (subTree != 0)
  {
    if (this == subTree.GetPointer())
    {
      return true;
    }
    if (subTree->children[0]->IsVisible() && subTree->children[1]->IsVisible())
    {
      if (subTree->GetSash()->IsVertical() != isVertical)
      {
        return false;
      }
    }
    subTree = subTree->GetParent();
  }
  return true;
}

int LayoutTreeNode::DoComputePreferredSize(bool width, int availableParallel,
    int availablePerpendicular, int preferredParallel)
{
  this->AssertValidSize(availablePerpendicular);
  this->AssertValidSize(availableParallel);
  this->AssertValidSize(preferredParallel);

  // If one child is invisible, defer to the other child
  if (!children[0]->IsVisible())
  {
    return children[1]->ComputePreferredSize(width, availableParallel,
        availablePerpendicular, preferredParallel);
  }

  if (!children[1]->IsVisible())
  {
    return children[0]->ComputePreferredSize(width, availableParallel,
        availablePerpendicular, preferredParallel);
  }

  if (availableParallel == 0)
  {
    return 0;
  }

  // If computing the dimension perpendicular to our sash
  if (width == this->GetSash()->IsVertical())
  {
    // Compute the child sizes
    ChildSizes sizes = this->ComputeChildSizes(availableParallel,
        availablePerpendicular,
        GetSash()->GetLeft(), GetSash()->GetRight(), preferredParallel);

    // Return the sum of the child sizes plus the sash size
    return this->Add(sizes.left, this->Add(sizes.right, this->GetSashSize()));
  }
  else
  {
    // Computing the dimension parallel to the sash. We will compute and return the preferred size
    // of whichever child is closest to the ideal size.

    // First compute the dimension of the child sizes perpendicular to the sash
    ChildSizes sizes = this->ComputeChildSizes(availablePerpendicular, availableParallel,
        GetSash()->GetLeft(), GetSash()->GetRight(), availablePerpendicular);

    // Use this information to compute the dimension of the child sizes parallel to the sash.
    // Return the preferred size of whichever child is largest
    int leftSize = children[0]->ComputePreferredSize(width, availableParallel,
        sizes.left, preferredParallel);

    // Compute the preferred size of the right child
    int rightSize = children[1]->ComputePreferredSize(width, availableParallel,
        sizes.right, preferredParallel);

    // Return leftSize or rightSize: whichever one is largest
    int result = rightSize;
    if (leftSize > rightSize)
    {
      result = leftSize;
    }

    this->AssertValidSize(result);

    return result;
  }
}

LayoutTreeNode::ChildSizes LayoutTreeNode::ComputeChildSizes(int width, int height, int left,
    int right, int preferredWidth)
{
  poco_assert(children[0]->IsVisible());
  poco_assert(children[1]->IsVisible());
  this->AssertValidSize(width);
  this->AssertValidSize(height);
  this->AssertValidSize(preferredWidth);
  poco_assert(left >= 0);
  poco_assert(right >= 0);
  poco_assert(preferredWidth >= 0);
  poco_assert(preferredWidth <= width);
  bool vertical = this->GetSash()->IsVertical();

  if (width <= this->GetSashSize())
  {
    return ChildSizes(0,0, false);
  }

  if (width == INF)
  {
    if (preferredWidth == INF)
    {
      return ChildSizes(children[0]->ComputeMaximumSize(vertical, height),
          children[1]->ComputeMaximumSize(vertical, height), false);
    }

    if (preferredWidth == 0)
    {
      return ChildSizes(children[0]->ComputeMinimumSize(vertical, height),
          children[1]->ComputeMinimumSize(vertical, height), false);
    }
  }

  int total = left + right;

  // Use all-or-none weighting
  double wLeft = left, wRight = right;
  switch (this->GetCompressionBias())
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
  preferredWidth = std::max<int>(0, this->Subtract(preferredWidth, this->GetSashSize()));
  width = std::max<int>(0, this->Subtract(width, this->GetSashSize()));

  int redistribute = this->Subtract(preferredWidth, total);

  // Compute the minimum and maximum sizes for each child
  int leftMinimum = children[0]->ComputeMinimumSize(vertical, height);
  int rightMinimum = children[1]->ComputeMinimumSize(vertical, height);
  int leftMaximum = children[0]->ComputeMaximumSize(vertical, height);
  int rightMaximum = children[1]->ComputeMaximumSize(vertical, height);

  int idealLeft = 0;
  int idealRight = 0;
  if (PartSashContainer::leftToRight)
  {
    // Keep track of the available space for each child, given the minimum size of the other child
    int leftAvailable = std::min<int>(leftMaximum, std::max<int>(0, this->Subtract(width,
        rightMinimum)));
    int rightAvailable = std::min<int>(rightMaximum, std::max<int>(0, this->Subtract(width,
        leftMinimum)));

    // Figure out the ideal size of the left child
    idealLeft = std::max<int>(leftMinimum, std::min<int>(preferredWidth, left
        + (int)(redistribute * wLeft / wTotal)));

    // If the right child can't use all its available space, let the left child fill it in
    idealLeft = std::max<int>(idealLeft, preferredWidth - rightAvailable);
    // Ensure the left child doesn't get larger than its available space
    idealLeft = std::min<int>(idealLeft, leftAvailable);

    // Check if the left child would prefer to be a different size
    idealLeft = children[0]->ComputePreferredSize(vertical, leftAvailable, height,
        idealLeft);

    // Ensure that the left child is larger than its minimum size
    idealLeft = std::max<int>(idealLeft, leftMinimum);
    idealLeft = std::min<int>(idealLeft, leftAvailable);

    // Compute the right child width
    idealRight = std::max<int>(rightMinimum, preferredWidth - idealLeft);

    rightAvailable = std::max<int>(0, std::min<int>(rightAvailable, this->Subtract(width,
        idealLeft)));
    idealRight = std::min<int>(idealRight, rightAvailable);
    idealRight = children[1]->ComputePreferredSize(vertical, rightAvailable,
        height, idealRight);
    idealRight = std::max<int>(idealRight, rightMinimum);
  }
  else
  {
    // Keep track of the available space for each child, given the minimum size of the other child
    int rightAvailable = std::min<int>(rightMaximum, std::max<int>(0, this->Subtract(width,
        leftMinimum)));
    int leftAvailable = std::min<int>(leftMaximum, std::max<int>(0, this->Subtract(width,
        rightMinimum)));

    // Figure out the ideal size of the right child
    idealRight = std::max<int>(rightMinimum, std::min<int>(preferredWidth, right
        + (int)(redistribute * wRight / wTotal)));

    // If the left child can't use all its available space, let the right child fill it in
    idealRight = std::max<int>(idealRight, preferredWidth - leftAvailable);
    // Ensure the right child doesn't get larger than its available space
    idealRight = std::min<int>(idealRight, rightAvailable);

    // Check if the right child would prefer to be a different size
    idealRight = children[1]->ComputePreferredSize(vertical, rightAvailable, height,
        idealRight);

    // Ensure that the right child is larger than its minimum size
    idealRight = std::max<int>(idealRight, rightMinimum);
    idealRight = std::min<int>(idealRight, rightAvailable);

    // Compute the left child width
    idealLeft = std::max<int>(leftMinimum, preferredWidth - idealRight);

    leftAvailable = std::max<int>(0, std::min<int>(leftAvailable, this->Subtract(width,
        idealRight)));
    idealLeft = std::min<int>(idealLeft, leftAvailable);
    idealLeft = children[0]->ComputePreferredSize(vertical, leftAvailable,
        height, idealLeft);
    idealLeft = std::max<int>(idealLeft, leftMinimum);
  }

  return ChildSizes(idealLeft, idealRight, leftMaximum> leftMinimum
      && rightMaximum> rightMinimum
      && leftMinimum + rightMinimum < width);
}

int LayoutTreeNode::DoGetSizeFlags(bool width)
{
  if (!children[0]->IsVisible())
  {
    return children[1]->GetSizeFlags(width);
  }

  if (!children[1]->IsVisible())
  {
    return children[0]->GetSizeFlags(width);
  }

  int leftFlags = children[0]->GetSizeFlags(width);
  int rightFlags = children[1]->GetSizeFlags(width);

  return ((leftFlags | rightFlags) & ~Constants::MAX) | (leftFlags & rightFlags
      & Constants::MAX);
}

void LayoutTreeNode::DoSetBounds(const QRect& b)
{
  if (!children[0]->IsVisible())
  {
    children[1]->SetBounds(b);
    this->GetSash()->SetVisible(false);
    return;
  }
  if (!children[1]->IsVisible())
  {
    children[0]->SetBounds(b);
    this->GetSash()->SetVisible(false);
    return;
  }

  QRect bounds = b;

  bool vertical = this->GetSash()->IsVertical();

  // If this is a horizontal sash, flip coordinate systems so
  // that we can eliminate special cases
  if (!vertical)
  {
    bounds = FlipRect(bounds);
  }

  ChildSizes childSizes = this->ComputeChildSizes(bounds.width(), bounds.height(),
      this->GetSash()->GetLeft(), this->GetSash()->GetRight(), bounds.width());

  this->GetSash()->SetVisible(true);
  this->GetSash()->SetEnabled(childSizes.resizable);

  QRect leftBounds = QRect(bounds.x(), bounds.y(), childSizes.left, bounds.height());
  QRect sashBounds = QRect(leftBounds.x() + leftBounds.width(), bounds.y(), this->GetSashSize(), bounds.height());
  QRect rightBounds = QRect(sashBounds.x() + sashBounds.width(), bounds.y(),
                            childSizes.right, bounds.height());

  if (!vertical)
  {
    leftBounds = FlipRect(leftBounds);
    sashBounds = FlipRect(sashBounds);
    rightBounds = FlipRect(rightBounds);
  }

  this->GetSash()->SetBounds(sashBounds);
  children[0]->SetBounds(leftBounds);
  children[1]->SetBounds(rightBounds);
}

void LayoutTreeNode::CreateControl(QWidget* parent)
{
  children[0]->CreateControl(parent);
  children[1]->CreateControl(parent);
  this->GetSash()->CreateControl(parent);

  LayoutTree::CreateControl(parent);
}

bool LayoutTreeNode::IsCompressible()
{
  return children[0]->IsCompressible() || children[1]->IsCompressible();
}

int LayoutTreeNode::GetCompressionBias()
{
  bool left = children[0]->IsCompressible();
  bool right = children[1]->IsCompressible();
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

bool LayoutTreeNode::IsLeftChild(LayoutTree::ConstPointer toTest)
{
  return children[0] == toTest;
}

LayoutTree::Pointer LayoutTreeNode::GetChild(bool left)
{
  int index = left ? 0 : 1;
  return (children[index]);
}

void LayoutTreeNode::SetChild(bool left, LayoutPart::Pointer part)
{
  LayoutTree::Pointer child(new LayoutTree(part));
  this->SetChild(left, child);
  this->FlushCache();
}

void LayoutTreeNode::SetChild(bool left, LayoutTree::Pointer child)
{
  int index = left ? 0 : 1;
  children[index] = child;
  child->SetParent(this);
  this->FlushCache();
}

QString LayoutTreeNode::ToString() const
{
  QString str;
  QTextStream s(&str);
  s << "<null>\n";
  if (part->GetControl() != nullptr)
  {
    s << "<@" << part->GetControl() << ">\n";
  }
  QString str2;
  QTextStream result(&str2);
  result << "[";
  if (children[0]->GetParent() != this)
  {
    result << str2 << "{" << children[0] << "}" << str;
  }
  else
  {
    result << str2 << children[0] << str;
  }

  if (children[1]->GetParent() != this)
  {
    result << str2 << "{" << children[1] << "}]";
  }
  else
  {
    result << str2 << children[1] << "]";
  }
  return str2;
}

//void LayoutTreeNode::UpdateSashes(QWidget* parent) {
//        if (parent == 0)
//            return;
//        children[0]->UpdateSashes(parent);
//        children[1]->UpdateSashes(parent);
//        if (children[0]->IsVisible() && children[1]->IsVisible())
//            this->GetSash()->CreateControl(parent);
//        else
//            this->GetSash()->Dispose();
//    }

void LayoutTreeNode::DescribeLayout(QString& buf) const
{
  if (!(children[0]->IsVisible()))
  {
    if (!children[1]->IsVisible())
    {
      return;
    }

    children[1]->DescribeLayout(buf);
    return;
  }

  if (!children[1]->IsVisible())
  {
    children[0]->DescribeLayout(buf);
    return;
  }

  buf.append("("); //$NON-NLS-1$
  children[0]->DescribeLayout(buf);

  buf.append(this->GetSash()->IsVertical() ? "|" : "-");

  children[1]->DescribeLayout(buf);
  buf.append(")");
}

QRect LayoutTreeNode::FlipRect(const QRect& rect)
{
  return QRect(rect.y(), rect.x(), rect.height(), rect.width());
}

}
