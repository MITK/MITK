#include "mitkPointOperation.h"

//##ModelId=3F0189F003A2
mitk::PointOperation::PointOperation(OperationType operationType,
									ITKPoint3D point, int index)
: mitk::Operation(operationType), m_Point(point), m_Index(index)
{}

//##ModelId=3F0189F003B2
mitk::PointOperation::~PointOperation()
{
}

//##ModelId=3F0189F003B4
mitk::ITKPoint3D mitk::PointOperation::GetPoint()
{
	return m_Point;
}

//##ModelId=3F0189F003B5
int mitk::PointOperation::GetIndex()
{
	return m_Index;
}
