#ifndef MITKROTATIONOPERATION_H_HEADER_INCLUDED_C188C29F
#define MITKROTATIONOPERATION_H_HEADER_INCLUDED_C188C29F

#include "mitkOperation.h"
#include "mitkVector.h"

namespace mitk {


class RotationOperation : public Operation
{
public:
  RotationOperation(OperationType operationType, ITKPoint3D pointOfRotation, ITKVector3D vectorOfRotation, ScalarType angleOfRotation);
  virtual ~RotationOperation(void);
  virtual ScalarType GetAngleOfRotation();
  virtual const ITKPoint3D GetCenterOfRotation();
  virtual const ITKVector3D GetVectorOfRotation();

protected:
  ScalarType m_AngleOfRotation;
  ITKPoint3D m_PointOfRotation;
  ITKVector3D m_VectorOfRotation;
};

} // namespace mitk

#endif /* MITKROTATIONOPERATION_H_HEADER_INCLUDED_C188C29F */
