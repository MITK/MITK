/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryLeftToRightTabOrder.h"

#include <internal/berryWorkbenchConstants.h>

namespace berry
{

LeftToRightTabOrder::LeftToRightTabOrder(IPresentablePartList* _list) :
  list(_list)
{

}

void LeftToRightTabOrder::Add(IPresentablePart::Pointer newPart)
{
  list->Insert(newPart, static_cast<int>(list->Size()));
}

void LeftToRightTabOrder::AddInitial(IPresentablePart::Pointer newPart)
{
  this->Add(newPart);
}

void LeftToRightTabOrder::Insert(IPresentablePart::Pointer newPart, int index)
{
  list->Insert(newPart, index);
}

void LeftToRightTabOrder::Remove(IPresentablePart::Pointer removed)
{
  list->Remove(removed);
}

void LeftToRightTabOrder::Select(IPresentablePart::Pointer selected)
{
  list->Select(selected);
}

void LeftToRightTabOrder::Move(IPresentablePart::Pointer toMove, int newIndex)
{
  list->Move(toMove, newIndex);
}

QList<IPresentablePart::Pointer> LeftToRightTabOrder::GetPartList()
{
  return list->GetPartList();
}

void LeftToRightTabOrder::RestoreState(IPresentationSerializer* serializer,
    IMemento::Pointer savedState)
{
  QList<IMemento::Pointer> parts = savedState->GetChildren(
      WorkbenchConstants::TAG_PART);

  for (int idx = 0; idx < parts.size(); idx++)
  {
    QString id;
    parts[idx]->GetString(WorkbenchConstants::TAG_ID, id);

    if (!id.isEmpty())
    {
      IPresentablePart::Pointer part = serializer->GetPart(id);

      if (part)
      {
        this->AddInitial(part);
      }
    }
  }
}

void LeftToRightTabOrder::SaveState(IPresentationSerializer* context,
    IMemento::Pointer memento)
{
  QList<IPresentablePart::Pointer> parts(list->GetPartList());

  for (QList<IPresentablePart::Pointer>::iterator iter = parts.begin();
       iter != parts.end(); ++iter)
  {
    IMemento::Pointer childMem = memento->CreateChild(WorkbenchConstants::TAG_PART);
    childMem->PutString(WorkbenchConstants::TAG_ID, context->GetId(*iter));
  }
}

}
