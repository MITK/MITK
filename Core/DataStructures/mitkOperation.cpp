#include "Operation.h"

//##ModelId=3E7830E70054
mitk::Operation::Operation(mitk::OperationType operationType)
: m_OperationType(operationType)
{
}

//##ModelId=3F01770A0007
mitk::Operation::~Operation()
{
}

//##ModelId=3E7831B600CA
mitk::OperationType mitk::Operation::GetOperationType()
{
	return m_OperationType;
}