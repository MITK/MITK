#include "UndoModel.h"

void mitk::UndoModel::SwapOperations(mitk::OperationEvent *operationEvent)
{
	operationEvent->SwapOperations();
}
