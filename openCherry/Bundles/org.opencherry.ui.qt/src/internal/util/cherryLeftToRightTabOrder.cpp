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

#include "cherryLeftToRightTabOrder.h"

#include <internal/cherryWorkbenchConstants.h>

namespace cherry
{

LeftToRightTabOrder::LeftToRightTabOrder(IPresentablePartList* _list) :
  list(_list)
{

}

void LeftToRightTabOrder::Add(IPresentablePart::Pointer newPart)
{
  list->Insert(newPart, list->Size());
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

std::vector<IPresentablePart::Pointer> LeftToRightTabOrder::GetPartList()
{
  return list->GetPartList();
}

void LeftToRightTabOrder::RestoreState(IPresentationSerializer* serializer,
    IMemento::Pointer  /*savedState*/)
{
  //TODO LeftToRightTabOrder restore state
//  std::vector<IMemento::Pointer> parts = savedState->GetChildren(
//      WorkbenchConstants::TAG_PART);
//
//  for (unsigned int idx = 0; idx < parts.size(); idx++)
//  {
//    std::string id = parts[idx]->GetString(WorkbenchConstants::TAG_ID);
//
//    if (id != "")
//    {
//      IPresentablePart::Pointer part = serializer->GetPart(id);
//
//      if (part != 0)
//      {
//        this->AddInitial(part);
//      }
//    }
//  }
}

void LeftToRightTabOrder::SaveState(IPresentationSerializer* context,
    IMemento::Pointer  /*memento*/)
{
//TODO LeftToRightTabOrder save state
//  std::vector<IPresentablePart::Pointer> parts(list->GetPartList());
//
//  for (std::vector<IPresentablePart::Pointer>::iterator iter = parts.begin();
//       iter != parts.end(); ++iter)
//  {
//    IMemento::Pointer childMem = memento->CreateChild(WorkbenchConstants::TAG_PART);
//    childMem->PutString(WorkbenchConstants::TAG_ID, context->GetId(*iter));
//  }
}

}
