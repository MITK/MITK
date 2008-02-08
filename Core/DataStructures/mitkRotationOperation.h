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


#ifndef MITKROTATIONOPERATION_H_HEADER_INCLUDED
#define MITKROTATIONOPERATION_H_HEADER_INCLUDED

#include "mitkOperation.h"
#include "mitkVector.h"

namespace mitk {

//##Documentation
//## @brief Operation, that holds everything necessary for an rotation operation 
//##
//## @ingroup Undo
class MITK_CORE_EXPORT RotationOperation : public Operation
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

#endif /* MITKROTATIONOPERATION_H_HEADER_INCLUDED */
