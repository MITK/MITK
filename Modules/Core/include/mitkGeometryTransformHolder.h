/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGeometryTransformHolder_h
#define mitkGeometryTransformHolder_h

#include <MitkCoreExports.h>
#include <mitkAffineTransform3D.h>
#include <mitkAffineTransform3D.h>
#include <mitkCommon.h>
#include <mitkMatrix.h>
#include <mitkPoint.h>
#include <mitkVector.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkTransform.h>

namespace mitk
{
  /**
   * \brief Helper class that manages transform-related variables for geometries.
   *
   * GeometryTransformHolder keeps all information about the transform (e.g. spacing, offset,
   * transformation matrices) in consistent states. It provides getters and setters to all
   * transformation-related information. Implementation is hidden and may be subject to changes.
   *
   * \sa BaseGeometry
   */
  class GeometryTransformHolder
  {
  private:
    /**
     * \brief Index to World Transform.
     *
     * Contains a transformation matrix to convert points from index coordinates to world
     * coordinates (mm). The spacing is included in this variable.
     */
    AffineTransform3D::Pointer m_IndexToWorldTransform;

    vtkMatrix4x4 *m_VtkMatrix;
    vtkMatrixToLinearTransform *m_VtkIndexToWorldTransform;

    static void CopySpacingFromTransform(const mitk::AffineTransform3D *transform, mitk::Vector3D &spacing);

    /**
     * \brief Copy the ITK transform (m_IndexToWorldTransform) to the VTK transform.
     *
     * \sa SetIndexToWorldTransform
     */
    void TransferItkToVtkTransform();

    /**
     * \brief Copy the VTK transform to the ITK transform (m_IndexToWorldTransform).
     *
     * \sa SetIndexToWorldTransform
     */
    void TransferVtkToItkTransform();

  protected:
  public:
    typedef itk::ScalableAffineTransform<ScalarType, 3> TransformType;

    /** \brief Default constructor. */
    GeometryTransformHolder();

    /**
     * \brief Copy constructor.
     *
     * \param other The GeometryTransformHolder to copy from.
     */
    GeometryTransformHolder(const GeometryTransformHolder &other);

    /** \brief Destructor. */
    virtual ~GeometryTransformHolder();

    /** \brief Initialize the transform to default values. */
    void Initialize();

    /**
     * \brief Initialize this transform holder from another instance.
     *
     * \param other The GeometryTransformHolder to initialize from.
     */
    void Initialize(const GeometryTransformHolder *other);

    /**
     * \brief Get the origin, e.g. the upper-left corner of the plane.
     *
     * \return The origin point in world coordinates.
     */
    Point3D GetOrigin() const;

    /**
     * \brief Set the origin, i.e. the upper-left corner of the plane.
     *
     * \param origin The origin point in world coordinates.
     */
    void SetOrigin(const Point3D &origin);

    /**
     * \brief Get the spacing (size of a pixel).
     *
     * \return The spacing as a 3D vector.
     */
    mitk::Vector3D GetSpacing() const;

    /**
     * \brief Set the spacing.
     *
     * The spacing is also changed in the IndexToWorldTransform.
     *
     * \param aSpacing The spacing to set as a 3D vector.
     * \param enforceSetSpacing If \c true, spacing is set even when the transform suggests otherwise.
     */
    void SetSpacing(const mitk::Vector3D &aSpacing, bool enforceSetSpacing = false);

    /**
     * \brief Get the transformation used to convert from index to world coordinates.
     *
     * \return Pointer to the AffineTransform3D.
     */
    mitk::AffineTransform3D *GetIndexToWorldTransform();

    /**
     * \brief Get the transformation used to convert from index to world coordinates (const version).
     *
     * \return Const pointer to the AffineTransform3D.
     */
    const mitk::AffineTransform3D *GetIndexToWorldTransform() const;

    /**
     * \brief Set the transformation used to convert from index to world coordinates.
     *
     * This will also change the spacing.
     *
     * \param transform The AffineTransform3D to set.
     */
    void SetIndexToWorldTransform(mitk::AffineTransform3D *transform);

