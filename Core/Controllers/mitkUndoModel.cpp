#include "UndoModel.h"

//##ModelId=3E95950F02B1
mitk::UndoModel::UndoModel()
{
}

//##ModelId=3E5F5C6C00DF
bool mitk::UndoModel::SetOperationEvent(OperationEvent* operationEvent)
{
	return true;
}

//##ModelId=3E5F5C6C00F3
bool mitk::UndoModel::Undo()
{
	return true;
}

//##ModelId=3E5F5C6C00FE
bool mitk::UndoModel::Redo()
{
	return true;
}