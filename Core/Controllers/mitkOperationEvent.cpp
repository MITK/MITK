#include "OperationEvent.h"

//##ModelId=3E5F610D00BB
mitk::Operation mitk::OperationEvent::GetOperation()
{
	return *m_Operation;
}

//##ModelId=3E5F6123006D
mitk::Operation mitk::OperationEvent::GetUndoOperation()
{
	return *m_UndoOperation;
}

//##ModelId=3E957AE700E6
mitk::OperationEvent::OperationEvent(OperationActor destination, Operation* operation, Operation* undoOperation, int objectEventId, int groupEventId)
{
}

//##ModelId=3E957C1102E3
void mitk::OperationEvent::swapOperations()
{
}