    /**
     * \brief Set the ITK transform (m_IndexToWorldTransform) via a vtkMatrix4x4.
     *
     * Convenience method. This will also change the spacing.
     *
     * \param vtkmatrix The vtkMatrix4x4 to set the transform from.
     *
     * \sa SetIndexToWorldTransform
     */
    virtual void SetIndexToWorldTransformByVtkMatrix(vtkMatrix4x4 *vtkmatrix);

    /**
     * \brief Set the transformation used to convert from index to world coordinates, keeping original spacing.
     *
     * \param transform The AffineTransform3D to set.
     */
    void SetIndexToWorldTransformWithoutChangingSpacing(mitk::AffineTransform3D *transform);

    /**
     * \brief Set the ITK transform via a vtkMatrix4x4, keeping original spacing.
     *
     * Convenience method.
     *
     * \param vtkmatrix The vtkMatrix4x4 to set the transform from.
     *
     * \sa SetIndexToWorldTransform
     */
    void SetIndexToWorldTransformByVtkMatrixWithoutChangingSpacing(vtkMatrix4x4 *vtkmatrix);

    /**
     * \brief Get the VTK matrix which describes the transform.
     *
     * \return Pointer to the vtkMatrix4x4.
     */
    vtkMatrix4x4 *GetVtkMatrix();

    /**
     * \brief Get the VTK matrix which describes the transform (const version).
     *
     * \return Const pointer to the vtkMatrix4x4.
     */
    const vtkMatrix4x4 *GetVtkMatrix() const;

    /**
     * \brief Get the index-to-world transform as a vtkLinearTransform.
     *
     * \return Pointer to the vtkLinearTransform.
     */
    vtkLinearTransform *GetVtkTransform() const;

    /**
     * \brief Set the rotation/scaling matrix.
     *
     * \param matrix The 3x3 matrix to set.
     */
    void SetMatrix(Matrix3D &matrix);

    /** \brief Set the transform to identity. */
    void SetIdentity();

    /**
     * \brief Compose with another transform.
     *
     * \param other The transform to compose with.
     * \param pre If \c true, the other transform is applied before this one; if \c false, after.
     */
    void Compose(const TransformType *other, bool pre = false);

    /**
     * \brief Deep-copy a vtkTransform into the internal VTK matrix.
     *
     * \param vtktransform The vtkTransform to copy from.
     */
    void SetVtkMatrixDeepCopy(vtkTransform *vtktransform);

    /**
     * \brief Check whether the IndexToWorldTransform is null.
     *
     * \return \c true if the transform is null, \c false otherwise.
     */
    bool IsIndexToWorldTransformNull();

    /**
     * \brief Get the VNL matrix representation of the transform.
     *
     * \return The VNL internal matrix type.
     */
    AffineTransform3D::MatrixType::InternalMatrixType GetVnlMatrix();
  };
  /**
   * \brief Compare two GeometryTransformHolder objects for equality.
   *
   * \param leftHandSide First transform holder.
   * \param rightHandSide Second transform holder.
   * \param eps Tolerance for floating-point comparison.
   * \param verbose If \c true, print details about differences.
   * \return \c true if the two transform holders are equal within the given tolerance.
   */
  MITKCORE_EXPORT bool Equal(const mitk::GeometryTransformHolder &leftHandSide,
                             const mitk::GeometryTransformHolder &rightHandSide,
                             ScalarType eps,
                             bool verbose);

  /**
   * \brief Compare two GeometryTransformHolder objects for equality (pointer version).
   *
   * \param leftHandSide Pointer to first transform holder.
   * \param rightHandSide Pointer to second transform holder.
   * \param eps Tolerance for floating-point comparison.
   * \param verbose If \c true, print details about differences.
   * \return \c true if the two transform holders are equal within the given tolerance.
   */
  MITKCORE_EXPORT bool Equal(const mitk::GeometryTransformHolder *leftHandSide,
                             const mitk::GeometryTransformHolder *rightHandSide,
                             ScalarType eps,
                             bool verbose);
}
#endif
