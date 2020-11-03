/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryEditorHistory.h"

#include "berryIEditorInput.h"
#include "berryIEditorDescriptor.h"
#include "berryIMemento.h"

#include "berryStatus.h"

#include "berryEditorHistoryItem.h"
#include "berryWorkbenchConstants.h"

namespace berry {

const int EditorHistory::MAX_SIZE = 15;

EditorHistory::EditorHistory()
{
}

void EditorHistory::Add(const SmartPointer<IEditorInput>& input, const SmartPointer<IEditorDescriptor>& desc)
{
  if (input.IsNotNull() && input->Exists())
  {
    EditorHistoryItem::Pointer item(new EditorHistoryItem(input, desc));
    Add(item, 0);
  }
}

QList<SmartPointer<EditorHistoryItem> > EditorHistory::GetItems()
{
  Refresh();
  return fifoList;
}

void EditorHistory::Refresh()
{
  auto iter = fifoList.begin();
  while (iter != fifoList.end())
  {
    if ((*iter)->IsRestored())
    {
      IEditorInput::Pointer input = (*iter)->GetInput();
      if (input.IsNotNull() && !input->Exists())
      {
        iter = fifoList.erase(iter);
        continue;
      }
    }
    ++iter;
  }
}

void EditorHistory::Remove(const SmartPointer<EditorHistoryItem>& item)
{
  fifoList.removeAll(item);
}

void EditorHistory::Remove(const SmartPointer<IEditorInput>& input)
{
  if (input.IsNull())
  {
    return;
  }
  auto iter = fifoList.begin();
  while (iter != fifoList.end())
  {
    if ((*iter)->Matches(input))
    {
      iter = fifoList.erase(iter);
    }
    else
    {
      ++iter;
    }
  }
}

SmartPointer<const IStatus> EditorHistory::RestoreState(const SmartPointer<IMemento>& memento)
{
  QList<IMemento::Pointer> mementos = memento->GetChildren(WorkbenchConstants::TAG_FILE);
  for (int i = 0; i < mementos.size(); i++)
  {
    EditorHistoryItem::Pointer item(new EditorHistoryItem(mementos[i]));
    if (!item->GetName().isEmpty() || !item->GetToolTipText().isEmpty())
    {
      this->Add(item, fifoList.size());
    }
  }
  return Status::OK_STATUS(BERRY_STATUS_LOC);
}

SmartPointer<const IStatus> EditorHistory::SaveState(const SmartPointer<IMemento>& memento) const
{
  for (auto iter = fifoList.begin(); iter != fifoList.end(); ++iter)
  {
    if ((*iter)->CanSave())
    {
      IMemento::Pointer itemMemento = memento->CreateChild(WorkbenchConstants::TAG_FILE);
      (*iter)->SaveState(itemMemento);
    }
  }
  return Status::OK_STATUS(BERRY_STATUS_LOC);
}

void EditorHistory::Add(const SmartPointer<EditorHistoryItem>& newItem, int index)
{
  // Remove the item if it already exists so that it will be put
  // at the top of the list.
  if (newItem->IsRestored())
  {
    this->Remove(newItem->GetInput());
  }

  // Remove the oldest one
  if (fifoList.size() == MAX_SIZE)
  {
    fifoList.pop_back();
  }

  // Add the new item.
  fifoList.insert(index < MAX_SIZE ? index : MAX_SIZE - 1, newItem);
}

}
