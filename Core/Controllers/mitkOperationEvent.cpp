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

//##ModelId=3E5F612E0203
void mitk::OperationEvent::SetOperation(Operation *operation)
{
	m_Operation = operation;
}

//##ModelId=3E5F61390386
void mitk::OperationEvent::SetUndoOperation(Operation *operation)
{
	m_UndoOperation = operation;
}

