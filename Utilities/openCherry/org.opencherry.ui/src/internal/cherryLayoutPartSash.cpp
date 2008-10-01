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

#include "cherryLayoutPartSash.h"

#include "cherryLayoutTree.h"
#include "cherryLayoutTreeNode.h"
#include "cherryWorkbenchPlugin.h"

#include "../cherryConstants.h"
#include "../tweaklets/cherryGuiWidgetsTweaklet.h"

namespace cherry
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
 : LayoutPart(""), sash(0), enabled(false)
{
  this->style = style;
  this->rootContainer = rootContainer;

  left = 300;
  right = 300;

  selectionListener = new SelectionListener(this);
}

void LayoutPartSash::CheckDragLimit(GuiTk::SelectionEvent::Pointer event)
{
  LayoutTree::Pointer root = rootContainer->GetLayoutTree();
  LayoutTreeNode::Pointer node = root->FindSash(this);
  Rectangle nodeBounds = node->GetBounds();
  Rectangle eventRect(event->x, event->y, event->width, event->height);

  bool vertical = (style == Constants::VERTICAL);

  // If a horizontal sash, flip the coordinate system so that we
  // can handle horizontal and vertical sashes without special cases
  if (!vertical)
  {
    nodeBounds.FlipXY();
    eventRect.FlipXY();
  }

  int eventX = eventRect.x;
  int left = std::max(0, eventX - nodeBounds.x);
  left = std::min(left, nodeBounds.width - this->GetSashSize());
  int right = nodeBounds.width - left - this->GetSashSize();

  LayoutTreeNode::ChildSizes sizes = node->ComputeChildSizes(nodeBounds.width, nodeBounds.height, left, right, nodeBounds.width);

  eventRect.x = nodeBounds.x + sizes.left;

  // If it's a horizontal sash, restore eventRect to its original coordinate system
  if (!vertical)
  {
    eventRect.FlipXY();
  }

  event->x = eventRect.x;
  event->y = eventRect.y;
}

void LayoutPartSash::CreateControl(void* /*parent*/)
{
  // Defer creation of the control until it becomes visible
  if (isVisible)
  {
    this->DoCreateControl();
  }
}

void LayoutPartSash::DoCreateControl()
{
  if (sash == 0)
  {
    // ask the presentation factory to create the sash
    IPresentationFactory* factory = WorkbenchPlugin::GetDefault()->GetPresentationFactory();

    int sashStyle = IPresentationFactory::SASHTYPE_NORMAL | style;
    sash = factory->CreateSash(this->rootContainer->GetParent(), sashStyle);

    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->AddSelectionListener(sash, selectionListener);
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetEnabled(sash, enabled);
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetBounds(sash, bounds);
    //sash.addSelectionListener(selectionListener);
    //sash.setEnabled(enabled);
    //sash.setBounds(bounds);
  }
}

void LayoutPartSash::SetBounds(const Rectangle& r)
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

  if (sash != 0)
  {
    bounds = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(sash);
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->Dispose(sash);
  }
  sash = 0;
}

Rectangle LayoutPartSash::GetBounds()
{
  if (sash == 0)
  {
    return bounds;
  }

  return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(sash);
}

void* LayoutPartSash::GetControl()
{
  return sash;
}

std::string LayoutPartSash::GetID()
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
    LayoutTreeNode::Pointer node = root->FindSash(this);
    if (node != 0)
    {
      node->FlushCache();
    }
  }
}

void LayoutPartSash::WidgetSelected(int x, int y, int width, int height)
{
  if (!enabled)
  {
    return;
  }

  LayoutTree::Pointer root = rootContainer->GetLayoutTree();
  LayoutTreeNode::Pointer node = root->FindSash(this);
  Rectangle nodeBounds = node->GetBounds();
  //Recompute ratio
  x -= nodeBounds.x;
  y -= nodeBounds.y;
  if (style == Constants::VERTICAL)
  {
    this->SetSizes(x, nodeBounds.width - x - this->GetSashSize());
  }
  else
  {
    this->SetSizes(y, nodeBounds.height - y - this->GetSashSize());
  }

  node->SetBounds(nodeBounds);
}

void LayoutPartSash::SetEnabled(bool resizable)
{
  this->enabled = resizable;
  if (sash != 0)
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
