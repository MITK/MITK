/*
 * mitkGeometryTransformHolder.h
 *
 *  Created on: Sep 3, 2014
 *      Author: wirkert
 */

#ifndef MITKGEOMETRYTRANSFORMHOLDER_H_
#define MITKGEOMETRYTRANSFORMHOLDER_H_



#include <mitkAffineTransform3D.h>
#include <vtkMatrixToLinearTransform.h>

namespace mitk {

// Here is a helper class which manages the transform related variables.
// This needs to be implemented at the beginning of the cpp file.
// Do not change if you don't know what you are doing!
class GeometryTransformHolder
{
private:
  //##Documentation
  //## @brief Index to World Transform, contains a transformation matrix to convert
  //## points from index coordinates to world coordinates (mm). The Spacing is included in this variable.
  AffineTransform3D::Pointer m_IndexToWorldTransform;

  vtkMatrix4x4* m_VtkMatrix;
  vtkMatrixToLinearTransform* m_VtkIndexToWorldTransform;

  static void CopySpacingFromTransform(const mitk::AffineTransform3D* transform, mitk::Vector3D& spacing);

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


  typedef itk::ScalableAffineTransform<ScalarType, 3>    TransformType;

  GeometryTransformHolder();

  GeometryTransformHolder(const GeometryTransformHolder& other);

  virtual ~GeometryTransformHolder();

  void Initialize();



  //##Documentation
  //## @brief Get the origin, e.g. the upper-left corner of the plane
  const Point3D GetOrigin() const;

  //##Documentation
  //## @brief Set the origin, i.e. the upper-left corner of the plane
  //##
  void SetOrigin(const Point3D& origin);

  //##Documentation
  //## @brief Get the spacing (size of a pixel).
  //##
  const mitk::Vector3D GetSpacing() const;

  //##Documentation
  //## @brief Set the spacing (m_Spacing).
  //##
  //##The spacing is also changed in the IndexToWorldTransform.
  void SetSpacing(const mitk::Vector3D& aSpacing, bool enforceSetSpacing = false);

  //##Documentation
  //## @brief Get the transformation used to convert from index
  //## to world coordinates
  mitk::AffineTransform3D* GetIndexToWorldTransform();

  //##Documentation
  //## @brief Get the transformation used to convert from index
  //## to world coordinates
  const mitk::AffineTransform3D*   GetIndexToWorldTransform() const;

  //## @brief Set the transformation used to convert from index
  //## to world coordinates. The spacing of the new transform is
  //## copied to m_spacing.
  void SetIndexToWorldTransform(mitk::AffineTransform3D* transform);

  //##Documentation
  //## @brief Convenience method for setting the ITK transform
  //## (m_IndexToWorldTransform) via an vtkMatrix4x4.The spacing of
  //## the new transform is copied to m_spacing.
  //## \sa SetIndexToWorldTransform
  virtual void SetIndexToWorldTransformByVtkMatrix(vtkMatrix4x4* vtkmatrix);

  //## Get the Vtk Matrix which describes the transform.
  vtkMatrix4x4* GetVtkMatrix();

  //##Documentation
  //## @brief Get the m_IndexToWorldTransform as a vtkLinearTransform
  vtkLinearTransform* GetVtkTransform() const;

  void SetMatrix(Matrix3D & matrix);

  void SetIdentity();

  void Compose( const TransformType * other, bool pre );

  void SetVtkMatrixDeepCopy(vtkTransform *vtktransform);

  bool IsIndexToWorldTransformNull();

  AffineTransform3D::MatrixType::InternalMatrixType GetVnlMatrix();
};
}
#endif /* MITKGEOMETRYTRANSFORMHOLDER_H_ */
