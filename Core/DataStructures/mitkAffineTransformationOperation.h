#ifndef MITKAFFINETRANSFORMATIONOPERATION_H_HEADER_INCLUDED_C188C29F
#define MITKAFFINETRANSFORMATIONOPERATION_H_HEADER_INCLUDED_C188C29F

#include "mitkPointOperation.h"
#include "mitkVector.h"

namespace mitk {


class AffineTransformationOperation : public PointOperation
{
public:
  AffineTransformationOperation(OperationType operationType, Point3D point, ScalarType angle, int index);
  virtual ~AffineTransformationOperation(void);
  ScalarType GetAngle();
protected:
  ScalarType m_Angle;
};

} // namespace mitk

#endif /* MITKAFFINETRANSFORMATIONOPERATION_H_HEADER_INCLUDED_C188C29F */

