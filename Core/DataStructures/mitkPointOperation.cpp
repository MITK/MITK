#include "mitkPointOperation.h"

mitk::PointOperation::PointOperation(OperationType operationType,
									ITKPoint3D point, int index)
: mitk::Operation(operationType), m_Point(point), m_Index(index)
{}

mitk::PointOperation::~PointOperation()
{
}

mitk::ITKPoint3D mitk::PointOperation::GetPoint()
{
	return m_Point;
}

int mitk::PointOperation::GetIndex()
{
	return m_Index;
}