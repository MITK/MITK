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


#ifndef mitkScaleOperation_h
#define mitkScaleOperation_h

#include <MitkCoreExports.h>
#include "mitkOperation.h"
#include "mitkPoint.h"


namespace mitk {

/**
 * @brief The ScaleOperation class
 */
class MITKCORE_EXPORT ScaleOperation : public Operation
{
  public:

  ScaleOperation(OperationType operationType, Point3D scaleFactor, Point3D scaleAnchorPoint);

  virtual ~ScaleOperation();

  Point3D GetScaleFactor();
  Point3D GetScaleAnchorPoint();

  private:
  Point3D m_ScaleFactor;
  Point3D m_ScaleAnchorPoint;

};
}//namespace mitk
#endif /* mitkScaleOperation_h */
