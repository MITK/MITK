#include "UndoModel.h"

//##ModelId=3F01770A018E
void mitk::UndoModel::SwapOperations(mitk::OperationEvent *operationEvent)
{
	operationEvent->SwapOperations();
}
