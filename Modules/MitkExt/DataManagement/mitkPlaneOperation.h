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


#ifndef MITKPlaneOperation_H
#define MITKPlaneOperation_H

#include "mitkCommon.h"
#include "mitkPointOperation.h"
#include "mitkVector.h"



namespace mitk {

/**
 * @brief Operation for setting a plane (defined by its origin and normal)
 *
 * @ingroup Undo
 */
class MITK_CORE_EXPORT PlaneOperation : public PointOperation
{
public:
  PlaneOperation( OperationType operationType, Point3D point, Vector3D normal );

  virtual ~PlaneOperation();

  Vector3D GetNormal();

private:
  Vector3D m_Normal;

};

} //namespace mitk


#endif /* MITKPlaneOperation_H */


