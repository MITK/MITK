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
	OperationEvent* operationEvent = m_UndoList.back();
	
	//Operation *operation = operationEvent->GetOperation();
	//OperationType operationType = operation.GetOperationType();
	//if (operationType == ROI)
	
	Execute( operationEvent );
	
	m_RedoList.push_back(operationEvent);
	m_UndoList.pop_back();
	return true;
}

//##ModelId=3E5F5D8C00DA
bool mitk::LimitedLinearUndo::Redo()
{
	OperationEvent* operationEvent = m_RedoList.back();
	
	//Operation *operation = operationEvent->GetOperation();
	//OperationType operationType = operation.GetOperationType();
	//if (operationType == ROI)...
	
	Execute( operationEvent );
	
	m_UndoList.push_back(operationEvent);
	m_RedoList.pop_back();
	return true;
}

