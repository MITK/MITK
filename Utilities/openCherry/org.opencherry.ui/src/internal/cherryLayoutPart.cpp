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

namespace cherry
{

const std::string LayoutPart::PROP_VISIBILITY = "PROP_VISIBILITY"; //$NON-NLS-1$


LayoutPart::LayoutPart(const std::string& id_) :
  deferCount(0), id(id_)
{

}

LayoutPart::~LayoutPart()
{
}

bool LayoutPart::AllowsAutoFocus()
{
  if (container != null)
  {
    return container.allowsAutoFocus();
  }
  return true;
}

std::string LayoutPart::GetID()
{
  return id;
}

std::string LayoutPart::GetCompoundId()
{
  return getID();
}

bool LayoutPart::IsCompressible()
{
  return false;
}

IWorkbenchWindow::Pointer LayoutPart::GetWorkbenchWindow()
{
  Shell s = getShell();
  if (s==null)
  {
    return null;
  }
  Object data = s.getData();
  if (data instanceof IWorkbenchWindow)
  {
    return (IWorkbenchWindow)data;
  }
  else if (data instanceof DetachedWindow)
  {
    return ((DetachedWindow) data).getWorkbenchPage()
    .getWorkbenchWindow();
  }

  return null;

}

void LayoutPart::MoveAbove(void* refControl)
{
}

void LayoutPart::SetContainer(ILayoutContainer::Pointer container)
{

  this.container = container;

  if (container != null)
  {
    setZoomed(container.childIsZoomed(this));
  }
}

void LayoutPart::SetFocus()
{
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
      startDeferringEvents();
    }
    deferCount++;
  }
  else
  {
    if (deferCount > 0)
    {
      deferCount--;
      if (deferCount == 0)
      {
        handleDeferredEvents();
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
  return deferCount > 0;
}

void LayoutPart::DescribeLayout(std::string& buf)
{

}

std::string LayoutPart::GetPlaceHolderId()
{
  return getID();
}

bool LayoutPart::AllowsAdd(LayoutPart::Pointer toAdd)
{
  return false;
}

void LayoutPart::TestInvariants()
{
}

}
