/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkLimitedLinearUndo.h"

mitk::LimitedLinearUndo::LimitedLinearUndo()
{
  // nothing to do
}

mitk::LimitedLinearUndo::~LimitedLinearUndo()
{
  m_UndoList.clear(); 
  m_RedoList.clear(); 
}


//##ModelId=3E5F5D8C00B2
bool mitk::LimitedLinearUndo::SetOperationEvent(UndoStackItem* stackItem)
{
  OperationEvent* operationEvent = dynamic_cast<OperationEvent*>(stackItem); 
  if (!operationEvent) return false;
  
  // clear the redolist, if a new operation is saved
  if (!m_RedoList.empty())
  {
    m_RedoList.clear();
    InvokeEvent( RedoEmptyEvent() );
  }

  m_UndoList.push_back(operationEvent);
  
  InvokeEvent( UndoNotEmptyEvent() );
  
  return true;
}

bool mitk::LimitedLinearUndo::Undo()
{
  return Undo(false);
}

//##ModelId=3F0451960156
bool mitk::LimitedLinearUndo::Undo(bool fine)
{
  if(m_UndoList.empty()) return false;

  if (fine) // undo all ObjectEventId
  {
    int undoObjectEventId = m_UndoList.back()->GetObjectEventId();
    do
    {
      // get the last operation from the static undo-list
      OperationEvent* operationEvent = dynamic_cast<OperationEvent*>(m_UndoList.back());
      if (operationEvent)
      {
        SwapOperations(operationEvent);
        operationEvent->GetDestination()->ExecuteOperation( operationEvent->GetOperation() );
      
        m_RedoList.push_back(operationEvent);  // set in redo 
        InvokeEvent( RedoNotEmptyEvent() );
        m_UndoList.pop_back();                 // delete last operation from undo-list
      }
      else
      {
        // everything except OperationEvents: just put into the other stack
        m_RedoList.push_back(m_UndoList.back());
        m_UndoList.pop_back();
      }

      if (m_UndoList.empty()) 
      {
        InvokeEvent( UndoEmptyEvent() );
        break;
      }
    } 
    while ( m_UndoList.back()->GetObjectEventId() == undoObjectEventId );
  }
  else // fine == false, so undo all GroupEventId
  {
    int undoGroupEventId = m_UndoList.back()->GetGroupEventId();
    do
    {
      // get the last operation from the static undo-list
      OperationEvent* operationEvent = dynamic_cast<OperationEvent*>(m_UndoList.back());
      if (operationEvent)
      {
        SwapOperations(operationEvent);
        operationEvent->GetDestination()->ExecuteOperation( operationEvent->GetOperation() );

        m_RedoList.push_back(operationEvent);  // set in redo
        InvokeEvent( RedoNotEmptyEvent() );
        m_UndoList.pop_back();                 // delete last operation from undo-list
      }
      else
      {
        // everything except OperationEvents: just put into the other stack
        m_RedoList.push_back(m_UndoList.back());
        m_UndoList.pop_back();
      }
      
      if (m_UndoList.empty()) 
      {
        InvokeEvent( UndoEmptyEvent() );
        break;
      }
    } 
    while ( m_UndoList.back()->GetGroupEventId() == undoGroupEventId );
  }

  return true;
}

//##ModelId=3E5F5D8C00DA
bool mitk::LimitedLinearUndo::Redo()
{
  return Redo(false);
}

//##ModelId=3F0451960176
bool mitk::LimitedLinearUndo::Redo(bool fine)
{
  if (m_RedoList.empty()) return false;

  if (fine) // undo all ObjectEventId
  {
    int redoObjectEventId = m_RedoList.back()->GetObjectEventId();
    do
    {
      // get the last operation from the static undo-list
      OperationEvent* operationEvent = dynamic_cast<OperationEvent*>(m_RedoList.back());
      if (operationEvent)
      {
        SwapOperations(operationEvent);
        operationEvent->GetDestination()->ExecuteOperation( operationEvent->GetOperation() );
      
        m_UndoList.push_back(operationEvent);
        InvokeEvent( UndoNotEmptyEvent() );
        m_RedoList.pop_back();
      }
      else
      {
        // everything except OperationEvents: just put into the other stack
        m_UndoList.push_back(m_RedoList.back());
        m_RedoList.pop_back();
      }

      if (m_RedoList.empty()) 
      {
        InvokeEvent( RedoEmptyEvent() );
        break;
      }
    } 
    while ( m_RedoList.back()->GetObjectEventId() == redoObjectEventId );
  }
  else // fine == false so undo all GroupEventId
  {
    int redoGroupEventId = (m_RedoList.back())->GetGroupEventId();
    do
    {
      // get the last operation from the static undo-list
      OperationEvent* operationEvent = dynamic_cast<OperationEvent*>(m_RedoList.back());
      if (operationEvent)
      {
        SwapOperations(operationEvent);
        operationEvent->GetDestination()->ExecuteOperation( operationEvent->GetOperation() );

        m_UndoList.push_back(operationEvent); // set in redo
        InvokeEvent( UndoNotEmptyEvent() );
        m_RedoList.pop_back();
      }
      else
      {
        // everything except OperationEvents: just put into the other stack
        m_UndoList.push_back(m_RedoList.back());
        m_RedoList.pop_back();
      }
      if (m_RedoList.empty())
      {
        InvokeEvent( RedoEmptyEvent() );
        break;
      }
    } 
    while ( m_RedoList.back()->GetGroupEventId() == redoGroupEventId );
  }
  return true;
}

//##ModelId=3F04519601A4
void mitk::LimitedLinearUndo::Clear()
{
  m_UndoList.clear();
  InvokeEvent( UndoEmptyEvent() );
  
  m_RedoList.clear();
  InvokeEvent( RedoEmptyEvent() );
}

//##ModelId=3F04519601B5
void mitk::LimitedLinearUndo::ClearRedoList()
{
  m_RedoList.clear();
  InvokeEvent( RedoEmptyEvent() );
}

//##ModelId=3F04519601D3
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
  for ( UndoContainerRevIter iter = m_UndoList.rbegin(); iter != m_UndoList.rend(); ++iter )
  {
    OperationEvent* opEvent = dynamic_cast<OperationEvent*>(*iter);
    if (!opEvent) continue;

    if (   opEvent->GetOperation()->GetOperationType() == opType
        && opEvent->GetDestination() == destination ) 
      return opEvent;
  }

  return NULL;
}

