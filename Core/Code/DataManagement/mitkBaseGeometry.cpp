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

#include <sstream>
#include <iomanip>

#include "mitkBaseGeometry.h"
#include "mitkvector.h"
#include "mitkMatrixConvert.h"
#include <vtkMatrixToLinearTransform.h>
#include <vtkMatrix4x4.h>

mitk::BaseGeometry::BaseGeometry(): Superclass(), mitk::OperationActor(),
  m_Valid(true), m_FrameOfReferenceID(0), m_IndexToWorldTransformLastModified(0)
{
  FillVector3D(m_FloatSpacing, 1,1,1);
  m_VtkMatrix = vtkMatrix4x4::New();
  m_VtkIndexToWorldTransform = vtkMatrixToLinearTransform::New();
  m_VtkIndexToWorldTransform->SetInput(m_VtkMatrix);
  Initialize();
}

mitk::BaseGeometry::BaseGeometry(const BaseGeometry& other): Superclass(), m_ParametricBoundingBox(other.m_ParametricBoundingBox), m_TimeBounds(other.m_TimeBounds),
  m_Valid(other.m_Valid), m_FrameOfReferenceID(other.m_FrameOfReferenceID), m_IndexToWorldTransformLastModified(other.m_IndexToWorldTransformLastModified), m_Origin(other.m_Origin)
{
  //  DEPRECATED(m_RotationQuaternion = other.m_RotationQuaternion);
  // AffineGeometryFrame
  SetBounds(other.GetBounds());
  m_VtkMatrix = vtkMatrix4x4::New();
  m_VtkMatrix->DeepCopy(other.m_VtkMatrix);
  if (other.m_ParametricBoundingBox.IsNotNull())
  {
    m_ParametricBoundingBox = other.m_ParametricBoundingBox->DeepCopy();
  }
  FillVector3D(m_FloatSpacing,other.m_FloatSpacing[0],other.m_FloatSpacing[1],other.m_FloatSpacing[2]);
  m_VtkIndexToWorldTransform = vtkMatrixToLinearTransform::New();
  m_VtkIndexToWorldTransform->DeepCopy(other.m_VtkIndexToWorldTransform);
  m_VtkIndexToWorldTransform->SetInput(m_VtkMatrix);
  other.InitializeGeometry(this);
}

mitk::BaseGeometry::~BaseGeometry()
{
  m_VtkMatrix->Delete();
  m_VtkIndexToWorldTransform->Delete();
}

const mitk::Point3D& mitk::BaseGeometry::GetOrigin() const
{
  return m_Origin;
}

void mitk::BaseGeometry::SetOrigin(const Point3D & origin)
{
  if(origin!=GetOrigin())
  {
    m_Origin = origin;
    m_IndexToWorldTransform->SetOffset(m_Origin.GetVectorFromOrigin());
    Modified();
    TransferItkToVtkTransform();
  }
}

void mitk::BaseGeometry::TransferItkToVtkTransform()
{
  // copy m_IndexToWorldTransform into m_VtkIndexToWorldTransform

  TransferItkTransformToVtkMatrix(m_IndexToWorldTransform.GetPointer(), m_VtkMatrix);
  m_VtkIndexToWorldTransform->Modified();
}

void mitk::BaseGeometry::CopySpacingFromTransform(mitk::AffineTransform3D* transform, mitk::Vector3D& spacing, float floatSpacing[3])
{
  mitk::AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
  vnlmatrix = transform->GetMatrix().GetVnlMatrix();

  spacing[0]=vnlmatrix.get_column(0).magnitude();
  spacing[1]=vnlmatrix.get_column(1).magnitude();
  spacing[2]=vnlmatrix.get_column(2).magnitude();
  floatSpacing[0]=spacing[0];
  floatSpacing[1]=spacing[1];
  floatSpacing[2]=spacing[2];
}

void mitk::BaseGeometry::Initialize()
{
  float b[6] = {0,1,0,1,0,1};
  SetFloatBounds(b);

  if(m_IndexToWorldTransform.IsNull())
    m_IndexToWorldTransform = TransformType::New();
  else
    m_IndexToWorldTransform->SetIdentity();
  CopySpacingFromTransform(m_IndexToWorldTransform, m_Spacing, m_FloatSpacing);
  vtk2itk(m_IndexToWorldTransform->GetOffset(), m_Origin);

  m_VtkMatrix->Identity();

  m_TimeBounds[0]=ScalarTypeNumericTraits::NonpositiveMin(); m_TimeBounds[1]=ScalarTypeNumericTraits::max();

  m_FrameOfReferenceID = 0;
}

