#include "mitkModeOperation.h"
#include <mitkOperation.h>

mitk::ModeOperation::ModeOperation(mitk::OperationType operationType, mitk::ModeOperation::ModeType mode)
: Operation(operationType), m_Mode(mode)
{
}

mitk::ModeOperation::~ModeOperation()
{
}

mitk::ModeOperation::ModeType mitk::ModeOperation::GetMode()
{
	return m_Mode;
}
