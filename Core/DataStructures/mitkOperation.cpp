#include "Operation.h"

//##ModelId=3E7830E70054
mitk::Operation::Operation(mitk::OperationType operationType, int execId)
:m_OperationType(operationType), m_ExecutionId(execId)
{}

//##ModelId=3E7831B600CA
mitk::OperationType mitk::Operation::GetOperationType()
{
	return m_OperationType;
}

//##ModelId=3EDDD7640079
int mitk::Operation::GetExecutionId()
{
	return m_ExecutionId;
}
