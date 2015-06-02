/*
* mitkGeometryTransformHolder.cpp
*
*  Created on: Sep 3, 2014
*      Author: wirkert
*/

#include <itkMatrix.h>
#include <itkScalableAffineTransform.h>
#include <itkSmartPointer.h>
#include <mitkBaseGeometry.h>
#include <mitkGeometryTransformHolder.h>
#include <mitkMatrix.h>
#include <mitkMatrixConvert.h>
#include <mitkPoint.h>
#include <mitkVector.h>
#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_vector.h>
#include <vnl/vnl_vector_fixed.h>
#include <vtkMatrix4x4.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkTransform.h>
#include <cassert>

namespace mitk {
  void GeometryTransformHolder::CopySpacingFromTransform(const mitk::AffineTransform3D* transform, mitk::Vector3D& spacing)
  {
    mitk::AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
    vnlmatrix = transform->GetMatrix().GetVnlMatrix();

    spacing[0]=vnlmatrix.get_column(0).magnitude();
    spacing[1]=vnlmatrix.get_column(1).magnitude();
    spacing[2]=vnlmatrix.get_column(2).magnitude();
  }

  GeometryTransformHolder::GeometryTransformHolder()
  {
    m_VtkMatrix = vtkMatrix4x4::New();
    m_VtkIndexToWorldTransform = vtkMatrixToLinearTransform::New();
    m_VtkIndexToWorldTransform->SetInput(m_VtkMatrix);
    this->Initialize();
  }

  GeometryTransformHolder::GeometryTransformHolder(const GeometryTransformHolder& other)
  {
    m_VtkMatrix = vtkMatrix4x4::New();
    m_VtkIndexToWorldTransform = vtkMatrixToLinearTransform::New();
    m_VtkIndexToWorldTransform->SetInput(m_VtkMatrix);
    this->Initialize(&other);
  }

  GeometryTransformHolder::~GeometryTransformHolder()
  {
    m_VtkMatrix->Delete();
    m_VtkIndexToWorldTransform->Delete();
  }

  void GeometryTransformHolder::Initialize()
  {
    if(m_IndexToWorldTransform.IsNull())
      m_IndexToWorldTransform = TransformType::New();
    else
      m_IndexToWorldTransform->SetIdentity();

    m_VtkMatrix->Identity();
  }

  void GeometryTransformHolder::Initialize(const GeometryTransformHolder* other)
  {
    Initialize();

    if(other->GetIndexToWorldTransform())
    {
      TransformType::Pointer indexToWorldTransform = other->GetIndexToWorldTransform()->Clone();
      this->SetIndexToWorldTransform(indexToWorldTransform);
    }
  }

  //##Documentation
  //## @brief Copy the ITK transform
  //## (m_IndexToWorldTransform) to the VTK transform
  //## \sa SetIndexToWorldTransform
  void GeometryTransformHolder::TransferItkToVtkTransform()
  {
    TransferItkTransformToVtkMatrix(m_IndexToWorldTransform.GetPointer(), m_VtkMatrix);
  }

  //##Documentation
  //## @brief Copy the VTK transform
  //## to the ITK transform (m_IndexToWorldTransform)
  //## \sa SetIndexToWorldTransform
  void GeometryTransformHolder::TransferVtkToItkTransform()
  {
    TransferVtkMatrixToItkTransform(m_VtkMatrix, m_IndexToWorldTransform.GetPointer());
  }

  //##Documentation
  //## @brief Get the origin, e.g. the upper-left corner of the plane
  const Point3D GeometryTransformHolder::GetOrigin() const
  {
    mitk::Point3D origin = this->GetIndexToWorldTransform()->GetOffset();
    return origin;
  }

  //##Documentation
  //## @brief Set the origin, i.e. the upper-left corner of the plane
  //##
  void GeometryTransformHolder::SetOrigin(const Point3D& origin)
  {
    m_IndexToWorldTransform->SetOffset(origin.GetVectorFromOrigin());
    TransferItkToVtkTransform();
  }

