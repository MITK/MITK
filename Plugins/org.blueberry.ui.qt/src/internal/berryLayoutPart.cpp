/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "tweaklets/berryGuiWidgetsTweaklet.h"

#include "berryLayoutPart.h"

#include "berryILayoutContainer.h"
#include "berryDetachedWindow.h"

#include "berryIWorkbenchWindow.h"
#include "berryConstants.h"


namespace berry
{

const QString LayoutPart::PROP_VISIBILITY = "PROP_VISIBILITY";


LayoutPart::LayoutPart(const QString& id_)
  : container(nullptr), id(id_), deferCount(0)
{

}

LayoutPart::~LayoutPart()
{
}

bool LayoutPart::AllowsAutoFocus()
{
  if (container != nullptr)
  {
    return container->AllowsAutoFocus();
  }
  return true;
}

void LayoutPart::Dispose()
{

}

QRect LayoutPart::GetBounds()
{
  if (this->GetControl() == nullptr)
    return QRect();

  return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(this->GetControl());
}

ILayoutContainer::Pointer LayoutPart::GetContainer()
{
  return ILayoutContainer::Pointer(container);
}

bool LayoutPart::IsPlaceHolder() const
{
  return false;
}

QString LayoutPart::GetID() const
{
  return id;
}

bool LayoutPart::IsCompressible()
{
  return false;
}

QSize LayoutPart::GetSize()
{
  return this->GetBounds().size();
}

int LayoutPart::GetSizeFlags(bool  /*horizontal*/)
{
  return Constants::MIN;
}

int LayoutPart::ComputePreferredSize(bool /*width*/, int /*availableParallel*/,
    int  /*availablePerpendicular*/, int preferredParallel)
{
  return preferredParallel;
}

IDropTarget::Pointer LayoutPart::GetDropTarget(Object::Pointer  /*draggedObject*/, const QPoint&  /*displayCoordinates*/)
{
  return IDropTarget::Pointer(nullptr);
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
  QWidget* ctrl = this->GetControl();
  if (ctrl)
  {
    return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetShell(ctrl);
  }
  return Shell::Pointer(nullptr);
}

IWorkbenchWindow::Pointer LayoutPart::GetWorkbenchWindow()
{
  Shell::Pointer s = this->GetShell();
  if (s == 0)
  {
    return IWorkbenchWindow::Pointer(nullptr);
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

  return IWorkbenchWindow::Pointer(nullptr);

}

void LayoutPart::MoveAbove(QWidget*  /*refControl*/)
{
}

void LayoutPart::Reparent(QWidget* newParent)
{
  QWidget* control = this->GetControl();

  GuiWidgetsTweaklet* guiTweaklet = Tweaklets::Get(GuiWidgetsTweaklet::KEY);
  if ((control == nullptr) || (guiTweaklet->GetParent(control) == newParent))
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
    guiTweaklet->MoveAbove(control, nullptr);
  }
}

bool LayoutPart::GetVisible()
{
  QWidget* ctrl = this->GetControl();
  if (ctrl)
  {
    return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetVisible(ctrl);
  }

  return false;
}

bool LayoutPart::IsVisible()
{
  QWidget* ctrl = this->GetControl();
  if (ctrl)
  {
    return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->IsVisible(ctrl);
  }

  return false;
}

void LayoutPart::SetVisible(bool makeVisible)
{
  QWidget* ctrl = this->GetControl();
  if (ctrl != nullptr)
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

bool LayoutPart::IsFocusAncestor(QWidget*  /*ctrl*/)
{
//  Control f = ctrl.getDisplay().getFocusControl();
//  while (f != null && f != ctrl)
//  {
//    f = f.getParent();
//  }
//  return f == ctrl;
  return false;
}

void LayoutPart::SetBounds(const QRect& r)
{
  QWidget* ctrl = this->GetControl();
  if (ctrl)
  {
    return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetBounds(ctrl, r);
  }
}

void LayoutPart::SetContainer(ILayoutContainer::Pointer container)
{

  this->container = container.GetPointer();

  //TODO Zoom
//  if (container != 0)
//  {
//    setZoomed(container.childIsZoomed(this));
  //  }
}

void LayoutPart::SetFocus()
{
}

void LayoutPart::SetID(const QString& str)
{
  id = str;
}

LayoutPart::Pointer LayoutPart::GetPart()
{
  return LayoutPart::Pointer(this);
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

void LayoutPart::DescribeLayout(QString&  /*buf*/) const
{

}

QString LayoutPart::GetPlaceHolderId()
{
  return this->GetID();
}

void LayoutPart::ResizeChild(LayoutPart::Pointer  /*childThatChanged*/)
{

}

void LayoutPart::FlushLayout()
{
  ILayoutContainer::Pointer container = this->GetContainer();
  if (container != 0)
  {
    container->ResizeChild(LayoutPart::Pointer(this));
  }
}

bool LayoutPart::AllowsAdd(LayoutPart::Pointer  /*toAdd*/)
{
  return false;
}

QString LayoutPart::ToString() const
{
  return "";
}

void LayoutPart::TestInvariants()
{
}

}
