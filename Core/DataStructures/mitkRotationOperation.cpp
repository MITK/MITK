#include "mitkRotationOperation.h"

mitk::RotationOperation::RotationOperation(OperationType operationType, ITKPoint3D pointOfRotation, ITKVector3D vectorOfRotation, ScalarType angleOfRotation)
  : mitk::Operation(operationType)
  , m_AngleOfRotation(angleOfRotation), m_PointOfRotation(pointOfRotation), m_VectorOfRotation(vectorOfRotation)
{
}

mitk::RotationOperation::~RotationOperation(void)
{
}

mitk::ScalarType mitk::RotationOperation::GetAngleOfRotation()
{     
  return this->m_AngleOfRotation; 
}

const mitk::ITKPoint3D mitk::RotationOperation::GetCenterOfRotation()
{     
  return this->m_PointOfRotation; 
}
const mitk::ITKVector3D mitk::RotationOperation::GetVectorOfRotation()
{     
  return this->m_VectorOfRotation; 
}
