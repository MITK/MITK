#include "mitkLineOperation.h"

mitk::LineOperation::LineOperation(OperationType operationType, int cellId, int pIdA, int pIdB)
: mitk::Operation(operationType), m_CellId(cellId), m_PIdA(pIdA), m_PIdB(pIdB)
{}

mitk::LineOperation::~LineOperation()
{
}

int mitk::LineOperation::GetCellId()
{
	return m_CellId;
}

int mitk::LineOperation::GetPIdA()
{
	return m_PIdA;
}

int mitk::LineOperation::GetPIdB()
{
	return m_PIdB;
}


