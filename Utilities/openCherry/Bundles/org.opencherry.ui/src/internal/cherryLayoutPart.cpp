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

#include "cherryLayoutPart.h"

#include "cherryILayoutContainer.h"
#include "cherryDetachedWindow.h"

#include "../tweaklets/cherryGuiWidgetsTweaklet.h"
#include "../cherryIWorkbenchWindow.h"
#include "../cherryConstants.h"


namespace cherry
{

const std::string LayoutPart::PROP_VISIBILITY = "PROP_VISIBILITY"; //$NON-NLS-1$


LayoutPart::LayoutPart(const std::string& id_) :
  id(id_), deferCount(0)
{

}

LayoutPart::~LayoutPart()
{
}

bool LayoutPart::AllowsAutoFocus()
{
  if (container != 0)
  {
    return container->AllowsAutoFocus();
  }
  return true;
}

void LayoutPart::Dispose()
{

}

Rectangle LayoutPart::GetBounds()
{
  if (this->GetControl() == 0)
    return Rectangle();

  return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(this->GetControl());
}

ILayoutContainer::Pointer LayoutPart::GetContainer()
{
  return container;
}

std::string LayoutPart::GetID()
{
  return id;
}

bool LayoutPart::IsCompressible()
{
  return false;
}

Point LayoutPart::GetSize()
{
  Rectangle r = this->GetBounds();
  Point ptSize(r.width, r.height);
  return ptSize;
}

int LayoutPart::GetSizeFlags(bool horizontal)
{
  return Constants::MIN;
}

int LayoutPart::ComputePreferredSize(bool width, int availableParallel,
    int availablePerpendicular, int preferredParallel)
{
  return preferredParallel;
}

IDropTarget::Pointer LayoutPart::GetDropTarget(Object::Pointer draggedObject, const Point& displayCoordinates)
{
  return 0;
}

bool LayoutPart::IsDocked()
{
  Shell::Pointer s = this->GetShell();
  if (s == 0) {
      return false;
  }

  return s->GetData().Cast<IWorkbenchWindow>() != 0;
}

Shell::Pointer LayoutPart::GetShell()
{
  void* ctrl = this->GetControl();
  if (ctrl)
  {
    return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetShell(ctrl);
  }
  return 0;
}

IWorkbenchWindow::Pointer LayoutPart::GetWorkbenchWindow()
{
  Shell::Pointer s = this->GetShell();
  if (s == 0)
  {
    return 0;
  }
  Object::Pointer data = s->GetData();
  if (data.Cast<IWorkbenchWindow>() != 0)
  {
    return data.Cast<IWorkbenchWindow>();
  }
  else if (data.Cast<DetachedWindow>() != 0)
  {
    return data.Cast<DetachedWindow>()->GetWorkbenchPage()->GetWorkbenchWindow();
  }

  return 0;

}

void LayoutPart::MoveAbove(void* refControl)
{
}

void LayoutPart::Reparent(void* newParent)
{
  void* control = this->GetControl();

  GuiWidgetsTweaklet* guiTweaklet = Tweaklets::Get(GuiWidgetsTweaklet::KEY);
  if ((control == 0) || (guiTweaklet->GetParent(control) == newParent))
  {
    return;
  }

  if (guiTweaklet->IsReparentable(control))
  {
    // make control small in case it is not resized with other controls
    //control.setBounds(0, 0, 0, 0);
    // By setting the control to disabled before moving it,
    // we ensure that the focus goes away from the control and its children
    // and moves somewhere else
    bool enabled = guiTweaklet->GetEnabled(control);
    guiTweaklet->SetEnabled(control, false);
    guiTweaklet->SetParent(control, newParent);
    guiTweaklet->SetEnabled(control, enabled);
    guiTweaklet->MoveAbove(control, 0);
  }
}

bool LayoutPart::GetVisible()
{
  void* ctrl = this->GetControl();
  if (ctrl)
  {
    return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetVisible(ctrl);
  }

  return false;
}

bool LayoutPart::IsVisible()
{
  void* ctrl = this->GetControl();
  if (ctrl)
  {
    return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->IsVisible(ctrl);
  }

  return false;
}

void LayoutPart::SetVisible(bool makeVisible)
{
  void* ctrl = this->GetControl();
  if (ctrl != 0)
  {
    if (makeVisible == Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetVisible(ctrl))
    {
      return;
    }

//    if (!makeVisible && this->IsFocusAncestor(ctrl))
//    {
//      // Workaround for Bug 60970 [EditorMgmt] setActive() called on an editor when it does not have focus.
//      // Force focus on the shell so that when ctrl is hidden,
//      // SWT does not try to send focus elsewhere, which may cause
//      // some other part to be activated, which affects the part
//      // activation order and can cause flicker.
//      ctrl.getShell().forceFocus();
//    }

    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetVisible(ctrl, makeVisible);
}
}

bool LayoutPart::IsFocusAncestor(void* ctrl)
{
//  Control f = ctrl.getDisplay().getFocusControl();
//  while (f != null && f != ctrl)
//  {
//    f = f.getParent();
//  }
//  return f == ctrl;
  return false;
}

void LayoutPart::SetBounds(const Rectangle& r)
{
  void* ctrl = this->GetControl();
  if (ctrl)
  {
    return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetBounds(ctrl, r);
  }
}

void LayoutPart::SetContainer(ILayoutContainer::Pointer container)
{

  this->container = container;

  //TODO Zoom
//  if (container != 0)
//  {
//    setZoomed(container.childIsZoomed(this));
//  }
}

void LayoutPart::SetID(const std::string& str)
{
  id = str;
}

LayoutPart::Pointer LayoutPart::GetPart()
{
  return this;
}

void LayoutPart::DeferUpdates(bool shouldDefer)
{
  if (shouldDefer)
  {
    if (deferCount == 0)
    {
      this->StartDeferringEvents();
    }
    deferCount++;
  }
  else
  {
    if (deferCount> 0)
    {
      deferCount--;
      if (deferCount == 0)
      {
        this->HandleDeferredEvents();
      }
    }
  }
}

void LayoutPart::StartDeferringEvents()
{

}

void LayoutPart::HandleDeferredEvents()
{

}

bool LayoutPart::IsDeferred()
{
  return deferCount> 0;
}

void LayoutPart::DescribeLayout(std::string& buf) const
{

}

std::string LayoutPart::GetPlaceHolderId()
{
  return this->GetID();
}

void LayoutPart::ResizeChild(LayoutPart::Pointer childThatChanged)
{

}

void LayoutPart::FlushLayout()
{
  ILayoutContainer::Pointer container = this->GetContainer();
  if (container != 0)
  {
    container->ResizeChild(this);
  }
}

bool LayoutPart::AllowsAdd(LayoutPart::Pointer toAdd)
{
  return false;
}

std::string LayoutPart::ToString()
{
  return "";
}

void LayoutPart::TestInvariants()
{
}

}
