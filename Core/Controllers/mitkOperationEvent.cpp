#include "OperationEvent.h"

//##ModelId=3E9B07B40374
int mitk::OperationEvent::m_NextObjectEventId = 0;
//##ModelId=3E9B07B5002B
int mitk::OperationEvent::m_NextGroupEventId = 0;

//##ModelId=3E5F610D00BB
mitk::Operation* mitk::OperationEvent::GetOperation()
{
	return m_Operation;
}

//##ModelId=3E957AE700E6
mitk::OperationEvent::OperationEvent(OperationActor* destination, 
									 Operation* operation, 
									 Operation* undoOperation,
									 int objectEventId,
									 int groupEventId)
									 : m_Destination(destination), 
									 m_Operation(operation),
									 m_UndoOperation(undoOperation),
									 m_ObjectEventId(objectEventId),
									 m_GroupEventId(groupEventId),
									 m_Swaped(false)
{}

/**
 *swaps the Undo and Redo- operation and changes m_Swaped
 */
//##ModelId=3E957C1102E3
void mitk::OperationEvent::swapOperations()
{
	Operation *tempOperation = m_Operation;
	m_Operation = m_UndoOperation;
	m_UndoOperation = tempOperation;
	if (m_Swaped)
		m_Swaped = false;
	else 
		m_Swaped = true;
}

//##ModelId=3E9B07B501A7
int mitk::OperationEvent::GenerateObjectEventId()
{
	return m_NextObjectEventId++;
}

//##ModelId=3E9B07B50220
int mitk::OperationEvent::GenerateGroupEventId()
{
	return m_NextGroupEventId++;
}

//##ModelId=3E9B07B502AC
mitk::OperationActor* mitk::OperationEvent::GetDestination() 
{
	return m_Destination;
}

