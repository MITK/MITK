#include "mitkLineOperation.h"

mitk::LineOperation::LineOperation(OperationType operationType, int cellId, Vector3D vector, int pIdA, int pIdB, int id)
: mitk::CellOperation(operationType, cellId, vector), m_PIdA(pIdA), m_PIdB(pIdB), m_Id(id)
{}
mitk::LineOperation::LineOperation(OperationType operationType, int cellId, int pIdA, int pIdB, int id)
: mitk::CellOperation(operationType, cellId), m_PIdA(pIdA), m_PIdB(pIdB), m_Id(id)
{}

int mitk::LineOperation::GetPIdA()
{
	return m_PIdA;
}

int mitk::LineOperation::GetPIdB()
{
	return m_PIdB;
}

int mitk::LineOperation::GetId()
{
	return m_Id;
}