  //##Documentation
  //## @brief Get the spacing (size of a pixel).
  //##
  const mitk::Vector3D GeometryTransformHolder::GetSpacing() const
  {
    mitk::Vector3D spacing;
    CopySpacingFromTransform(this->GetIndexToWorldTransform(), spacing);
    return spacing;
  }

  //##Documentation
  //## @brief Set the spacing.
  //##
  //##The spacing is also changed in the IndexToWorldTransform.
  void GeometryTransformHolder::SetSpacing(const mitk::Vector3D& aSpacing, bool enforceSetSpacing)
  {
    if(mitk::Equal(this->GetSpacing(), aSpacing) == false || enforceSetSpacing)
    {
      assert(aSpacing[0]>0 && aSpacing[1]>0 && aSpacing[2]>0);

      AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;

      vnlmatrix = m_IndexToWorldTransform->GetMatrix().GetVnlMatrix();

      mitk::VnlVector col;
      col = vnlmatrix.get_column(0); col.normalize(); col*=aSpacing[0]; vnlmatrix.set_column(0, col);
      col = vnlmatrix.get_column(1); col.normalize(); col*=aSpacing[1]; vnlmatrix.set_column(1, col);
      col = vnlmatrix.get_column(2); col.normalize(); col*=aSpacing[2]; vnlmatrix.set_column(2, col);

      Matrix3D matrix;
      matrix = vnlmatrix;

      AffineTransform3D::Pointer transform = AffineTransform3D::New();
      transform->SetMatrix(matrix);
      transform->SetOffset(m_IndexToWorldTransform->GetOffset());

      SetIndexToWorldTransform(transform.GetPointer());
    }
  }

  //##Documentation
  //## @brief Get the transformation used to convert from index
  //## to world coordinates
  mitk::AffineTransform3D* GeometryTransformHolder::GetIndexToWorldTransform()
  {
    return m_IndexToWorldTransform;
  }

  //##Documentation
  //## @brief Get the transformation used to convert from index
  //## to world coordinates
  const mitk::AffineTransform3D*   GeometryTransformHolder::GetIndexToWorldTransform() const
  {
    return m_IndexToWorldTransform;
  }

  //## @brief Set the transformation used to convert from index
  //## to world coordinates. The spacing of the new transform is
  //## copied to m_spacing.
  void GeometryTransformHolder::SetIndexToWorldTransform(mitk::AffineTransform3D* transform){
    m_IndexToWorldTransform = transform;
    TransferItkToVtkTransform();
  }

  //##Documentation
  //## @brief Convenience method for setting the ITK transform
  //## (m_IndexToWorldTransform) via an vtkMatrix4x4.The spacing of
  //## the new transform is copied to m_spacing.
  //## \sa SetIndexToWorldTransform
  void GeometryTransformHolder::SetIndexToWorldTransformByVtkMatrix(vtkMatrix4x4* vtkmatrix)
  {
    m_VtkMatrix->DeepCopy(vtkmatrix);
    TransferVtkToItkTransform();
  }

  //## @brief Set the transformation used to convert from index
  //## to world coordinates.This function keeps the original spacing.
  void GeometryTransformHolder::SetIndexToWorldTransformWithoutChangingSpacing(mitk::AffineTransform3D* transform){
    mitk::Vector3D originalSpacing = this->GetSpacing();
    this->SetIndexToWorldTransform(transform);
    this->SetSpacing(originalSpacing);
  }

  //##Documentation
  //## @brief Convenience method for setting the ITK transform
  //## (m_IndexToWorldTransform) via an vtkMatrix4x4. This function keeps the original spacing.
  //## \sa SetIndexToWorldTransform
  void GeometryTransformHolder::SetIndexToWorldTransformByVtkMatrixWithoutChangingSpacing(vtkMatrix4x4* vtkmatrix){
    mitk::Vector3D originalSpacing = this->GetSpacing();
    this->SetIndexToWorldTransformByVtkMatrix(vtkmatrix);
    this->SetSpacing(originalSpacing);
  }

