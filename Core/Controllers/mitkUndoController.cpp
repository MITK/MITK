#include "UndoController.h"

//##ModelId=3E5F543402C5
bool mitk::UndoController::SetOperationEvent(OperationEvent* operationEvent)
{
	m_UndoModel.SetOperationEvent(operationEvent);
	return true;
}

//##ModelId=3E5F55C80207
bool mitk::UndoController::Undo()
{
	return m_UndoModel.Undo();
}

//##ModelId=3E5F55E6003E
bool mitk::UndoController::Redo()
{
	return m_UndoModel.Redo();
}

//##ModelId=3E5F56EB017A
bool mitk::UndoController::SetUndoType(UndoType undoType)
{
	//setting the undoType...
	return true;
}

