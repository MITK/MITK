#include "mitkLineOperation.h"

mitk::LineOperation::LineOperation(OperationType operationType,
                                   int cellId, int pIdA, int pIdB, mitk::ITKPoint3D vector)
: mitk::Operation(operationType), m_CellId(cellId), m_PIdA(pIdA), m_PIdB(pIdB), m_Vector(vector)
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

mitk::ITKPoint3D mitk::LineOperation::GetVector()
{
	return m_Vector;
}
