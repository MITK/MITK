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

#include "tweaklets/berryGuiWidgetsTweaklet.h"

#include "berryPartSashContainer.h"

#include "berryLayoutTree.h"
#include "berryLayoutTreeNode.h"
#include "berryPartStack.h"
#include "berryPageLayout.h"
#include "berryPerspective.h"
#include "berryPerspectiveHelper.h"
#include "berryDragUtil.h"
#include "berryWorkbenchPlugin.h"
#include "berryIPreferencesService.h"
#include "berryIPreferences.h"

#include "berryWorkbenchPreferenceConstants.h"
#include "berryGeometry.h"
#include "berryPartPane.h"

#include "berryQtTracker.h"

#include "berryConstants.h"


namespace berry
{

bool PartSashContainer::leftToRight = true;

PartSashContainer::ControlListener::ControlListener(
    PartSashContainer* container) :
  partSashContainer(container)
{

}

GuiTk::IControlListener::Events::Types PartSashContainer::ControlListener::GetEventTypes() const
{
  return Events::RESIZED;
}

void PartSashContainer::ControlListener::ControlResized(
    GuiTk::ControlEvent::Pointer /*e*/)
{
  partSashContainer->ResizeSashes();
}

PartSashContainer::SashContainerDropTarget::SashContainerDropTarget(
    PartSashContainer* partSashContainer, Object::Pointer sourcePart, int side,
    int cursor, Object::Pointer targetPart) :
  partSashContainer(partSashContainer)
{
  this->SetTarget(sourcePart, side, cursor, targetPart);
}

void PartSashContainer::SashContainerDropTarget::SetTarget(
    Object::Pointer sourcePart, int side, int cursor,
    Object::Pointer targetPart)
{
  this->side = side;
  this->targetPart = targetPart;
  this->sourcePart = sourcePart;
  this->cursor = cursor;
}

void PartSashContainer::SashContainerDropTarget::Drop()
{
  if (side != Constants::NONE)
  {
    LayoutPart::Pointer visiblePart = sourcePart.Cast<LayoutPart> ();

    if (sourcePart.Cast<ILayoutContainer> () != 0)
    {
      visiblePart = partSashContainer->GetVisiblePart(sourcePart.Cast<
          ILayoutContainer> ());
    }

    partSashContainer->DropObject(
        partSashContainer->GetVisibleParts(sourcePart), visiblePart,
        targetPart, side);
  }
}

void PartSashContainer::DropObject(const QList<PartPane::Pointer>& toDrop,
    LayoutPart::Pointer visiblePart, Object::Pointer targetPart, int side)
{
  //getControl().setRedraw(false);

  // Targetpart is null if there isn't a part under the cursor (all the parts are
  // hidden or the container is empty). In this case, the actual side doesn't really
  // since we'll be the only visible container and will fill the entire space. However,
  // we can't leave it as Constants::CENTER since we can't stack if we don't have something
  // to stack on. In this case, we pick Constants::BOTTOM -- this will insert the new pane
  // below any currently-hidden parts.
  if (targetPart == 0 && side == Constants::CENTER)
  {
    side = Constants::BOTTOM;
  }

  PartStack::Pointer targetStack = targetPart.Cast<PartStack> ();
  if (targetStack == 0 && targetPart.Cast<PartPane>() != 0)
  {
    targetStack = targetPart.Cast<PartPane> ()->GetStack();
  }
  LayoutPart::Pointer targetLayoutPart = targetStack;

  // if targetLayoutPart == 0 then we normally got a EditorSashContainer
  if (targetLayoutPart == 0)
    targetLayoutPart = targetPart.Cast<LayoutPart>();

  if (side == Constants::CENTER)
  {

    if (this->IsStackType(targetStack))
    {

      for (int idx = 0; idx < toDrop.size(); idx++)
      {
        LayoutPart::Pointer next = toDrop[idx];
        this->Stack(next, targetStack);
      }
    }
  }
  else
  {
    PartStack::Pointer newPart = this->CreateStack();

    // if the toDrop array has 1 item propagate the stack
    // appearance
    if (toDrop.size() == 1 && toDrop[0]->GetStack() != 0)
    {
      toDrop[0]->GetStack()->CopyAppearanceProperties(newPart);
    }

    for (int idx = 0; idx < toDrop.size(); idx++)
    {
      LayoutPart::Pointer next = toDrop[idx];
      this->Stack(next, newPart);
    }

    this->AddEnhanced(newPart, side, this->GetDockingRatio(newPart, targetStack),
        targetLayoutPart);
  }

  if (visiblePart != 0)
  {
    this->SetVisiblePart(visiblePart->GetContainer(),
        visiblePart.Cast<PartPane> ());
  }

  //getControl().setRedraw(true);

  if (visiblePart != 0)
  {
    visiblePart->SetFocus();
  }
}

CursorType PartSashContainer::SashContainerDropTarget::GetCursor()
{
  return QtDragManager::PositionToCursorType(cursor);
}

QRect PartSashContainer::SashContainerDropTarget::GetSnapRectangle()
{
  QRect targetBounds;

  if (targetPart.Cast<LayoutPart> () != 0)
  {
    targetBounds = DragUtil::GetDisplayBounds(
        targetPart.Cast<LayoutPart> ()->GetControl());
  }
  else if (targetPart.Cast<LayoutPart> () != 0)
  {
    targetBounds = DragUtil::GetDisplayBounds(
        targetPart.Cast<LayoutPart> ()->GetControl());
  }
  else
  {
    targetBounds = DragUtil::GetDisplayBounds(partSashContainer->GetParent());
  }

  if (side == Constants::CENTER || side == Constants::NONE)
  {
    return targetBounds;
  }

  int distance = Geometry::GetDimension(targetBounds, !Geometry::IsHorizontal(
      side));

  ILayoutContainer::Pointer stack = targetPart.Cast<ILayoutContainer> ();
  if (stack == 0 && targetPart.Cast<LayoutPart> () != 0)
  {
    stack = targetPart.Cast<LayoutPart> ()->GetContainer();
  }

  return Geometry::GetExtrudedEdge(targetBounds, (int) (distance
      * partSashContainer->GetDockingRatio(sourcePart, stack)), side);
}

PartSashContainer::PartSashContainer(const QString& id,
    WorkbenchPage* _page, QWidget* _parentWidget) :
  LayoutPart(id), parentWidget(_parentWidget), parent(nullptr), page(_page), active(
      false), layoutDirty(false)
{
  resizeListener = new ControlListener(this);

  QString layout = WorkbenchPlugin::GetDefault()->GetPreferencesService()->
      GetSystemPreferences()->Get(WorkbenchPreferenceConstants::PREFERRED_SASH_LAYOUT,
          WorkbenchPreferenceConstants::LEFT);
  if (layout == WorkbenchPreferenceConstants::RIGHT)
  {
    leftToRight = false;
  }
}

QList<PartPane::Pointer> PartSashContainer::GetVisibleParts(
    Object::Pointer pane)
{
  QList<PartPane::Pointer> parts;
  if (pane.Cast<PartPane> ().IsNotNull())
  {
    parts.push_back(pane.Cast<PartPane> ());
  }
  else if (pane.Cast<PartStack> ().IsNotNull())
  {
    PartStack::Pointer stack = pane.Cast<PartStack> ();
    QList<LayoutPart::Pointer> children = stack->GetChildren();
    for (QList<LayoutPart::Pointer>::iterator iter = children.begin(); iter
        != children.end(); ++iter)
    {
      if (iter->Cast<PartPane> () != 0)
      {
        parts.push_back(iter->Cast<PartPane> ());
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

void PartSashContainer::AddPart(LayoutPart::Pointer child)
{
  if (child.IsNull())
  {
    return;
  }

  PartStack::Pointer newFolder = this->CreateStack();
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

  QRect bounds;
  if (this->GetParent() == nullptr)
  {
    QWidget* control = this->GetPage()->GetClientComposite();
    if (control != nullptr)
    {
      bounds = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(control);
    }
    else
    {
      bounds = QRect(0, 0, 800, 600);
    }
    bounds.setX(0);
    bounds.setY(0);
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

int PartSashContainer::MeasureTree(const QRect& outerBounds,
    LayoutTree::ConstPointer toMeasure, bool horizontal)
{
  if (toMeasure == 0)
  {
    return Geometry::GetDimension(outerBounds, horizontal);
  }

  LayoutTreeNode* parent = toMeasure->GetParent();
  if (parent == nullptr)
  {
    return Geometry::GetDimension(outerBounds, horizontal);
  }

  if (parent->GetSash()->IsHorizontal() == horizontal)
  {
    return MeasureTree(outerBounds, LayoutTree::ConstPointer(parent), horizontal);
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
      return MeasureTree(outerBounds, LayoutTree::ConstPointer(parent), horizontal) - (left + right
          - childSize);
    }

    // Else return the size of the parent, scaled appropriately
    return MeasureTree(outerBounds, LayoutTree::ConstPointer(parent), horizontal) * childSize / (left
        + right);
  }

  return MeasureTree(outerBounds, LayoutTree::ConstPointer(parent), horizontal);
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
    LayoutPartSash::Pointer sash(new LayoutPartSash(this, vertical));
    sash->SetSizes(info.left, info.right);
    if ((parent != nullptr) && child.Cast<PartPlaceholder> ().IsNull())
    {
      sash->CreateControl(parent);
    }
    LayoutTree::Pointer newroot =
        root->Insert(child, left, sash, info.relative);
    root = newroot;
  }

  this->ChildAdded(child);

  if (active)
  {
    child->CreateControl(parent);
    child->SetVisible(true);
    child->SetContainer(ILayoutContainer::Pointer(this));
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
  QList<RelationshipInfo> relationships = this->ComputeRelation();
  for (int i = 0; i < relationships.size(); i++)
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

QList<PartSashContainer::RelationshipInfo> PartSashContainer::ComputeRelation()
{
  LayoutTree::Pointer treeRoot = root;
  QList<RelationshipInfo> list;
  if (treeRoot == 0)
  {
    return QList<RelationshipInfo>();
  }
  RelationshipInfo r;
  r.part = treeRoot->ComputeRelation(list);
  list.push_front(r);
  return list;
}

void PartSashContainer::SetActive(bool isActive)
{
  if (isActive == active)
  {
    return;
  }

  active = isActive;

  ILayoutContainer::ChildrenType children = this->children;
  for (ILayoutContainer::ChildrenType::iterator childIter = children.begin(); childIter
      != children.end(); ++childIter)
  {
    if (childIter->Cast<PartStack> ().IsNotNull())
    {
      PartStack::Pointer stack = childIter->Cast<PartStack> ();
      stack->SetActive(isActive);
    }
  }

  if (isActive)
  {

    this->CreateControl(parentWidget);

    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->AddControlListener(parent,
        resizeListener);

    DragUtil::AddDragTarget(parent, this);
    DragUtil::AddDragTarget(Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetShell(parent)->GetControl(), this);

    ILayoutContainer::ChildrenType children = this->children;
    for (ILayoutContainer::ChildrenType::iterator childIter = children.begin(); childIter
        != children.end(); ++childIter)
    {
      LayoutPart::Pointer child = *childIter;
      child->SetContainer(ILayoutContainer::Pointer(this));
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
    DragUtil::RemoveDragTarget(parent, this);
    DragUtil::RemoveDragTarget(Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetShell(parent)->GetControl(), this);

    // remove all Listeners
    if (resizeListener != 0 && parent != nullptr)
    {
      Tweaklets::Get(GuiWidgetsTweaklet::KEY)->RemoveControlListener(parent,
          resizeListener);
    }

    for (ILayoutContainer::ChildrenType::iterator iter = children.begin(); iter
        != children.end(); ++iter)
    {
      LayoutPart::Pointer child = *iter;
      child->SetContainer(ILayoutContainer::Pointer(nullptr));
      if (child.Cast<PartStack> ().IsNotNull())
      {
        child->SetVisible(false);
      }
    }

    this->DisposeSashes();

    //dispose();
  }
}

void PartSashContainer::CreateControl(QWidget* parentWidget)
{
  if (this->parent != nullptr)
  {
    return;
  }

  parent = this->CreateParent(parentWidget);

  ILayoutContainer::ChildrenType children = this->children;
  for (ILayoutContainer::ChildrenType::iterator childIter = children.begin(); childIter
      != children.end(); ++childIter)
  {
    (*childIter)->CreateControl(parent);
  }

}

void PartSashContainer::Dispose()
{
  if (parent == nullptr)
  {
    return;
  }

  for (ILayoutContainer::ChildrenType::iterator iter = children.begin();
       iter != children.end(); ++iter)
  {
    // In PartSashContainer dispose really means deactivate, so we
    // only dispose PartTabFolders.
    if (iter->Cast<PartStack>() != 0)
    {
      (*iter)->Dispose();
    }
  }

  this->DisposeParent();
  this->parent = nullptr;
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
  for (ILayoutContainer::ChildrenType::iterator childIter = children.begin(); childIter
      != children.end(); ++childIter)
  {
    (*childIter)->SetVisible(makeVisible); // && (zoomedPart == null || child == zoomedPart));
  }
}

LayoutPart::Pointer PartSashContainer::FindBottomRight()
{
  if (root == 0)
  {
    return LayoutPart::Pointer(nullptr);
  }
  return root->FindBottomRight();
}

QRect PartSashContainer::GetBounds()
{
  return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(this->parent);
}

ILayoutContainer::ChildrenType PartSashContainer::GetChildren() const
{
  return children;
}

QWidget* PartSashContainer::GetControl()
{
  return this->parent;
}

LayoutTree::Pointer PartSashContainer::GetLayoutTree()
{
  return root;
}

WorkbenchPage::Pointer PartSashContainer::GetPage()
{
  return WorkbenchPage::Pointer(page);
}

QWidget* PartSashContainer::GetParent()
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

  children.removeAll(child);
  if (root != 0)
  {
    root = root->Remove(child);
  }
  this->ChildRemoved(child);

  if (active)
  {
    child->SetVisible(false);
    child->SetContainer(ILayoutContainer::Pointer(nullptr));
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
    oldChild->SetContainer(ILayoutContainer::Pointer(nullptr));
    newChild->CreateControl(parent);
    newChild->SetContainer(ILayoutContainer::Pointer(this));
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
      root->SetBounds(Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetClientArea(
          parent));
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

void PartSashContainer::SetBounds(const QRect& r)
{
  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetBounds(this->parent, r);
}

IDropTarget::Pointer PartSashContainer::Drag(QWidget* /*currentControl*/,
    const Object::Pointer& draggedObject, const QPoint& position,
    const QRect&  /*dragRectangle*/)
{
  if (!(draggedObject.Cast<PartStack> () != 0
      || draggedObject.Cast<PartPane> () != 0))
  {
    return IDropTarget::Pointer(nullptr);
  }

  PartPane::Pointer sourcePart = draggedObject.Cast<PartPane> ();
  PartStack::Pointer sourceContainer = draggedObject.Cast<PartStack> ();
  if (sourceContainer == 0)
  {
    sourceContainer = sourcePart->GetStack();
  }

  if (!this->IsStackType(sourceContainer) && !this->IsPaneType(sourcePart))
  {
    return IDropTarget::Pointer(nullptr);
  }

  IWorkbenchWindow::Pointer window = sourcePart ? sourcePart->GetWorkbenchWindow() : sourceContainer->GetWorkbenchWindow();
  bool differentWindows = window
      != this->GetWorkbenchWindow();
  bool editorDropOK = ((sourceContainer->GetAppearance()
      == PresentationFactoryUtil::ROLE_EDITOR)
      && window->GetWorkbench()
          == this->GetWorkbenchWindow()->GetWorkbench());
  if (differentWindows && !editorDropOK)
  {
    return IDropTarget::Pointer(nullptr);
  }

  QRect containerBounds = DragUtil::GetDisplayBounds(parent);
  LayoutPart::Pointer targetPart;

  // If this container has no visible children
  if (this->GetVisibleChildrenCount(ILayoutContainer::Pointer(this)) == 0)
  {
    return this->CreateDropTarget(draggedObject, Constants::CENTER,
        Constants::CENTER, Object::Pointer(nullptr));
  }

  if (containerBounds.contains(position))
  {

    if (root != 0)
    {
      targetPart = root->FindPart(
          Tweaklets::Get(GuiWidgetsTweaklet::KEY)->ToControl(parent, position));
    }

    if (targetPart != 0)
    {
      QWidget* targetControl = targetPart->GetControl();

      QRect targetBounds = DragUtil::GetDisplayBounds(targetControl);

      int side = Geometry::GetClosestSide(targetBounds, position);
      int distance =
          Geometry::GetDistanceFromEdge(targetBounds, position, side);

      // is the source coming from a standalone part
      bool standalone = (this->IsStackType(sourceContainer)
          && sourceContainer->IsStandalone()) || (this->IsPaneType(sourcePart)
          && sourcePart->GetStack()->IsStandalone());

      // Only allow dropping onto an existing stack from different windows
      if (differentWindows && targetPart.Cast<PartStack> () != 0
          && targetPart.Cast<PartStack> ()->GetAppearance()
              == PresentationFactoryUtil::ROLE_EDITOR)
      {
        IDropTarget::Pointer target = targetPart->GetDropTarget(draggedObject,
            position);
        return target;
      }

      // Reserve the 5 pixels around the edge of the part for the drop-on-edge cursor
      if (distance >= 5 && !standalone)
      {
        // Otherwise, ask the part if it has any special meaning for this drop location
        IDropTarget::Pointer target = targetPart->GetDropTarget(draggedObject,
            position);
        if (target != 0)
        {
          return target;
        }
      }

      if (distance > 30 && this->IsStackType(targetPart.Cast<PartStack> ())
          && !standalone)
      {
        PartStack::Pointer targetContainer = targetPart.Cast<PartStack> ();
        if (targetContainer->AllowsAdd(sourcePart))
        {
          side = Constants::CENTER;
        }
      }

      // If the part doesn't want to override this drop location then drop on the edge

      // A "pointless drop" would be one that will put the dragged object back where it started.
      // Note that it should be perfectly valid to drag an object back to where it came from -- however,
      // the drop should be ignored.

      bool pointlessDrop = false; // = isZoomed();

      if (!sourcePart && sourceContainer == targetPart)
      {
        pointlessDrop = true;
      }

      if ((sourceContainer != 0) && (sourceContainer == targetPart)
          && this->GetVisibleChildrenCount(sourceContainer.Cast<ILayoutContainer>()) <= 1)
      {
        pointlessDrop = true;
      }

      if (side == Constants::CENTER && sourceContainer == targetPart)
      {
        pointlessDrop = true;
      }

      int cursor = side;

      if (pointlessDrop)
      {
        side = Constants::NONE;
        cursor = Constants::CENTER;
      }

      if (sourcePart)
        return this->CreateDropTarget(sourcePart, side, cursor, targetPart);
      else
        return this->CreateDropTarget(sourceContainer, side, cursor, targetPart);
    }
  }
  else
  {
    // We only allow dropping into a stack, not creating one
    if (differentWindows)
      return IDropTarget::Pointer(nullptr);

    int side = Geometry::GetClosestSide(containerBounds, position);

    bool pointlessDrop = false; // = isZoomed();

    if (/*(this->IsStackType(sourceContainer) && sourceContainer == this)
        ||*/ (this->IsPaneType(sourcePart) && this->GetVisibleChildrenCount(
            sourceContainer.Cast<ILayoutContainer>()) <= 1) && sourceContainer->GetContainer() == this)
    {
      if (root == 0 || this->GetVisibleChildrenCount(ILayoutContainer::Pointer(this)) <= 1)
      {
        pointlessDrop = true;
      }
    }

    int cursor = Geometry::GetOppositeSide(side);

    if (pointlessDrop)
    {
      side = Constants::NONE;
    }

    if (sourcePart)
      return this->CreateDropTarget(sourcePart, side, cursor, Object::Pointer(nullptr));
    else
      return this->CreateDropTarget(sourceContainer, side, cursor, Object::Pointer(nullptr));
  }

  return IDropTarget::Pointer(nullptr);
}

PartSashContainer::SashContainerDropTarget::Pointer
PartSashContainer::CreateDropTarget(
    Object::Pointer sourcePart, int side, int cursor,
    Object::Pointer targetPart)
{
  if (dropTarget == 0)
  {
    dropTarget
        = new SashContainerDropTarget(this, sourcePart, side, cursor, targetPart);
  }
  else
  {
    dropTarget->SetTarget(sourcePart, side, cursor, targetPart);
  }
  return dropTarget;
}

void PartSashContainer::Stack(LayoutPart::Pointer newPart,
    ILayoutContainer::Pointer container)
{
  //this->GetControl().setRedraw(false);

  // Only deref the part if it is being referenced in -this- perspective
  Perspective::Pointer persp = page->GetActivePerspective();
  PerspectiveHelper* pres = (persp != 0) ? persp->GetPresentation() : nullptr;
  if (pres != nullptr && container.Cast<PartStack>()->GetAppearance() != PresentationFactoryUtil::ROLE_EDITOR)
  {
    IWorkbenchPartReference::Pointer newPartRef =
        newPart.Cast<PartPane> ()->GetPartReference();
    IViewReference::Pointer vRef = newPartRef.Cast<IViewReference> ();
    if (vRef != 0)
    {
      LayoutPart::Pointer fpp = pres->FindPart(vRef->GetId(),
          vRef->GetSecondaryId());

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

void PartSashContainer::DerefPart(LayoutPart::Pointer sourcePart)
{
  ILayoutContainer::Pointer container = sourcePart->GetContainer();
  if (container != 0)
  {
    container->Remove(sourcePart);

    if (this->IsStackType(container) && container.Cast<LayoutPart> () != 0)
    {
      if (container->GetChildren().size() == 0)
      {
        LayoutPart::Pointer stack = container.Cast<LayoutPart> ();
        this->Remove(stack);
        stack->Dispose();
      }
    }
  }
}

std::size_t PartSashContainer::GetVisibleChildrenCount(
    ILayoutContainer::Pointer container)
{
  // Treat null as an empty container
  if (container == 0)
  {
    return 0;
  }

  ILayoutContainer::ChildrenType children = container->GetChildren();

  std::size_t count = 0;
  for (ILayoutContainer::ChildrenType::iterator iter = children.begin(); iter
      != children.end(); ++iter)
  {
    if (!(*iter)->IsPlaceHolder())
    {
      count++;
    }
  }

  return count;
}

float PartSashContainer::GetDockingRatio(Object::Pointer /*dragged*/,
    ILayoutContainer::Pointer  /*target*/)
{
  return 0.5f;
}

void PartSashContainer::DescribeLayout(QString& buf) const
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
  for (ILayoutContainer::ChildrenType::iterator iter = deferredChildren.begin(); iter
      != deferredChildren.end(); ++iter)
  {
    (*iter)->DeferUpdates(true);
  }
}

void PartSashContainer::HandleDeferredEvents()
{
  LayoutPart::HandleDeferredEvents();

  ILayoutContainer::ChildrenType deferredChildren = children;
  for (ILayoutContainer::ChildrenType::iterator iter = deferredChildren.begin(); iter
      != deferredChildren.end(); ++iter)
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
  for (ILayoutContainer::ChildrenType::iterator iter = childArray.begin(); iter
      != childArray.end(); ++iter)
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