  //## Get the Vtk Matrix which describes the transform.
  vtkMatrix4x4* GeometryTransformHolder::GetVtkMatrix()
  {
    return m_VtkMatrix;
  }

  //## Get the Vtk Matrix which describes the transform.
  const vtkMatrix4x4* GeometryTransformHolder::GetVtkMatrix() const
  {
    return m_VtkMatrix;
  }

  //##Documentation
  //## @brief Get the m_IndexToWorldTransform as a vtkLinearTransform
  vtkLinearTransform* GeometryTransformHolder::GetVtkTransform() const
  {
    return (vtkLinearTransform*)m_VtkIndexToWorldTransform;
  }

  void GeometryTransformHolder::SetMatrix(Matrix3D & matrix)
  {
    m_IndexToWorldTransform->SetMatrix(matrix);
    TransferItkToVtkTransform();
  }

  void GeometryTransformHolder::SetIdentity()
  {
    m_IndexToWorldTransform->SetIdentity();
    TransferItkToVtkTransform();
  }

  void GeometryTransformHolder::Compose( const TransformType * other, bool pre )
  {
    m_IndexToWorldTransform->Compose(other, pre);
    TransferItkToVtkTransform();
  }

  void GeometryTransformHolder::SetVtkMatrixDeepCopy(vtkTransform *vtktransform)
  {
    m_VtkMatrix->DeepCopy(vtktransform->GetMatrix());
    TransferVtkToItkTransform();
  }

  bool GeometryTransformHolder::IsIndexToWorldTransformNull()
  {
    return m_IndexToWorldTransform.IsNull();
  }

  AffineTransform3D::MatrixType::InternalMatrixType GeometryTransformHolder::GetVnlMatrix()
  {
    return m_IndexToWorldTransform->GetMatrix().GetVnlMatrix();
  }
}

bool mitk::Equal(const mitk::GeometryTransformHolder *leftHandSide, const mitk::GeometryTransformHolder *rightHandSide, ScalarType eps, bool verbose)
{
  if(( leftHandSide == nullptr) || ( rightHandSide == nullptr ))
  {
    MITK_ERROR << "mitk::Equal(const mitk::Geometry3D *leftHandSide, const mitk::Geometry3D *rightHandSide, ScalarType eps, bool verbose) does not with NULL pointer input.";
    return false;
  }
  return Equal( *leftHandSide, *rightHandSide, eps, verbose);
}

bool mitk::Equal(const mitk::GeometryTransformHolder& leftHandSide, const mitk::GeometryTransformHolder& rightHandSide, ScalarType eps, bool verbose)
{
  bool result = true;

  //Compare spacings
  if( !mitk::Equal( leftHandSide.GetSpacing(), rightHandSide.GetSpacing(), eps ) )
  {
    if(verbose)
    {
      MITK_INFO << "[( Geometry3D )] Spacing differs.";
      MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide.GetSpacing() << " : leftHandSide is " << leftHandSide.GetSpacing() << " and tolerance is " << eps;
    }
    result = false;
  }

  //Compare Origins
  if( !mitk::Equal( leftHandSide.GetOrigin(), rightHandSide.GetOrigin(), eps ) )
  {
    if(verbose)
    {
      MITK_INFO << "[( Geometry3D )] Origin differs.";
      MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide.GetOrigin() << " : leftHandSide is " << leftHandSide.GetOrigin() << " and tolerance is " << eps;
    }
    result = false;
  }

  //Compare IndexToWorldTransform Matrix
  if( !mitk::Equal( *leftHandSide.GetIndexToWorldTransform(), *rightHandSide.GetIndexToWorldTransform(), eps, verbose) )
  {
    result = false;
  }

  //Compare vtk Matrix
  for (int i=0;i<4;i++){
    for (int j=0;j<4;j++){
      if( leftHandSide.GetVtkMatrix()->GetElement(i,j) != rightHandSide.GetVtkMatrix()->GetElement(i,j))
      {
        result = false;
      }
    }
  }

  return result;
}