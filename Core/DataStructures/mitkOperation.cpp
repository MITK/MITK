#include "Operation.h"

//##ModelId=3E7830E70054
mitk::Operation::Operation(OperationType operationType)
{
	m_OperationType = operationType;
}

//##ModelId=3E7831B600CA
OperationType mitk::Operation::GetOperationType()
{
	return m_OperationType;
}

//##ModelId=3E7F4D7D01AE
void mitk::Operation::Execute()
{
	//virtual method; implement the operation here!
}

