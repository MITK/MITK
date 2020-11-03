/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryDetachedPlaceHolder.h"

#include "berryILayoutContainer.h"
#include "berryPartPlaceholder.h"
#include "berryWorkbenchConstants.h"

namespace berry
{

DetachedPlaceHolder::DetachedPlaceHolder(const QString& id,
    const QRect& b) :
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

QRect DetachedPlaceHolder::GetBounds()
{
  return bounds;
}

QList<LayoutPart::Pointer> DetachedPlaceHolder::GetChildren()
{
  return children;
}

void DetachedPlaceHolder::Remove(LayoutPart::Pointer part)
{
  children.removeAll(part);
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

  bounds = QRect(x, y, width, height);

  // Restore the placeholders.
  QList<IMemento::Pointer> childrenMem(memento
         ->GetChildren(WorkbenchConstants::TAG_VIEW));
  for (int i = 0; i < childrenMem.size(); i++)
  {
     QString id;
     childrenMem[i]->GetString(WorkbenchConstants::TAG_ID, id);
     PartPlaceholder::Pointer holder(new PartPlaceholder(id));
     holder->SetContainer(ILayoutContainer::Pointer(this));
     children.push_back(holder);
  }
}

void DetachedPlaceHolder::SaveState(IMemento::Pointer memento)
{
  // Save the bounds.
  memento->PutInteger(WorkbenchConstants::TAG_X, bounds.x());
  memento->PutInteger(WorkbenchConstants::TAG_Y, bounds.y());
  memento->PutInteger(WorkbenchConstants::TAG_WIDTH, bounds.width());
  memento->PutInteger(WorkbenchConstants::TAG_HEIGHT, bounds.height());

  // Save the views.
  for (QList<LayoutPart::Pointer>::iterator i = children.begin();
      i != children.end(); ++i)
  {
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
