#include "LimitedLinearUndo.h"

//##ModelId=3E5F5D8C00B2
bool mitk::LimitedLinearUndo::SetOperationEvent(OperationEvent* operationEvent)
{
	if (operationEvent != NULL)
	{
		m_UndoList.push_back(operationEvent);
		return true;
	}
	return false;
}

//##ModelId=3E5F5D8C00C6
bool mitk::LimitedLinearUndo::Undo()
{
	//get the last operation from the static undo-list
	OperationEvent* operationEvent = m_UndoList.back();
	
	//in execute the undo and redo gets switched and the operation is send to the destination
	Execute( operationEvent );
	
	m_RedoList.push_back(operationEvent);//set in redo
	m_UndoList.pop_back();//delete last operation from undo-list
	return true;
}

//##ModelId=3E5F5D8C00DA
bool mitk::LimitedLinearUndo::Redo()
{
	OperationEvent* operationEvent = m_RedoList.back();
	
	Execute( operationEvent );
	
	m_UndoList.push_back(operationEvent);
	m_RedoList.pop_back();
	return true;
}