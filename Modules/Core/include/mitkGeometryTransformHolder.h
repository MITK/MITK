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

#ifndef MITKGEOMETRYTRANSFORMHOLDER_H_
#define MITKGEOMETRYTRANSFORMHOLDER_H_

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
  /*
  GeometryTransformHolder is a helper class which manages the transform related variables.
  Its job is to keep all information about the transform (as e.g.: spacing, offset, transformation matrices) in
  consistent states.
  It provides getters and setters to all transformation related information. Implementation is hidden and may be
  subject to changes.
  */
  class GeometryTransformHolder
  {
  private:
    //##Documentation
    //## @brief Index to World Transform, contains a transformation matrix to convert
    //## points from index coordinates to world coordinates (mm). The Spacing is included in this variable.
    AffineTransform3D::Pointer m_IndexToWorldTransform;

    vtkMatrix4x4 *m_VtkMatrix;
    vtkMatrixToLinearTransform *m_VtkIndexToWorldTransform;

    static void CopySpacingFromTransform(const mitk::AffineTransform3D *transform, mitk::Vector3D &spacing);

    //##Documentation
    //## @brief Copy the ITK transform
    //## (m_IndexToWorldTransform) to the VTK transform
    //## \sa SetIndexToWorldTransform
    void TransferItkToVtkTransform();

    //##Documentation
    //## @brief Copy the VTK transform
    //## to the ITK transform (m_IndexToWorldTransform)
    //## \sa SetIndexToWorldTransform
    void TransferVtkToItkTransform();

  protected:
  public:
    typedef itk::ScalableAffineTransform<ScalarType, 3> TransformType;

    GeometryTransformHolder();

    GeometryTransformHolder(const GeometryTransformHolder &other);

    virtual ~GeometryTransformHolder();

    void Initialize();

    void Initialize(const GeometryTransformHolder *other);

    //##Documentation
    //## @brief Get the origin, e.g. the upper-left corner of the plane
    const Point3D GetOrigin() const;

    //##Documentation
    //## @brief Set the origin, i.e. the upper-left corner of the plane
    //##
    void SetOrigin(const Point3D &origin);

    //##Documentation
    //## @brief Get the spacing (size of a pixel).
    //##
    const mitk::Vector3D GetSpacing() const;

    //##Documentation
    //## @brief Set the spacing (m_Spacing).
    //##
    //##The spacing is also changed in the IndexToWorldTransform.
    void SetSpacing(const mitk::Vector3D &aSpacing, bool enforceSetSpacing = false);

    //##Documentation
    //## @brief Get the transformation used to convert from index
    //## to world coordinates
    mitk::AffineTransform3D *GetIndexToWorldTransform();

    //##Documentation
    //## @brief Get the transformation used to convert from index
    //## to world coordinates
    const mitk::AffineTransform3D *GetIndexToWorldTransform() const;

    //## @brief Set the transformation used to convert from index
    //## to world coordinates.This will also change the spacing.
    void SetIndexToWorldTransform(mitk::AffineTransform3D *transform);

    //##Documentation
    //## @brief Convenience method for setting the ITK transform
    //## (m_IndexToWorldTransform) via an vtkMatrix4x4. This will also change the spacing.
    //## \sa SetIndexToWorldTransform
    virtual void SetIndexToWorldTransformByVtkMatrix(vtkMatrix4x4 *vtkmatrix);

    //## @brief Set the transformation used to convert from index
    //## to world coordinates.This function keeps the original spacing.
    void SetIndexToWorldTransformWithoutChangingSpacing(mitk::AffineTransform3D *transform);

    //##Documentation
    //## @brief Convenience method for setting the ITK transform
    //## (m_IndexToWorldTransform) via an vtkMatrix4x4. This function keeps the original spacing.
    //## \sa SetIndexToWorldTransform
    void SetIndexToWorldTransformByVtkMatrixWithoutChangingSpacing(vtkMatrix4x4 *vtkmatrix);

    //## Get the Vtk Matrix which describes the transform.
    vtkMatrix4x4 *GetVtkMatrix();

    //## Get the Vtk Matrix which describes the transform.
    const vtkMatrix4x4 *GetVtkMatrix() const;

    //##Documentation
    //## @brief Get the m_IndexToWorldTransform as a vtkLinearTransform
    vtkLinearTransform *GetVtkTransform() const;

    void SetMatrix(Matrix3D &matrix);

    void SetIdentity();

    void Compose(const TransformType *other, bool pre = false);

    void SetVtkMatrixDeepCopy(vtkTransform *vtktransform);

    bool IsIndexToWorldTransformNull();

    AffineTransform3D::MatrixType::InternalMatrixType GetVnlMatrix();
  };
  MITKCORE_EXPORT bool Equal(const mitk::GeometryTransformHolder &leftHandSide,
                             const mitk::GeometryTransformHolder &rightHandSide,
                             ScalarType eps,
                             bool verbose);

  MITKCORE_EXPORT bool Equal(const mitk::GeometryTransformHolder *leftHandSide,
                             const mitk::GeometryTransformHolder *rightHandSide,
                             ScalarType eps,
                             bool verbose);
}
#endif /* MITKGEOMETRYTRANSFORMHOLDER_H_ */
