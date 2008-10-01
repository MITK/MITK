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

#include "cherryPartSashContainer.h"

#include "cherryLayoutTree.h"
#include "cherryLayoutTreeNode.h"
#include "cherryPartStack.h"
#include "cherryPageLayout.h"
#include "cherryPerspective.h"
#include "cherryPerspectiveHelper.h"
#include "../cherryPartPane.h"

#include "../tweaklets/cherryGuiWidgetsTweaklet.h"
#include "../cherryConstants.h"

namespace cherry
{

PartSashContainer::ControlListener::ControlListener(
    PartSashContainer* container) :
  partSashContainer(container)
{

}

void PartSashContainer::ControlListener::ControlResized(
    GuiTk::ControlEvent::Pointer /*e*/)
{
  partSashContainer->ResizeSashes();
}

PartSashContainer::PartSashContainer(const std::string& id,
    WorkbenchPage::Pointer _page, void* _parentWidget) :
  LayoutPart(id), parentWidget(_parentWidget), parent(0),
  page(_page), active(false), layoutDirty(false)
{
  resizeListener = new ControlListener(this);
}

std::vector<PartPane::Pointer> PartSashContainer::GetVisibleParts(
    LayoutPart::Pointer pane)
{
  std::vector<PartPane::Pointer> parts;
  if (pane.Cast<PartPane> ().IsNotNull())
  {
    parts.push_back(pane.Cast<PartPane> ());
  }
  else if (pane.Cast<PartStack> ().IsNotNull())
  {
    PartStack::Pointer stack = pane.Cast<PartStack> ();
    std::list<StackablePart::Pointer> children = stack->GetChildren();
    for (std::list<StackablePart::Pointer>::iterator iter = children.begin();
         iter != children.end(); ++iter)
    {
      if (iter->Cast<PartPane>() != 0)
      {
        parts.push_back(iter->Cast<PartPane>());
      }
    }
  }

  return parts;
}

PartSashContainer::~PartSashContainer()
{

}

void PartSashContainer::FindSashes(LayoutPart::Pointer pane,
    PartPane::Sashes& sashes)
{
  if (root == 0)
  {
    return;
  }
  LayoutTree::Pointer part = root->Find(pane);
  if (part == 0)
  {
    return;
  }
  part->FindSashes(sashes);
}

void PartSashContainer::Add(LayoutPart::Pointer child)
{
  if (child.IsNull())
  {
    return;
  }

  this->AddEnhanced(child, Constants::RIGHT, 0.5f, this->FindBottomRight());
}

void PartSashContainer::AddPart(StackablePart::Pointer child)
{
  if (child.IsNull())
  {
    return;
  }

  PartStack::Pointer newFolder = new PartStack(this->GetPage());
  newFolder->Add(child);
  this->AddEnhanced(newFolder, Constants::RIGHT, 0.5f, this->FindBottomRight());
}

void PartSashContainer::AddEnhanced(LayoutPart::Pointer child,
    int directionConstant, float ratioForNewPart, LayoutPart::Pointer relative)
{
  int relativePosition =
      PageLayout::ConstantToLayoutPosition(directionConstant);

  float ratioForUpperLeftPart;

  if (relativePosition == IPageLayout::RIGHT || relativePosition
      == IPageLayout::BOTTOM)
  {
    ratioForUpperLeftPart = 1.0f - ratioForNewPart;
  }
  else
  {
    ratioForUpperLeftPart = ratioForNewPart;
  }

  this->Add(child, relativePosition, ratioForUpperLeftPart, relative);
}

void PartSashContainer::Add(LayoutPart::Pointer child, int relationship,
    float ratio, LayoutPart::Pointer relative)
{
  bool isHorizontal = (relationship == IPageLayout::LEFT || relationship
      == IPageLayout::RIGHT);

  LayoutTree::Pointer node;
  if (root != 0 && relative != 0)
  {
    node = root->Find(relative);
  }

  Rectangle bounds;
  if (this->GetParent() == 0)
  {
    void* control = this->GetPage()->GetClientComposite();
    if (control != 0)
    {
      bounds = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(control);
    }
    else
    {
      bounds = Rectangle(0, 0, 800, 600);
    }
    bounds.x = 0;
    bounds.y = 0;
  }
  else
  {
    bounds = this->GetBounds();
  }

  int totalSize = this->MeasureTree(bounds, node, isHorizontal);

  int left = (int) (totalSize * ratio);
  int right = totalSize - left;

  this->Add(child, relationship, left, right, relative);
}

int PartSashContainer::MeasureTree(const Rectangle& outerBounds,
    LayoutTree::ConstPointer toMeasure, bool horizontal)
{
  if (toMeasure == 0)
  {
    return outerBounds.GetDimension(horizontal);
  }

  LayoutTreeNode* parent = toMeasure->GetParent();
  if (parent == 0)
  {
    return outerBounds.GetDimension(horizontal);
  }

  if (parent->GetSash()->IsHorizontal() == horizontal)
  {
    return MeasureTree(outerBounds, parent, horizontal);
  }

  bool isLeft = parent->IsLeftChild(toMeasure);

  LayoutTree::Pointer otherChild = parent->GetChild(!isLeft);
  if (otherChild->IsVisible())
  {
    int left = parent->GetSash()->GetLeft();
    int right = parent->GetSash()->GetRight();
    int childSize = isLeft ? left : right;

    int bias = parent->GetCompressionBias();

    // Normalize bias: 1 = we're fixed, -1 = other child is fixed
    if (isLeft)
    {
      bias = -bias;
    }

    if (bias == 1)
    {
      // If we're fixed, return the fixed size
      return childSize;
    }
    else if (bias == -1)
    {

      // If the other child is fixed, return the size of the parent minus the fixed size of the
      // other child
      return MeasureTree(outerBounds, parent, horizontal) - (left + right
          - childSize);
    }

    // Else return the size of the parent, scaled appropriately
    return MeasureTree(outerBounds, parent, horizontal) * childSize / (left
        + right);
  }

  return MeasureTree(outerBounds, parent, horizontal);
}

void PartSashContainer::AddChild(const RelationshipInfo& info)
{
  LayoutPart::Pointer child = info.part;

  children.push_back(child);

  if (root == 0)
  {
    root = new LayoutTree(child);
  }
  else
  {
    //Add the part to the tree.
    int vertical = (info.relationship == IPageLayout::LEFT || info.relationship
        == IPageLayout::RIGHT) ? Constants::VERTICAL : Constants::HORIZONTAL;
    bool left = info.relationship == IPageLayout::LEFT || info.relationship
        == IPageLayout::TOP;
    LayoutPartSash::Pointer sash = new LayoutPartSash(this, vertical);
    sash->SetSizes(info.left, info.right);
    if ((parent != 0) && child.Cast<PartPlaceholder> ().IsNull())
    {
      sash->CreateControl(parent);
    }
    LayoutTree::Pointer newroot = root->Insert(child, left, sash, info.relative);
    root = newroot;
  }

  this->ChildAdded(child);

  if (active)
  {
    child->CreateControl(parent);
    child->SetVisible(true);
    child->SetContainer(this);
    this->ResizeChild(child);
  }

}

void PartSashContainer::AddChildForPlaceholder(LayoutPart::Pointer child,
    LayoutPart::Pointer placeholder)
{
  RelationshipInfo newRelationshipInfo;
  newRelationshipInfo.part = child;
  if (root != 0)
  {
    newRelationshipInfo.relationship = IPageLayout::RIGHT;
    newRelationshipInfo.relative = root->FindBottomRight();
    newRelationshipInfo.left = 200;
    newRelationshipInfo.right = 200;
  }

  // find the relationship info for the placeholder
  std::vector<RelationshipInfo> relationships = this->ComputeRelation();
  for (unsigned int i = 0; i < relationships.size(); i++)
  {
    RelationshipInfo info = relationships[i];
    if (info.part == placeholder)
    {
      newRelationshipInfo.left = info.left;
      newRelationshipInfo.right = info.right;
      newRelationshipInfo.relationship = info.relationship;
      newRelationshipInfo.relative = info.relative;
    }
  }

  this->AddChild(newRelationshipInfo);
  this->FlushLayout();
}

bool PartSashContainer::AllowsBorder()
{
  return true;
}

void PartSashContainer::ChildAdded(LayoutPart::Pointer child)
{
  if (this->IsDeferred())
  {
    child->DeferUpdates(true);
  }
}

void PartSashContainer::ChildRemoved(LayoutPart::Pointer child)
{
  if (this->IsDeferred())
  {
    child->DeferUpdates(false);
  }
}

std::vector<PartSashContainer::RelationshipInfo> PartSashContainer::ComputeRelation()
{
  LayoutTree::Pointer treeRoot = root;
  std::list<RelationshipInfo> list;
  if (treeRoot == 0)
  {
    return std::vector<RelationshipInfo>();
  }
  RelationshipInfo r;
  r.part = treeRoot->ComputeRelation(list);
  list.push_front(r);
  std::vector<RelationshipInfo> result(list.begin(), list.end());
  return result;
}

void PartSashContainer::SetActive(bool isActive)
{
  if (isActive == active)
  {
    return;
  }

  active = isActive;

  ILayoutContainer::ChildrenType children = this->children;
  for (ILayoutContainer::ChildrenType::iterator childIter = children.begin();
       childIter != children.end(); ++childIter)
  {
    if (childIter->Cast<PartStack> ().IsNotNull())
    {
      PartStack::Pointer stack = childIter->Cast<PartStack>();
      stack->SetActive(isActive);
    }
  }

  if (isActive)
  {

    this->CreateControl(parentWidget);

    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->AddControlListener(parent, resizeListener);

    //TODO DND
    //DragUtil.addDragTarget(parent, this);
    //DragUtil.addDragTarget(parent.getShell(), this);

    ILayoutContainer::ChildrenType children = this->children;
    for (ILayoutContainer::ChildrenType::iterator childIter = children.begin();
         childIter != children.end(); ++childIter)
    {
      LayoutPart::Pointer child = *childIter;
      child->SetContainer(this);
      child->SetVisible(true); //zoomedPart == null || child == zoomedPart);
      if (child.Cast<PartStack> ().IsNull())
      {
        if (root != 0)
        {
          LayoutTree::Pointer node = root->Find(child);
          if (node != 0)
          {
            node->FlushCache();
          }
        }
      }
    }

    if (root != 0)
    {
      //root.flushChildren();
      //if (!isZoomed())
      {
        root->CreateControl(parent);
      }
    }

    this->ResizeSashes();
  }
  else
  {
    //TODO DND
    //DragUtil.removeDragTarget(parent, this);
    //DragUtil.removeDragTarget(parent.getShell(), this);

    // remove all Listeners
    if (resizeListener != 0 && parent != 0)
    {
      Tweaklets::Get(GuiWidgetsTweaklet::KEY)->RemoveControlListener(parent, resizeListener);
    }


    for (ILayoutContainer::ChildrenType::iterator iter = children.begin();
         iter != children.end(); ++iter)
    {
      LayoutPart::Pointer child = *iter;
      child->SetContainer(0);
      if (child.Cast<PartStack> ().IsNotNull())
      {
        child->SetVisible(false);
      }
    }

    this->DisposeSashes();

    //dispose();
  }
}

void PartSashContainer::CreateControl(void* parentWidget)
{
  if (this->parent != 0)
  {
    return;
  }

  parent = this->CreateParent(parentWidget);

  ILayoutContainer::ChildrenType children = this->children;
  for (ILayoutContainer::ChildrenType::iterator childIter = children.begin();
       childIter != children.end(); ++childIter)
  {
    (*childIter)->CreateControl(parent);
  }

}

void PartSashContainer::Dispose()
{
  if (parent == 0)
  {
    return;
  }

//  for (int i = 0, length = children.size(); i < length; i++)
//  {
//
//    // In PartSashContainer dispose really means deactivate, so we
//    // only dispose PartTabFolders.
//    if (children[i].Cast<PartStack> ().IsNotNull())
//    {
//      children[i]->Dispose();
//    }
//  }

  this->DisposeParent();
  this->parent = 0;
}

void PartSashContainer::DisposeSashes()
{
  if (root != 0)
  {
    root->DisposeSashes();
  }
}

void PartSashContainer::SetVisible(bool makeVisible)
{

  if (makeVisible == this->GetVisible())
  {
    return;
  }

  //if (!SwtUtil.isDisposed(this.parent))
  //{
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetEnabled(this->parent, makeVisible);
  //}
  LayoutPart::SetVisible(makeVisible);

  ILayoutContainer::ChildrenType children = this->children;
  for (ILayoutContainer::ChildrenType::iterator childIter = children.begin();
       childIter != children.end(); ++childIter)
  {
    (*childIter)->SetVisible(makeVisible); // && (zoomedPart == null || child == zoomedPart));
  }
}

LayoutPart::Pointer PartSashContainer::FindBottomRight()
{
  if (root == 0)
  {
    return 0;
  }
  return root->FindBottomRight();
}

Rectangle PartSashContainer::GetBounds()
{
  return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(this->parent);
}

ILayoutContainer::ChildrenType PartSashContainer::GetChildren()
{
  return children;
}

void* PartSashContainer::GetControl()
{
  return this->parent;
}

LayoutTree::Pointer PartSashContainer::GetLayoutTree()
{
  return root;
}

WorkbenchPage::Pointer PartSashContainer::GetPage()
{
  return page;
}

void* PartSashContainer::GetParent()
{
  return parent;
}

bool PartSashContainer::IsChild(LayoutPart::Pointer part)
{
  return std::find(children.begin(), children.end(), part) != children.end();
}

void PartSashContainer::ResizeChild(LayoutPart::Pointer childThatChanged)
{
  if (root != 0)
  {
    LayoutTree::Pointer tree = root->Find(childThatChanged);

    if (tree != 0)
    {
      tree->FlushCache();
    }
  }

  this->FlushLayout();

}

void PartSashContainer::Remove(LayoutPart::Pointer child)
{
//  if (child == getZoomedPart())
//  {
//    childRequestZoomOut();
//  }

  if (!this->IsChild(child))
  {
    return;
  }

  children.erase(std::find(children.begin(), children.end(), child));
  if (root != 0)
  {
    root = root->Remove(child);
  }
  this->ChildRemoved(child);

  if (active)
  {
    child->SetVisible(false);
    child->SetContainer(0);
    this->FlushLayout();
  }
}

void PartSashContainer::FlushLayout()
{
  layoutDirty = true;
  LayoutPart::FlushLayout();

  if (layoutDirty)
  {
    this->ResizeSashes();
  }
}

void PartSashContainer::Replace(LayoutPart::Pointer oldChild,
    LayoutPart::Pointer newChild)
{

  if (!this->IsChild(oldChild))
  {
    return;
  }

//  if (oldChild == getZoomedPart())
//  {
//    if (newChild.Cast<PartPlaceholder> ().IsNotNull())
//    {
//      childRequestZoomOut();
//    }
//    else
//    {
//      zoomedPart.setZoomed(false);
//      zoomedPart = newChild;
//      zoomedPart.setZoomed(true);
//    }
//  }

  children.erase(std::find(children.begin(), children.end(), oldChild));
  children.push_back(newChild);

  this->ChildAdded(newChild);

  if (root != 0)
  {
    LayoutTree::Pointer leaf;

    leaf = root->Find(oldChild);
    if (leaf != 0)
    {
      leaf->SetPart(newChild);
    }
  }

  this->ChildRemoved(oldChild);
  if (active)
  {
    oldChild->SetVisible(false);
    oldChild->SetContainer(0);
    newChild->CreateControl(parent);
    newChild->SetContainer(this);
    newChild->SetVisible(true); //zoomedPart == null || zoomedPart == newChild);
    this->ResizeChild(newChild);
  }
}

void PartSashContainer::ResizeSashes()
{
  layoutDirty = false;
  if (!active)
  {
    return;
  }

//  if (isZoomed())
//  {
//    getZoomedPart().setBounds(parent.getClientArea());
//  }
//  else
  {
    if (root != 0)
    {
      root->SetBounds(Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetClientArea(parent));
    }
  }
}

int PartSashContainer::ComputePreferredSize(bool width, int availableParallel,
    int availablePerpendicular, int preferredParallel)
{
//  if (isZoomed())
//  {
//    return getZoomedPart().computePreferredSize(width, availableParallel,
//        availablePerpendicular, preferredParallel);
//  }

  if (root != 0)
  {
    return root->ComputePreferredSize(width, availableParallel,
        availablePerpendicular, preferredParallel);
  }

  return preferredParallel;
}

int PartSashContainer::GetSizeFlags(bool width)
{
//  if (isZoomed())
//  {
//    return getZoomedPart().getSizeFlags(width);
//  }

  if (root != 0)
  {
    return root->GetSizeFlags(width);
  }

  return 0;
}

void PartSashContainer::SetBounds(const Rectangle& r)
{
  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetBounds(this->parent, r);
}

void PartSashContainer::Stack(StackablePart::Pointer newPart,
    IStackableContainer::Pointer container)
{
  //this->GetControl().setRedraw(false);

  // Only deref the part if it is being referenced in -this- perspective
  Perspective::Pointer persp = page->GetActivePerspective();
  PerspectiveHelper* pres = (persp != 0) ? persp->GetPresentation() : 0;
  if (pres != 0 && newPart.Cast<PartPane>() != 0)
  {
    IWorkbenchPartReference::Pointer newPartRef = newPart.Cast<PartPane>()->GetPartReference();
    IViewReference::Pointer vRef = newPartRef.Cast<IViewReference>();
    if (vRef != 0)
    {
      StackablePart::Pointer fpp = pres->FindPart(vRef->GetId(), vRef->GetSecondaryId());

      if (fpp != 0)
      {
        // Remove the part from old container.
        this->DerefPart(newPart);
      }
    }
  }
  else
  {
    // Remove the part from old container.
    this->DerefPart(newPart);
  }

  // Reparent part and add it to the workbook
  newPart->Reparent(this->GetParent());
  container->Add(newPart);

  //getControl().setRedraw(true);

}

//void PartSashContainer::DerefPart(LayoutPart::Pointer sourcePart)
//{
//  ILayoutContainer::Pointer container = sourcePart->GetContainer();
//  if (container != 0)
//  {
//    container->Remove(sourcePart);
//  }
//
//  if (container.Cast<LayoutPart> ().IsNotNull())
//  {
//    if (this->IsStackType(container.Cast<LayoutPart>()))
//    {
//      PartStack::Pointer stack = container.Cast<PartStack>();
//      if (stack->GetChildren().size() == 0)
//      {
//        this->Remove(stack);
//        //stack->Dispose();
//      }
//    }
//  }
//}
void PartSashContainer::DerefPart(StackablePart::Pointer sourcePart)
{
  IStackableContainer::Pointer container = sourcePart->GetContainer();
  if (container != 0)
  {
    container->Remove(sourcePart);

    if (container.Cast<LayoutPart>() != 0)
    {
      if (container->GetChildren().size() == 0)
      {
        this->Remove(container.Cast<LayoutPart>());
        //stack->Dispose();
      }
    }
  }
}

int PartSashContainer::GetVisibleChildrenCount(
    IStackableContainer::Pointer container)
{
  // Treat null as an empty container
  if (container == 0)
  {
    return 0;
  }

  IStackableContainer::ChildrenType children = container->GetChildren();

  int count = 0;
  for (IStackableContainer::ChildrenType::iterator iter = children.begin();
       iter != children.end(); ++iter)
  {
    if (!(*iter)->IsPlaceHolder())
    {
      count++;
    }
  }

  return count;
}

float PartSashContainer::GetDockingRatio(StackablePart::Pointer dragged,
    IStackableContainer::Pointer target)
{
  return 0.5f;
}

void PartSashContainer::DescribeLayout(std::string& buf) const
{
  if (root == 0)
  {
    return;
  }

//  if (isZoomed())
//  {
//    buf.append("zoomed "); //$NON-NLS-1$
//    root.describeLayout(buf);
//  }
//  else
  {
    buf.append("layout "); //$NON-NLS-1$
    root->DescribeLayout(buf);
  }
}

void PartSashContainer::Add(LayoutPart::Pointer child, int relationship,
    int left, int right, LayoutPart::Pointer relative)
{

  if (child == 0)
  {
    return;
  }
  if (relative != 0 && !this->IsChild(relative))
  {
    return;
  }
  if (relationship < IPageLayout::LEFT || relationship > IPageLayout::BOTTOM)
  {
    relationship = IPageLayout::LEFT;
  }

  // store info about relative positions
  RelationshipInfo info;
  info.part = child;
  info.relationship = relationship;
  info.left = left;
  info.right = right;
  info.relative = relative;
  this->AddChild(info);
}

bool PartSashContainer::AllowsAutoFocus()
{
  return true;
}

void PartSashContainer::StartDeferringEvents()
{
  LayoutPart::StartDeferringEvents();

  ILayoutContainer::ChildrenType deferredChildren = children;
  for (ILayoutContainer::ChildrenType::iterator iter = deferredChildren.begin();
       iter != deferredChildren.end(); ++iter)
  {
    (*iter)->DeferUpdates(true);
  }
}

void PartSashContainer::HandleDeferredEvents()
{
  LayoutPart::HandleDeferredEvents();

  ILayoutContainer::ChildrenType deferredChildren = children;
  for (ILayoutContainer::ChildrenType::iterator iter = deferredChildren.begin();
       iter != deferredChildren.end(); ++iter)
  {
    (*iter)->DeferUpdates(false);
  }
}

void PartSashContainer::TestInvariants()
{
  LayoutPart::TestInvariants();

  // If we have a parent container, ensure that we are displaying the zoomed appearance iff
  // our parent is zoomed in on us
//  if (this->GetContainer() != 0)
//  {
//    Assert.isTrue((getZoomedPart() != null) == (getContainer().childIsZoomed(
//        this)));
//  }

  ILayoutContainer::ChildrenType childArray = this->GetChildren();
  for (ILayoutContainer::ChildrenType::iterator iter = childArray.begin();
       iter != childArray.end(); ++iter)
  {
    (*iter)->TestInvariants();
  }

  // If we're zoomed, ensure that we're actually zoomed into one of our children
//  if (isZoomed())
//  {
//    Assert.isTrue(children.contains(zoomedPart));
//  }
}

}
