/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkScaleOperation_h
#define mitkScaleOperation_h

#include <mitkOperation.h>
#include <mitkPoint.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Operation to scale any mitk::BaseGeometry.
   *
   * The scale factor is expressed as a factor relative to the current size.
   * For doubling the size, you would use a factor of 1.0 == 100% plus.
   */
  class MITKCORE_EXPORT ScaleOperation : public Operation
  {
  public:
    /**
     * \brief Construct a ScaleOperation.
     * \param[in] operationType Must be set to OpSCALE.
     * \param[in] scaleFactor Relative scaling factor in percentage of the input geometry.
     * \param[in] scaleAnchorPoint Point to scale the object around.
     * Typically geometry->GetCenter() or geometry->GetOrigin() are suitable choices.
     */
    ScaleOperation(OperationType operationType, Point3D scaleFactor, Point3D scaleAnchorPoint);

    ~ScaleOperation() override;

    /**
     * \brief Get the scale factor.
     * \return The scale factor as a 3D point (one factor per axis).
     */
    Point3D GetScaleFactor();

    /**
     * \brief Get the anchor point for scaling.
     * \return The point around which scaling is performed.
     */
    Point3D GetScaleAnchorPoint();

  private:
    Point3D m_ScaleFactor;
    Point3D m_ScaleAnchorPoint;
  };
} // namespace mitk
#endif
