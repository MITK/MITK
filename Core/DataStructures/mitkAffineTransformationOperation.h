#ifndef MITKAFFINETRANSFORMATIONOPERATION_H_HEADER_INCLUDED
#define MITKAFFINETRANSFORMATIONOPERATION_H_HEADER_INCLUDED

#include "mitkPointOperation.h"
#include "mitkVector.h"

namespace mitk {

//##Documentation
//## @brief Operation, that holds everything necessary for an affine operation.
//##
//## @ingroup Undo
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

#endif /* MITKAFFINETRANSFORMATIONOPERATION_H_HEADER_INCLUDED */

