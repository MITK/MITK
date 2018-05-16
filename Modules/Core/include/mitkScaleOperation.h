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

#include "mitkOperation.h"
#include "mitkPoint.h"
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * @brief The ScaleOperation is an operation to scale any mitk::BaseGeometry.
   *
   * The scale factor is expressed as a factor relative to the current size.
   * For doubling the size, you would use a factor of 1.0 == 100% plus.
   */
  class MITKCORE_EXPORT ScaleOperation : public Operation
  {
  public:
    /**
     * @brief ScaleOperation constructor.
     * @param operationType this has to be set to OpSCALE.
     * @param scaleFactor Relative scaling factor in percentage of the input geometry.
     * @param scaleAnchorPoint Point to scale the object around.
     * Typically geometry->GetCenter() or geometry->GetOrigin() are nice.
     */
    ScaleOperation(OperationType operationType, Point3D scaleFactor, Point3D scaleAnchorPoint);

    /**
     * @brief ~ScaleOperation destructor.
     */
    ~ScaleOperation() override;

    /**
     * @brief GetScaleFactor getter for the scale factor.
     * @return return the scale factor as 3D scalar.
     */
    Point3D GetScaleFactor();

    /**
     * @brief GetScaleAnchorPoint getter for anchor point.
     * @return The point to perform the scaling around.
     */
    Point3D GetScaleAnchorPoint();

  private:
    Point3D m_ScaleFactor;
    Point3D m_ScaleAnchorPoint;
  };
} // namespace mitk
#endif /* mitkScaleOperation_h */