void mitk::BaseGeometry::SetFloatBounds(const float bounds[6])
{
  mitk::BoundingBox::BoundsArrayType b;
  const float *input = bounds;
  int i=0;
  for(mitk::BoundingBox::BoundsArrayType::Iterator it = b.Begin(); i < 6 ;++i) *it++ = (mitk::ScalarType)*input++;
  SetBounds(b);
}

void mitk::BaseGeometry::SetFloatBounds(const double bounds[6])
{
  mitk::BoundingBox::BoundsArrayType b;
  const double *input = bounds;
  int i=0;
  for(mitk::BoundingBox::BoundsArrayType::Iterator it = b.Begin(); i < 6 ;++i) *it++ = (mitk::ScalarType)*input++;
  SetBounds(b);
}

/** Initialize the geometry */
void
  mitk::BaseGeometry::InitializeGeometry(BaseGeometry* newGeometry) const
{
  newGeometry->SetBounds(m_BoundingBox->GetBounds());
  // we have to create a new transform!!

  if(m_IndexToWorldTransform)
  {
    TransformType::Pointer indexToWorldTransform = TransformType::New();
    indexToWorldTransform->SetCenter( m_IndexToWorldTransform->GetCenter() );
    indexToWorldTransform->SetMatrix( m_IndexToWorldTransform->GetMatrix() );
    indexToWorldTransform->SetOffset( m_IndexToWorldTransform->GetOffset() );
    newGeometry->SetIndexToWorldTransform(indexToWorldTransform);
  }
}

/** Set the bounds */
void mitk::BaseGeometry::SetBounds(const BoundsArrayType& bounds)
{
  m_BoundingBox = BoundingBoxType::New();

  BoundingBoxType::PointsContainer::Pointer pointscontainer =
    BoundingBoxType::PointsContainer::New();
  BoundingBoxType::PointType p;
  BoundingBoxType::PointIdentifier pointid;

  for(pointid=0; pointid<2;++pointid)
  {
    unsigned int i;
    for(i=0; i<NDimensions; ++i)
    {
      p[i] = bounds[2*i+pointid];
    }
    pointscontainer->InsertElement(pointid, p);
  }

  m_BoundingBox->SetPoints(pointscontainer);
  m_BoundingBox->ComputeBoundingBox();
  this->Modified();
}

void mitk::BaseGeometry::SetIndexToWorldTransform(mitk::AffineTransform3D* transform)
{
  if(m_IndexToWorldTransform.GetPointer() != transform)
  {
    m_IndexToWorldTransform = transform;
    CopySpacingFromTransform(m_IndexToWorldTransform, m_Spacing, m_FloatSpacing);
    vtk2itk(m_IndexToWorldTransform->GetOffset(), m_Origin);
    TransferItkToVtkTransform();
    Modified();
  }
}

const  mitk::BaseGeometry::BoundsArrayType  mitk::BaseGeometry::GetBounds() const
{
  assert(m_BoundingBox.IsNotNull());
  return m_BoundingBox->GetBounds();
}

bool mitk::BaseGeometry::IsValid() const
{
  return m_Valid;
}

const float* mitk::BaseGeometry::GetFloatSpacing() const
{
  return m_FloatSpacing;
}

bool mitk::Equal(const BaseGeometry::TransformType *leftHandSide, const BaseGeometry::TransformType *rightHandSide, ScalarType eps, bool verbose )
{
  //Compare IndexToWorldTransform Matrix
  if( !mitk::MatrixEqualElementWise(  leftHandSide->GetMatrix(),
    rightHandSide->GetMatrix() ) )
  {
    if(verbose)
    {
      MITK_INFO << "[( BaseGeometry::TransformType )] Index to World Transformation matrix differs.";
      MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide->GetMatrix() << " : leftHandSide is " << leftHandSide->GetMatrix() << " and tolerance is " << eps;
    }
    return false;
  }
  return true;
}

