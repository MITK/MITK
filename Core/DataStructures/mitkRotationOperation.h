#ifndef MITKROTATIONOPERATION_H_HEADER_INCLUDED_C188C29F
#define MITKROTATIONOPERATION_H_HEADER_INCLUDED_C188C29F

#include "mitkOperation.h"
#include "mitkVector.h"

namespace mitk {


class RotationOperation : public Operation
{
public:
  RotationOperation(OperationType operationType, Point3D pointOfRotation, Vector3D vectorOfRotation, ScalarType angleOfRotation);
  virtual ~RotationOperation(void);
  virtual ScalarType GetAngleOfRotation();
  virtual const Point3D GetCenterOfRotation();
  virtual const Vector3D GetVectorOfRotation();

protected:
  ScalarType m_AngleOfRotation;
  Point3D m_PointOfRotation;
  Vector3D m_VectorOfRotation;
};

} // namespace mitk

#endif /* MITKROTATIONOPERATION_H_HEADER_INCLUDED_C188C29F */
