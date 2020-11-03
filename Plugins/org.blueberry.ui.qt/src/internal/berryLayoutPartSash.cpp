/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "tweaklets/berryGuiWidgetsTweaklet.h"

#include "berryLayoutPartSash.h"

#include "berryLayoutTree.h"
#include "berryLayoutTreeNode.h"
#include "berryWorkbenchPlugin.h"

#include "berryConstants.h"

namespace berry
{

LayoutPartSash::SelectionListener::SelectionListener(LayoutPartSash* lp)
 : layoutPartSash(lp)
{

}

void LayoutPartSash::SelectionListener::WidgetSelected(GuiTk::SelectionEvent::Pointer e)
{
  layoutPartSash->CheckDragLimit(e);

  if (e->detail != Constants::DRAG)
        {
          layoutPartSash->WidgetSelected(e->x, e->y, e->width,
              e->height);
        }
}

LayoutPartSash::LayoutPartSash(PartSashContainer* rootContainer, int style)
 : LayoutPart(""), sash(nullptr), enabled(false), rootContainer(rootContainer),
   style(style), left(300), right(300), presFactory(nullptr), isVisible(false)
{
  selectionListener = new SelectionListener(this);
}

LayoutPartSash::~LayoutPartSash()
{
  this->Dispose();
}

void LayoutPartSash::CheckDragLimit(GuiTk::SelectionEvent::Pointer event)
{
  LayoutTree::Pointer root = rootContainer->GetLayoutTree();
  LayoutTreeNode::Pointer node = root->FindSash(LayoutPartSash::Pointer(this));
  QRect nodeBounds = node->GetBounds();
  QRect eventRect(event->x, event->y, event->width, event->height);

  bool vertical = (style == Constants::VERTICAL);

  // If a horizontal sash, flip the coordinate system so that we
  // can handle horizontal and vertical sashes without special cases
  if (!vertical)
  {
    nodeBounds = QRect(nodeBounds.y(), nodeBounds.x(), nodeBounds.height(), nodeBounds.width());
    eventRect = QRect(eventRect.y(), eventRect.x(), eventRect.height(), eventRect.width());
  }

  int eventX = eventRect.x();
  int left = std::max<int>(0, eventX - nodeBounds.x());
  left = std::min<int>(left, nodeBounds.width() - this->GetSashSize());
  int right = nodeBounds.width() - left - this->GetSashSize();

  LayoutTreeNode::ChildSizes sizes = node->ComputeChildSizes(nodeBounds.width(), nodeBounds.height(), left, right, nodeBounds.width());

  eventRect.setX(nodeBounds.x() + sizes.left);

  // If it's a horizontal sash, restore eventRect to its original coordinate system
  if (!vertical)
  {
    eventRect = QRect(eventRect.y(), eventRect.x(), eventRect.height(), eventRect.width());
  }

  event->x = eventRect.x();
  event->y = eventRect.y();
}

void LayoutPartSash::CreateControl(QWidget* /*parent*/)
{
  // Defer creation of the control until it becomes visible
  if (isVisible)
  {
    this->DoCreateControl();
  }
}

void LayoutPartSash::DoCreateControl()
{
  if (sash == nullptr)
  {
    // ask the presentation factory to create the sash
    IPresentationFactory* factory = WorkbenchPlugin::GetDefault()->GetPresentationFactory();

    int sashStyle = IPresentationFactory::SASHTYPE_NORMAL | style;
    sash = factory->CreateSash(this->rootContainer->GetParent(), sashStyle);

    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->AddSelectionListener(sash, selectionListener);
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetEnabled(sash, enabled);
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetBounds(sash, bounds);
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetVisible(sash, isVisible);
  }
}

void LayoutPartSash::SetBounds(const QRect& r)
{
  LayoutPart::SetBounds(r);

  bounds = r;
}

void LayoutPartSash::SetVisible(bool visible)
{
  if (visible == isVisible)
  {
    return;
  }

  if (visible)
  {
    this->DoCreateControl();
  }
  else
  {
    this->Dispose();
  }

  LayoutPart::SetVisible(visible);

  isVisible = visible;
}

bool LayoutPartSash::IsVisible()
{
  return isVisible;
}

void LayoutPartSash::Dispose()
{

  if (sash != nullptr)
  {
    bounds = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(sash);
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->Dispose(sash);
  }
  sash = nullptr;
}

QRect LayoutPartSash::GetBounds()
{
  if (sash == nullptr)
  {
    return bounds;
  }

  return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(sash);
}

QWidget* LayoutPartSash::GetControl()
{
  return sash;
}

QString LayoutPartSash::GetID() const
{
  return "";
}

LayoutPartSash::Pointer LayoutPartSash::GetPostLimit()
{
  return postLimit;
}

LayoutPartSash::Pointer LayoutPartSash::GetPreLimit()
{
  return preLimit;
}

int LayoutPartSash::GetLeft()
{
  return left;
}

int LayoutPartSash::GetRight()
{
  return right;
}

bool LayoutPartSash::IsHorizontal()
{
  return ((style & Constants::HORIZONTAL) == Constants::HORIZONTAL);
}

bool LayoutPartSash::IsVertical()
{
  return ((style & Constants::VERTICAL) == Constants::VERTICAL);
}

void LayoutPartSash::SetPostLimit(LayoutPartSash::Pointer newPostLimit)
{
  postLimit = newPostLimit;
}

void LayoutPartSash::SetPreLimit(LayoutPartSash::Pointer newPreLimit)
{
  preLimit = newPreLimit;
}

void LayoutPartSash::SetRatio(float newRatio)
{
  int total = left + right;
  int newLeft = (int) (total * newRatio);
  this->SetSizes(newLeft, total - newLeft);
}

void LayoutPartSash::SetSizes(int left, int right)
{
  if (left < 0 || right < 0)
  {
    return;
  }

  if (left == this->left && right == this->right)
  {
    return;
  }

  this->left = left;
  this->right = right;

  this->FlushCache();
}

void LayoutPartSash::FlushCache()
{
  LayoutTree::Pointer root = rootContainer->GetLayoutTree();

  if (root != 0)
  {
    LayoutTreeNode::Pointer node = root->FindSash(LayoutPartSash::Pointer(this));
    if (node != 0)
    {
      node->FlushCache();
    }
  }
}

void LayoutPartSash::WidgetSelected(int x, int y, int  /*width*/, int  /*height*/)
{
  if (!enabled)
  {
    return;
  }

  LayoutTree::Pointer root = rootContainer->GetLayoutTree();
  LayoutTreeNode::Pointer node = root->FindSash(LayoutPartSash::Pointer(this));
  QRect nodeBounds = node->GetBounds();
  //Recompute ratio
  x -= nodeBounds.x();
  y -= nodeBounds.y();
  if (style == Constants::VERTICAL)
  {
    this->SetSizes(x, nodeBounds.width() - x - this->GetSashSize());
  }
  else
  {
    this->SetSizes(y, nodeBounds.height() - y - this->GetSashSize());
  }

  node->SetBounds(nodeBounds);
}

void LayoutPartSash::SetEnabled(bool resizable)
{
  this->enabled = resizable;
  if (sash != nullptr)
  {
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetEnabled(sash, enabled);
  }
}

int LayoutPartSash::GetSashSize() const
{
  IPresentationFactory* factory = WorkbenchPlugin::GetDefault()->GetPresentationFactory();
  int sashStyle = IPresentationFactory::SASHTYPE_NORMAL | style;
  int size = factory->GetSashSize(sashStyle);
  return size;
}

}
