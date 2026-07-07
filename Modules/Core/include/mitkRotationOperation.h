/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRotationOperation_h
#define mitkRotationOperation_h

#include <mitkNumericTypes.h>
#include <mitkOperation.h>

namespace mitk
{
  /**
   * \brief Operation that holds everything necessary for a rotation operation on mitk::BaseData.
   *
   * \sa Operation
   * \ingroup Undo
   */
  class MITKCORE_EXPORT RotationOperation : public Operation
  {
  public:
    /**
     * \brief Constructor to create a rotation operation.
     *
     * \param operationType this has to be set to OpROTATE.
     * \param pointOfRotation anchor point for rotation.
     * \param vectorOfRotation axis for rotation.
     * \param angleOfRotation angle for rotation in degrees.
     */
    RotationOperation(OperationType operationType,
                      Point3D pointOfRotation,
                      Vector3D vectorOfRotation,
                      ScalarType angleOfRotation);

    /** \brief Destructor. */
    ~RotationOperation(void) override;

    /**
     * \brief Returns the rotation angle.
     *
     * \return Angle in degrees.
     */
    virtual ScalarType GetAngleOfRotation();

    /**
     * \brief Returns the anchor point of rotation.
     *
     * \return The anchor point to rotate the base data around.
     */
    virtual const Point3D GetCenterOfRotation();

    /**
     * \brief Returns the rotation axis.
     *
     * \return Rotation axis as vector.
     */
    virtual const Vector3D GetVectorOfRotation();

  protected:
    ScalarType m_AngleOfRotation;
    Point3D m_PointOfRotation;
    Vector3D m_VectorOfRotation;
  };

} // namespace mitk

#endif
