/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlaneOperation_h
#define mitkPlaneOperation_h

#include <mitkNumericTypes.h>
#include <mitkPointOperation.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Operation for setting a plane defined by its origin and normal or axis vectors.
   *
   * A PlaneOperation can be constructed in two ways:
   * - With a point (origin) and a normal vector.
   * - With a point (origin) and two in-plane axis vectors.
   *
   * \ingroup Undo
   * \sa PointOperation
   * \sa PlaneGeometry
   */
  class MITKCORE_EXPORT PlaneOperation : public PointOperation
  {
  public:
    /**
     * \brief Construct a plane operation from a point and a normal vector.
     * \param operationType The type of operation.
     * \param point The origin of the plane.
     * \param normal The normal vector of the plane.
     */
    PlaneOperation(OperationType operationType, Point3D point, Vector3D normal);

    /**
     * \brief Construct a plane operation from a point and two axis vectors.
     * \param operationType The type of operation.
     * \param point The origin of the plane.
     * \param axisVec0 The first in-plane axis vector.
     * \param axisVec1 The second in-plane axis vector.
     */
    PlaneOperation(OperationType operationType, Point3D point, Vector3D axisVec0, Vector3D axisVec1);

    ~PlaneOperation() override;

    /** \brief Get the normal vector of the plane. Only valid if constructed with a normal. */
    Vector3D GetNormal();

    /** \brief Get the first in-plane axis vector. Only valid if AreAxisDefined() returns true. */
    Vector3D GetAxisVec0();

    /** \brief Get the second in-plane axis vector. Only valid if AreAxisDefined() returns true. */
    Vector3D GetAxisVec1();

    /**
     * \brief Check whether the axis vectors were defined in the constructor.
     * \return true if this operation was constructed with two axis vectors, false if constructed with a normal.
     */
    bool AreAxisDefined();

  private:
    Vector3D m_Normal;
    Vector3D m_AxisVec0;
    Vector3D m_AxisVec1;
    bool m_AreAxisDefined;
  };

} // namespace mitk

#endif
