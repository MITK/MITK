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

#include "cherryContainerPlaceholder.h"

#include "cherryPartPlaceholder.h"
#include "cherryILayoutContainer.h"

namespace cherry
{

int ContainerPlaceholder::nextId = 0;

ContainerPlaceholder::ContainerPlaceholder(const std::string& id) :
  LayoutPart(id == "" ? "Container Placeholder " + nextId++ : id)
{

}

void ContainerPlaceholder::CreateControl(void* parent)
{

}

void* ContainerPlaceholder::GetControl()
{
  return 0;
}

void ContainerPlaceholder::Add(StackablePart::Pointer child)
{
  if (child.Cast<PartPlaceholder>() == 0)
  {
    return;
  }
  realContainer->Add(child);
}

bool ContainerPlaceholder::AllowsAdd(StackablePart::Pointer toAdd)
{
  return false;
}

std::list<StackablePart::Pointer> ContainerPlaceholder::GetChildren() const
{
  return realContainer->GetChildren();
}

std::string ContainerPlaceholder::GetID() const
{
  return LayoutPart::GetID();
}

IStackableContainer::Pointer ContainerPlaceholder::GetRealContainer()
{
  return realContainer;
}

void ContainerPlaceholder::Remove(StackablePart::Pointer child)
{
  if (child.Cast<PartPlaceholder> () == 0)
  {
    return;
  }
  realContainer->Remove(child);
}

void ContainerPlaceholder::Replace(StackablePart::Pointer oldChild,
    StackablePart::Pointer newChild)
{
  if (oldChild.Cast<PartPlaceholder>() == 0 && newChild.Cast<PartPlaceholder>()
      == 0)
  {
    return;
  }
  realContainer->Replace(oldChild, newChild);
}

void ContainerPlaceholder::SetRealContainer(
    IStackableContainer::Pointer container)
{

  if (container == 0)
  {
    // set the parent container of the children back to the real container
    if (realContainer != 0)
    {
      std::list<StackablePart::Pointer> children = realContainer->GetChildren();
      for (std::list<StackablePart::Pointer>::iterator iter = children.begin(); iter
          != children.end(); ++iter)
      {
        (*iter)->SetContainer(realContainer);
      }
    }
  }
  else
  {
    // replace the real container with this place holder
    std::list<StackablePart::Pointer> children = container->GetChildren();
    for (std::list<StackablePart::Pointer>::iterator iter = children.begin(); iter
        != children.end(); ++iter)
    {
      (*iter)->SetContainer(IStackableContainer::Pointer(this));
    }
  }

  this->realContainer = container;
}

void ContainerPlaceholder::FindSashes(PartPane::Sashes& sashes)
{
  ILayoutContainer::Pointer container = this->GetContainer();

  if (container != 0) {
      container->FindSashes(LayoutPart::Pointer(this), sashes);
  }
}

void ContainerPlaceholder::ResizeChild(StackablePart::Pointer childThatChanged)
{

}

bool ContainerPlaceholder::AllowsAutoFocus()
{
  return false;
}

}
