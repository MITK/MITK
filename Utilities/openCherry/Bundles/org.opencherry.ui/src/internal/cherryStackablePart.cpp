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

#include "cherryStackablePart.h"

#include "cherryIStackableContainer.h"
#include "cherryDetachedWindow.h"
#include "../cherryIWorkbenchWindow.h"
#include "../tweaklets/cherryGuiWidgetsTweaklet.h"

namespace cherry
{

StackablePart::StackablePart(std::string id_)
: id(id_)
{

}

IStackableContainer::Pointer StackablePart::GetContainer() const
{
  return container;
}

void StackablePart::SetContainer(IStackableContainer::Pointer container)
{
  this->container = container;
}

void StackablePart::Reparent(void* newParent)
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

void StackablePart::DescribeLayout(std::string& description) const
{

}

std::string StackablePart::GetPlaceHolderId() const
{
  return this->GetId();
}

std::string StackablePart::GetId() const
{
  return id;
}

void StackablePart::SetId(const std::string& id)
{
  this->id = id;
}

void StackablePart::SetFocus()
{

}

void StackablePart::SetBounds(const Rectangle& r)
{
  void* ctrl = this->GetControl();
  if (ctrl)
  {
    return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetBounds(ctrl, r);
  }
}

Rectangle StackablePart::GetBounds()
{
  return Rectangle();
}

Point StackablePart::GetSize()
{
  Rectangle r = this->GetBounds();
  Point ptSize(r.width, r.height);
  return ptSize;
}

bool StackablePart::IsDocked()
{
  Shell::Pointer s = this->GetShell();
  if (s == 0)
  {
    return false;
  }

  return s->GetData().Cast<IWorkbenchWindow>() != 0;
}

Shell::Pointer StackablePart::GetShell()
{
  void* ctrl = this->GetControl();
  if (ctrl)
  {
    return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetShell(ctrl);
  }
  return 0;
}

IWorkbenchWindow::Pointer StackablePart::GetWorkbenchWindow()
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

std::string StackablePart::GetCompoundId()
{
  return this->GetId();
}

bool StackablePart::IsPlaceHolder() const
{
  return false;
}

void StackablePart::TestInvariants()
{

}

}
