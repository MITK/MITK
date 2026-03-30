/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestorePlanePositionOperation_h
#define mitkRestorePlanePositionOperation_h

#include <mitkCommon.h>
#include <mitkNumericTypes.h>
#include <mitkPointOperation.h>

namespace mitk
{
  /**
   * \brief Operation to restore a plane position defined by width, height, spacing, direction, and transform.
   *
   * This operation stores all parameters needed to restore a plane's position and orientation
   * in the scene, typically used with undo/redo mechanisms.
   *
   * \sa Operation
   * \ingroup Undo
   */
  class MITKCORE_EXPORT RestorePlanePositionOperation : public Operation
  {
  public:
    /** \brief Constructor.
     *
     * \param operationType the type of operation.
     * \param width the width of the plane.
     * \param height the height of the plane.
     * \param spacing the spacing vector of the plane geometry.
     * \param pos the position index of the slice.
     * \param direction the direction vector of the plane.
     * \param transform the affine transform defining the plane orientation.
     */
    RestorePlanePositionOperation(OperationType operationType,
                                  ScalarType width,
                                  ScalarType height,
                                  Vector3D spacing,
                                  unsigned int pos,
                                  Vector3D direction,
                                  AffineTransform3D::Pointer transform);

    /** \brief Destructor. */
    ~RestorePlanePositionOperation() override;

    /** \brief Returns the direction vector of the plane.
     *
     * \return The direction vector.
     */
    Vector3D GetDirectionVector();

    /** \brief Returns the width of the plane.
     *
     * \return The plane width.
     */
    ScalarType GetWidth();

    /** \brief Returns the height of the plane.
     *
     * \return The plane height.
     */
    ScalarType GetHeight();

    /** \brief Returns the spacing vector of the plane geometry.
     *
     * \return The spacing vector.
     */
    Vector3D GetSpacing();

    /** \brief Returns the position index of the slice.
     *
     * \return The slice position index.
     */
    unsigned int GetPos();

    /** \brief Returns the affine transform defining the plane orientation.
     *
     * \return The affine transform.
     */
    AffineTransform3D::Pointer GetTransform();

  private:
    Vector3D m_Spacing;

    Vector3D m_DirectionVector;

    ScalarType m_Width;

    ScalarType m_Height;

    unsigned int m_Pos;

    AffineTransform3D::Pointer m_Transform;
  };
} // namespace mitk
#endif
