/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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

