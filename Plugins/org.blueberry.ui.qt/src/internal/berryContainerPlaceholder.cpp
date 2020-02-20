/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryContainerPlaceholder.h"

#include "berryPartPlaceholder.h"
#include "berryILayoutContainer.h"

namespace berry
{

int ContainerPlaceholder::nextId = 0;

ContainerPlaceholder::ContainerPlaceholder(const QString& id) :
  PartPlaceholder(id == "" ? QString("Container Placeholder ") + nextId++ : id)
{

}

void ContainerPlaceholder::Add(LayoutPart::Pointer child)
{
  if (child.Cast<PartPlaceholder>() == 0)
  {
    return;
  }
  realContainer->Add(child);
}

bool ContainerPlaceholder::AllowsAdd(LayoutPart::Pointer  /*toAdd*/)
{
  return false;
}

QList<LayoutPart::Pointer> ContainerPlaceholder::GetChildren() const
{
  return realContainer->GetChildren();
}

QString ContainerPlaceholder::GetID() const
{
  return LayoutPart::GetID();
}

LayoutPart::Pointer ContainerPlaceholder::GetRealContainer()
{
  return realContainer.Cast<LayoutPart>();
}

void ContainerPlaceholder::Remove(LayoutPart::Pointer child)
{
  if (child.Cast<PartPlaceholder> () == 0)
  {
    return;
  }
  realContainer->Remove(child);
}

void ContainerPlaceholder::Replace(LayoutPart::Pointer oldChild,
    LayoutPart::Pointer newChild)
{
  if (oldChild.Cast<PartPlaceholder>() == 0 && newChild.Cast<PartPlaceholder>()
      == 0)
  {
    return;
  }
  realContainer->Replace(oldChild, newChild);
}

void ContainerPlaceholder::SetRealContainer(
    ILayoutContainer::Pointer container)
{

  if (container == 0)
  {
    // set the parent container of the children back to the real container
    if (realContainer != 0)
    {
      QList<LayoutPart::Pointer> children = realContainer->GetChildren();
      for (QList<LayoutPart::Pointer>::iterator iter = children.begin(); iter
          != children.end(); ++iter)
      {
        (*iter)->SetContainer(realContainer);
      }
    }
  }
  else
  {
    // replace the real container with this place holder
    QList<LayoutPart::Pointer> children = container->GetChildren();
    for (QList<LayoutPart::Pointer>::iterator iter = children.begin(); iter
        != children.end(); ++iter)
    {
      (*iter)->SetContainer(ILayoutContainer::Pointer(this));
    }
  }

  this->realContainer = container;
}

void ContainerPlaceholder::FindSashes(LayoutPart::Pointer /*part*/, PartPane::Sashes& sashes)
{
  ILayoutContainer::Pointer container = this->GetContainer();

  if (container != 0) {
      container->FindSashes(LayoutPart::Pointer(this), sashes);
  }
}

void ContainerPlaceholder::ResizeChild(LayoutPart::Pointer  /*childThatChanged*/)
{

}

bool ContainerPlaceholder::AllowsAutoFocus()
{
  return false;
}

}
