#include "UndoModel.h"

//##ModelId=3E95950F02B1
//mitk::UndoModel::UndoModel(){}

/**
 * sends the Operation to the destination and swaps the operations
 */
//##ModelId=3E9B07B601A9
void mitk::UndoModel::Execute( OperationEvent* operationEvent ) 
{
	operationEvent->swapOperations();
	operationEvent->GetDestination()->SetOperation( operationEvent->GetOperation() );
}
