/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef MITKAFFINETRANSFORMATIONOPERATION_H_HEADER_INCLUDED
#define MITKAFFINETRANSFORMATIONOPERATION_H_HEADER_INCLUDED

#include "mitkPointOperation.h"
#include "MitkExtExports.h"
#include "mitkVector.h"

namespace mitk {

//##Documentation
//## @brief Operation, that holds everything necessary for an affine operation.
//##
//## @ingroup Undo
class MitkExt_EXPORT AffineTransformationOperation : public PointOperation
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


