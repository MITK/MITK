#include "LimitedLinearUndo.h"

//##ModelId=3E5F5D8C00B2
bool mitk::LimitedLinearUndo::SetOperationEvent(OperationEvent* operationEvent)
{
	if (operationEvent == NULL)
		return false;
	
int size = m_UndoList.size();//for debugging
	
	m_UndoList.push_back(operationEvent);
	
size = m_UndoList.size();//for debugging
	
	return true;
}

//##ModelId=3E5F5D8C00C6
bool mitk::LimitedLinearUndo::Undo(bool fine)
{
	if(m_UndoList.size()<1) return false;

	if (fine == true)//undo all ObjectEventId
	{
		int CurrObjectEventId = (m_UndoList.back())->GetCurrObjectEventId();
		do
		{
			//get the last operation from the static undo-list
			OperationEvent* operationEvent = m_UndoList.back();
			this->SwapOperations(operationEvent);
			operationEvent->GetDestination()->ExecuteOperation(operationEvent->GetOperation());
			
			m_RedoList.push_back(operationEvent);//set in redo //!!!TODO:store the param fine
			m_UndoList.pop_back();//delete last operation from undo-list
		} while ((m_UndoList.back())->GetCurrObjectEventId() == CurrObjectEventId);
	}
	else //fine==false so undo all GroupEventId
	{
		int CurrGroupEventId = (m_UndoList.back())->GetCurrGroupEventId();
		do
		{
			//get the last operation from the static undo-list
			OperationEvent* operationEvent = m_UndoList.back();
			this->SwapOperations(operationEvent);
			operationEvent->GetDestination()->ExecuteOperation(operationEvent->GetOperation());

			m_RedoList.push_back(operationEvent);//set in redo//!!!TODO:store the param fine
			m_UndoList.pop_back();//delete last operation from undo-list
		} while ((m_UndoList.back())->GetCurrGroupEventId() == CurrGroupEventId);
	}

	return true;
}

//##ModelId=3E5F5D8C00DA
bool mitk::LimitedLinearUndo::Redo()
{
	//ToDo: Object or GroupEventId!!!
	if(m_RedoList.size()<=0) return false;

	OperationEvent* operationEvent = m_RedoList.back();

	this->SwapOperations(operationEvent);
	operationEvent->GetDestination()->ExecuteOperation(operationEvent->GetOperation());

	m_UndoList.push_back(operationEvent);
	m_RedoList.pop_back();

	return true;
}
