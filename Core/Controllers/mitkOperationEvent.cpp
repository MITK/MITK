#include "mitkOperationEvent.h"

//##ModelId=3E9B07B40374
int mitk::OperationEvent::m_CurrObjectEventId = 0;
//##ModelId=3E9B07B5002B
int mitk::OperationEvent::m_CurrGroupEventId = 0;

//##ModelId=3E5F610D00BB
mitk::Operation* mitk::OperationEvent::GetOperation()
{
	return m_Operation;
}

//##ModelId=3E957AE700E6
mitk::OperationEvent::OperationEvent(OperationActor* destination, 
									 Operation* operation, Operation* undoOperation,
									 int objectEventId, int groupEventId)
: m_Destination(destination), m_Operation(operation), m_UndoOperation(undoOperation),
  m_ObjectEventId(objectEventId), m_GroupEventId(groupEventId), m_Swaped(false)
{
}

//##ModelId=3F0451960212
mitk::OperationEvent::~OperationEvent()
{
}

//##ModelId=3E957C1102E3
//##Documentation
//##  swaps the Undo and Redo- operation and changes m_Swaped
void mitk::OperationEvent::SwapOperations()
{
    if (m_Operation == NULL) 
        return;

	Operation *tempOperation = m_Operation;
	m_Operation = m_UndoOperation;
	m_UndoOperation = tempOperation;
	if (m_Swaped)
		m_Swaped = false;
	else 
		m_Swaped = true;
}

//##ModelId=3E9B07B50220
int mitk::OperationEvent::GetCurrGroupEventId()
{
	return m_CurrGroupEventId;
}

//##ModelId=3E9B07B501A7
int mitk::OperationEvent::GetCurrObjectEventId()
{
	return m_CurrObjectEventId;
}

//##ModelId=3EF099E90269
int mitk::OperationEvent::IncCurrGroupEventId()
{
	return ++m_CurrGroupEventId;
}

//##ModelId=3EF099E90289
int mitk::OperationEvent::IncCurrObjectEventId()
{
	return ++m_CurrObjectEventId;
}

//##ModelId=3E9B07B502AC
mitk::OperationActor* mitk::OperationEvent::GetDestination() 
{
	return m_Destination;
}

//##ModelId=3EF099E90249
int mitk::OperationEvent::GetGroupEventId()
{
	return m_GroupEventId;
}

//##ModelId=3EF099E90259
int mitk::OperationEvent::GetObjectEventId()
{
	return m_ObjectEventId;
}