bool mitk::Equal( const mitk::BaseGeometry::BoundingBoxType *leftHandSide, const mitk::BaseGeometry::BoundingBoxType *rightHandSide, ScalarType eps, bool verbose )
{
  bool result = true;
  if( rightHandSide == NULL )
  {
    if(verbose)
      MITK_INFO << "[( BaseGeometry::BoundingBoxType )] rightHandSide NULL.";
    return false;
  }
  if( leftHandSide == NULL )
  {
    if(verbose)
      MITK_INFO << "[( BaseGeometry::BoundingBoxType )] leftHandSide NULL.";
    return false;
  }

  BaseGeometry::BoundsArrayType rightBounds = rightHandSide->GetBounds();
  BaseGeometry::BoundsArrayType leftBounds = leftHandSide->GetBounds();
  BaseGeometry::BoundsArrayType::Iterator itLeft = leftBounds.Begin();
  for( BaseGeometry::BoundsArrayType::Iterator itRight = rightBounds.Begin(); itRight != rightBounds.End(); ++itRight)
  {
    if(( !mitk::Equal( *itLeft, *itRight, eps )) )
    {
      if(verbose)
      {
        MITK_INFO << "[( BaseGeometry::BoundingBoxType )] bounds are not equal.";
        MITK_INFO << "rightHandSide is " << setprecision(12) << *itRight << " : leftHandSide is " << *itLeft << " and tolerance is " << eps;
      }
      result = false;
    }
    itLeft++;
  }
  return result;
}

bool mitk::Equal(const mitk::BaseGeometry *leftHandSide, const mitk::BaseGeometry *rightHandSide, ScalarType eps, bool verbose)
{
  bool result = true;

  if( rightHandSide == NULL )
  {
    if(verbose)
      MITK_INFO << "[( BaseGeometry )] rightHandSide NULL.";
    return false;
  }
  if( leftHandSide == NULL)
  {
    if(verbose)
      MITK_INFO << "[( BaseGeometry )] leftHandSide NULL.";
    return false;
  }

  //Compare spacings
  if( !mitk::Equal( leftHandSide->GetSpacing(), rightHandSide->GetSpacing(), eps ) )
  {
    if(verbose)
    {
      MITK_INFO << "[( BaseGeometry )] Spacing differs.";
      MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide->GetSpacing() << " : leftHandSide is " << leftHandSide->GetSpacing() << " and tolerance is " << eps;
    }
    result = false;
  }

  //Compare Origins
  if( !mitk::Equal( leftHandSide->GetOrigin(), rightHandSide->GetOrigin(), eps ) )
  {
    if(verbose)
    {
      MITK_INFO << "[( BaseGeometry )] Origin differs.";
      MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide->GetOrigin() << " : leftHandSide is " << leftHandSide->GetOrigin() << " and tolerance is " << eps;
    }
    result = false;
  }

  //Compare Axis and Extents
/* xxxxxxxxxxxxxxx Funktionen noch nciht umgezogen!

for( unsigned int i=0; i<3; ++i)
  {
    if( !mitk::Equal( leftHandSide->GetAxisVector(i), rightHandSide->GetAxisVector(i), eps))
    {
      if(verbose)
      {
        MITK_INFO << "[( BaseGeometry )] AxisVector #" << i << " differ";
        MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide->GetAxisVector(i) << " : leftHandSide is " << leftHandSide->GetAxisVector(i) << " and tolerance is " << eps;
      }
      result =  false;
    }

    if( !mitk::Equal( leftHandSide->GetExtent(i), rightHandSide->GetExtent(i), eps) )
    {
      if(verbose)
      {
        MITK_INFO << "[( BaseGeometry )] Extent #" << i << " differ";
        MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide->GetExtent(i) << " : leftHandSide is " << leftHandSide->GetExtent(i) << " and tolerance is " << eps;
      }
      result = false;
    }
  }

*/

  //Compare BoundingBoxes
  if( !mitk::Equal( leftHandSide->GetBoundingBox(), rightHandSide->GetBoundingBox(), eps, verbose) )
  {
    result = false;
  }

  //Compare IndexToWorldTransform Matrix
  if( !mitk::Equal( leftHandSide->GetIndexToWorldTransform(), rightHandSide->GetIndexToWorldTransform(), eps, verbose) )
  {
    result = false;
  }
  return result;
}

void mitk::BaseGeometry::SetSpacing(const mitk::Vector3D& aSpacing)
{
  if(mitk::Equal(m_Spacing, aSpacing) == false)
  {
    assert(aSpacing[0]>0 && aSpacing[1]>0 && aSpacing[2]>0);

    m_Spacing = aSpacing;

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

    itk2vtk(m_Spacing, m_FloatSpacing);
  }
}
