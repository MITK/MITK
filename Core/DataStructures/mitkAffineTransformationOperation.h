#ifndef MITKAFFINETRANSFORMATIONOPERATION_H_HEADER_INCLUDED_C188C29F
#define MITKAFFINETRANSFORMATIONOPERATION_H_HEADER_INCLUDED_C188C29F

#include "mitkPointOperation.h"


namespace mitk {


class AffineTransformationOperation : public PointOperation
{
public:
  AffineTransformationOperation(OperationType operationType, ITKPoint3D point, double angle, int index);
  virtual ~AffineTransformationOperation(void);
  double GetAngle();
protected:
  double m_Angle;
};

} // namespace mitk

#endif /* MITKAFFINETRANSFORMATIONOPERATION_H_HEADER_INCLUDED_C188C29F */

