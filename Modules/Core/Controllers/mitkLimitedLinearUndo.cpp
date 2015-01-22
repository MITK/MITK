/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "mitkLimitedLinearUndo.h"
#include <mitkRenderingManager.h>

mitk::LimitedLinearUndo::LimitedLinearUndo()
{
  // nothing to do
}

mitk::LimitedLinearUndo::~LimitedLinearUndo()
{
  //delete undo and redo list
  this->ClearList(&m_UndoList);
  this->ClearList(&m_RedoList);
}

void mitk::LimitedLinearUndo::ClearList(UndoContainer* list)
{
  while(!list->empty())
  {
    UndoStackItem* item = list->back();
    list->pop_back();
    delete item;
  }
}

bool mitk::LimitedLinearUndo::SetOperationEvent(UndoStackItem* stackItem)
{
  OperationEvent* operationEvent = dynamic_cast<OperationEvent*>(stackItem);
  if (!operationEvent) return false;

  // clear the redolist, if a new operation is saved
  if (!m_RedoList.empty())
  {
    this->ClearList(&m_RedoList);
    InvokeEvent( RedoEmptyEvent() );
  }

  m_UndoList.push_back(operationEvent);

  InvokeEvent( UndoNotEmptyEvent() );

  return true;
}

bool mitk::LimitedLinearUndo::Undo(bool fine)
{
  if (fine)
  {
    // undo one object event ID
    return Undo();
  }
  else
  {
    // undo one group event ID
    int oeid = FirstObjectEventIdOfCurrentGroup(m_UndoList); // get the Object Event ID of the first item with a differnt Group ID (as seen from the end of stack)
    return Undo(oeid);
  }
}

bool mitk::LimitedLinearUndo::Undo()
{
  if(m_UndoList.empty()) return false;

  int undoObjectEventId = m_UndoList.back()->GetObjectEventId();
  return Undo( undoObjectEventId );
}

bool mitk::LimitedLinearUndo::Undo(int oeid)
{
  if(m_UndoList.empty()) return false;

  bool rc = true;
  do
  {
    m_UndoList.back()->ReverseAndExecute();

    m_RedoList.push_back(m_UndoList.back());  // move to redo stack
    m_UndoList.pop_back();
    InvokeEvent( RedoNotEmptyEvent() );

    if (m_UndoList.empty())
    {
      InvokeEvent( UndoEmptyEvent() );
      rc = false;
      break;
    }
  }
  while ( m_UndoList.back()->GetObjectEventId() >= oeid );

  //Update. Check Rendering Mechanism where to request updates
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  return rc;
}

bool mitk::LimitedLinearUndo::Redo(bool)
{
  return Redo();
}

bool mitk::LimitedLinearUndo::Redo()
{
  if (m_RedoList.empty()) return false;

  int redoObjectEventId = m_RedoList.back()->GetObjectEventId();
  return Redo( redoObjectEventId );
}

bool mitk::LimitedLinearUndo::Redo(int oeid)
{
  if (m_RedoList.empty()) return false;

  do
  {
    m_RedoList.back()->ReverseAndExecute();

    m_UndoList.push_back(m_RedoList.back());
    m_RedoList.pop_back();
    InvokeEvent( UndoNotEmptyEvent() );

    if (m_RedoList.empty())
    {
      InvokeEvent( RedoEmptyEvent() );
      break;
    }
  }
  while ( m_RedoList.back()->GetObjectEventId() <= oeid );

  //Update. This should belong into the ExecuteOperation() of OperationActors, but it seems not to be used everywhere
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  return true;
}

void mitk::LimitedLinearUndo::Clear()
{
  this->ClearList(&m_UndoList);
  InvokeEvent( UndoEmptyEvent() );

  this->ClearList(&m_RedoList);
  InvokeEvent( RedoEmptyEvent() );
}

void mitk::LimitedLinearUndo::ClearRedoList()
{
  this->ClearList(&m_RedoList);
  InvokeEvent( RedoEmptyEvent() );
}

bool mitk::LimitedLinearUndo::RedoListEmpty()
{
  return m_RedoList.empty();
}

int mitk::LimitedLinearUndo::GetLastObjectEventIdInList()
{
  return m_UndoList.back()->GetObjectEventId();
}

int mitk::LimitedLinearUndo::GetLastGroupEventIdInList()
{
  return m_UndoList.back()->GetGroupEventId();
}

mitk::OperationEvent* mitk::LimitedLinearUndo::GetLastOfType(OperationActor* destination, OperationType opType)
{
  // When/where is this function needed? In CoordinateSupplier...
  for ( UndoContainerRevIter iter = m_UndoList.rbegin(); iter != m_UndoList.rend(); ++iter )
  {
    OperationEvent* opEvent = dynamic_cast<OperationEvent*>(*iter);
    if (!opEvent) continue;

    if (   opEvent->GetOperation() != NULL
        && opEvent->GetOperation()->GetOperationType() == opType
        && opEvent->IsValid()
        && opEvent->GetDestination() == destination )
      return opEvent;
  }

  return NULL;
}

int mitk::LimitedLinearUndo::FirstObjectEventIdOfCurrentGroup(mitk::LimitedLinearUndo::UndoContainer& stack)
{
  int currentGroupEventId = stack.back()->GetGroupEventId();
  int firstObjectEventId = -1;

  for ( UndoContainerRevIter iter = stack.rbegin(); iter != stack.rend(); ++iter )
  {
    if ( (*iter)->GetGroupEventId() == currentGroupEventId )
    {
      firstObjectEventId = (*iter)->GetObjectEventId();
    }
    else break;
  }

  return firstObjectEventId;
}

