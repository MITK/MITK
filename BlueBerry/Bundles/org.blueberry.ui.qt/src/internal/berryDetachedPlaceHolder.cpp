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

#include "berryDetachedPlaceHolder.h"

#include "berryILayoutContainer.h"
#include "berryPartPlaceholder.h"
#include "berryWorkbenchConstants.h"

namespace berry
{

DetachedPlaceHolder::DetachedPlaceHolder(const std::string& id,
    const Rectangle& b) :
  PartPlaceholder(id), bounds(b)
{

}

void DetachedPlaceHolder::Add(LayoutPart::Pointer newPart)
{
  if (newPart.Cast<PartPlaceholder> () == 0)
  {
    return;
  }
  children.push_back(newPart);
}

bool DetachedPlaceHolder::AllowsBorder()
{
  return false;
}

bool DetachedPlaceHolder::AllowsAdd(LayoutPart::Pointer toAdd)
{
  return PartPlaceholder::AllowsAdd(toAdd);
}

bool DetachedPlaceHolder::AllowsAutoFocus()
{
  return PartPlaceholder::AllowsAutoFocus();
}

Rectangle DetachedPlaceHolder::GetBounds()
{
  return bounds;
}

std::list<LayoutPart::Pointer> DetachedPlaceHolder::GetChildren()
{
  return children;
}

void DetachedPlaceHolder::Remove(LayoutPart::Pointer part)
{
  children.remove(part);
}

void DetachedPlaceHolder::Replace(LayoutPart::Pointer oldPart,
    LayoutPart::Pointer newPart)
{
  this->Remove(oldPart);
  this->Add(newPart);
}

void DetachedPlaceHolder::RestoreState(IMemento::Pointer memento)
{
  // Read the bounds.
  int x = 0;
  memento->GetInteger(WorkbenchConstants::TAG_X, x);
  int y = 0;
  memento->GetInteger(WorkbenchConstants::TAG_Y, y);
  int width = 0;
  memento->GetInteger(WorkbenchConstants::TAG_WIDTH, width);
  int height = 0;
  memento->GetInteger(WorkbenchConstants::TAG_HEIGHT, height);

  bounds = Rectangle(x, y, width, height);

  // Restore the placeholders.
  std::vector<IMemento::Pointer> childrenMem(memento
         ->GetChildren(WorkbenchConstants::TAG_VIEW));
  for (std::size_t i = 0; i < childrenMem.size(); i++) {
     std::string id;
     childrenMem[i]->GetString(WorkbenchConstants::TAG_ID, id);
     PartPlaceholder::Pointer holder(new PartPlaceholder(id));
     holder->SetContainer(ILayoutContainer::Pointer(this));
     children.push_back(holder);
  }
}

void DetachedPlaceHolder::SaveState(IMemento::Pointer memento)
{
  // Save the bounds.
  memento->PutInteger(WorkbenchConstants::TAG_X, bounds.x);
  memento->PutInteger(WorkbenchConstants::TAG_Y, bounds.y);
  memento->PutInteger(WorkbenchConstants::TAG_WIDTH, bounds.width);
  memento->PutInteger(WorkbenchConstants::TAG_HEIGHT, bounds.height);

  // Save the views.
  for (std::list<LayoutPart::Pointer>::iterator i = children.begin();
      i != children.end(); ++i) {
     IMemento::Pointer childMem = memento
             ->CreateChild(WorkbenchConstants::TAG_VIEW);
     childMem->PutString(WorkbenchConstants::TAG_ID, (*i)->GetID());
  }
}

void DetachedPlaceHolder::FindSashes(LayoutPart::Pointer /*part*/,
    PartPane::Sashes& sashes)
{
  ILayoutContainer::Pointer container = this->GetContainer();

  if (container != 0)
  {
    container->FindSashes(LayoutPart::Pointer(this), sashes);
  }
}

ILayoutContainer::ChildrenType DetachedPlaceHolder::GetChildren() const
{
  return children;
}

void DetachedPlaceHolder::ResizeChild(LayoutPart::Pointer childThatChanged)
{
  PartPlaceholder::ResizeChild(childThatChanged);
}


}
