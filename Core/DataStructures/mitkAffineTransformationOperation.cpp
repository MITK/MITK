#include "mitkAffineTransformationOperation.h"

mitk::AffineTransformationOperation::AffineTransformationOperation(OperationType operationType,ITKPoint3D point, ScalarType angle, int index)
  : mitk::PointOperation(operationType, point, index), m_Angle(angle)
{
}

mitk::AffineTransformationOperation::~AffineTransformationOperation(void)
{
}

mitk::ScalarType mitk::AffineTransformationOperation::GetAngle()
{
  return m_Angle;
}

