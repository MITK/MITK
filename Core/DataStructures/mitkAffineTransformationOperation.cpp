#include "mitkAffineTransformationOperation.h"

mitk::AffineTransformationOperation::AffineTransformationOperation(OperationType operationType,ITKPoint3D point, double angle, int index)
  : mitk::PointOperation(operationType, point, index), m_Angle(angle)
{
}

mitk::AffineTransformationOperation::~AffineTransformationOperation(void)
{
}

double mitk::AffineTransformationOperation::GetAngle()
{
  return m_Angle;
}