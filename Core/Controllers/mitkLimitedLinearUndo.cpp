/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkLimitedLinearUndo.h"

//##ModelId=3E5F5D8C00B2
bool mitk::LimitedLinearUndo::SetOperationEvent(OperationEvent* operationEvent)
{
	if (operationEvent == NULL)
		return false;

	//clear the redolist, if a new operation is saved
	if ( ! (this->RedoListEmpty() ))
		this->ClearRedoList();

	m_UndoList.push_back(operationEvent);
	
	return true;
}

bool mitk::LimitedLinearUndo::Undo()
{
	return Undo(false);
}

//##ModelId=3F0451960156
bool mitk::LimitedLinearUndo::Undo(bool fine)
{
	if(m_UndoList.size()<1) return false;

	if (fine == true)//undo all ObjectEventId
	{
		int undoObjectEventId = (m_UndoList.back())->GetObjectEventId();
		do
		{
			//get the last operation from the static undo-list
			OperationEvent* operationEvent = m_UndoList.back();
			this->SwapOperations(operationEvent);
			operationEvent->GetDestination()->ExecuteOperation(operationEvent->GetOperation());
			
			m_RedoList.push_back(operationEvent);//set in redo 
			m_UndoList.pop_back();//delete last operation from undo-list
      if (m_UndoList.empty())
      {
        break;
      }
		} while ((m_UndoList.back())->GetObjectEventId() == undoObjectEventId);
	}
	else //fine==false so undo all GroupEventId
	{
		int undoGroupEventId = (m_UndoList.back())->GetGroupEventId();
		do
		{
			//get the last operation from the static undo-list
			OperationEvent* operationEvent = m_UndoList.back();
			this->SwapOperations(operationEvent);
			operationEvent->GetDestination()->ExecuteOperation(operationEvent->GetOperation());

			m_RedoList.push_back(operationEvent);//set in redo//
			m_UndoList.pop_back();//delete last operation from undo-list
            if (m_UndoList.empty())
            {
                break;
            }
		} while ((m_UndoList.back())->GetGroupEventId() == undoGroupEventId);
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
	if(m_RedoList.size()<1) return false;

	if (fine == true)//undo all ObjectEventId
	{
		int redoObjectEventId = (m_RedoList.back())->GetObjectEventId();
		do
		{
			//get the last operation from the static undo-list
			OperationEvent* operationEvent = m_RedoList.back();
			this->SwapOperations(operationEvent);
			operationEvent->GetDestination()->ExecuteOperation(operationEvent->GetOperation());
			
			m_UndoList.push_back(operationEvent);
			m_RedoList.pop_back();
            if (m_RedoList.empty())
            {
                break;
            }
		} while ((m_RedoList.back())->GetObjectEventId() == redoObjectEventId);
	}
	else //fine==false so undo all GroupEventId
	{
		int redoGroupEventId = (m_RedoList.back())->GetGroupEventId();
		do
		{
			//get the last operation from the static undo-list
			OperationEvent* operationEvent = m_RedoList.back();
			this->SwapOperations(operationEvent);
			operationEvent->GetDestination()->ExecuteOperation(operationEvent->GetOperation());

			m_UndoList.push_back(operationEvent);//set in redo
			m_RedoList.pop_back();
            if (m_RedoList.empty())
            {
                break;
            }
		} while ((m_RedoList.back())->GetGroupEventId() == redoGroupEventId);
	}
	return true;
}
//##ModelId=3F04519601A4
void mitk::LimitedLinearUndo::Clear()
{
  m_UndoList.clear();
  m_RedoList.clear();
}

//##ModelId=3F04519601B5
void mitk::LimitedLinearUndo::ClearRedoList()
{
  m_RedoList.clear();
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
  UndoContainerRevIter iter;
  for (iter = m_UndoList.rbegin(); iter != m_UndoList.rend(); ++iter){
    if ( ( (*iter)->GetOperation()->GetOperationType() == opType ) && 
         ((*iter)->GetDestination() == destination) )
      return (*iter);
  }
  return NULL;
}

