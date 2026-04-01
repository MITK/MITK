/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkApplyTransformMatrixOperation_h
#define mitkApplyTransformMatrixOperation_h

#include <mitkCommon.h>
#include <mitkPointOperation.h>

#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  /**
   * \brief Operation that applies a VTK 4x4 transform matrix to a geometry.
   *
   * Encapsulates a vtkMatrix4x4 transformation and a reference point used for
   * realigning a plane stack. Typically dispatched via the undo/redo framework.
   *
   * \sa Operation
   * \sa BaseGeometry
   */
  class MITKCORE_EXPORT ApplyTransformMatrixOperation : public Operation
  {
  public:
    /**
     * \brief Construct an operation that applies a transform matrix.
     *
     * \param[in] operationType The type of operation (see mitkOperation.h).
     * \param[in] matrix The 4x4 VTK transformation matrix to apply.
     * \param[in] refPoint The reference point for realigning the plane stack.
     */
    ApplyTransformMatrixOperation(OperationType operationType,
                                  vtkSmartPointer<vtkMatrix4x4> matrix,
                                  mitk::Point3D refPoint);

    ~ApplyTransformMatrixOperation() override;

    /**
     * \brief Get the transformation matrix.
     * \return The 4x4 VTK matrix.
     */
    vtkSmartPointer<vtkMatrix4x4> GetMatrix();

    /**
     * \brief Get the reference point.
     * \return The 3D reference point used for plane stack realignment.
     */
    mitk::Point3D GetReferencePoint();

  private:
    vtkSmartPointer<vtkMatrix4x4> m_vtkMatrix;
    mitk::Point3D m_referencePoint;
  };
} // namespace mitk
#endif
