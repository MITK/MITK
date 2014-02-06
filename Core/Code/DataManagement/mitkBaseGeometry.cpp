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
  m_ImageGeometry(false), m_Valid(true), m_FrameOfReferenceID(0), m_IndexToWorldTransformLastModified(0)
{
  FillVector3D(m_FloatSpacing, 1,1,1);
  m_VtkMatrix = vtkMatrix4x4::New();
  m_VtkIndexToWorldTransform = vtkMatrixToLinearTransform::New();
  m_VtkIndexToWorldTransform->SetInput(m_VtkMatrix);
  Initialize();
}

mitk::BaseGeometry::BaseGeometry(const BaseGeometry& other): Superclass(), m_ParametricBoundingBox(other.m_ParametricBoundingBox), m_TimeBounds(other.m_TimeBounds),
  m_ImageGeometry(other.m_ImageGeometry), m_Valid(other.m_Valid), m_FrameOfReferenceID(other.m_FrameOfReferenceID), m_IndexToWorldTransformLastModified(other.m_IndexToWorldTransformLastModified), m_RotationQuaternion( other.m_RotationQuaternion ), m_Origin(other.m_Origin)
{
  // AffineGeometryFrame
  SetBounds(other.GetBounds());
  //SetIndexToObjectTransform(other.GetIndexToObjectTransform());
  //SetObjectToNodeTransform(other.GetObjectToNodeTransform());
  //SetIndexToWorldTransform(other.GetIndexToWorldTransform());
  // this is not used in AffineGeometryFrame of ITK, thus there are not Get and Set methods
  // m_IndexToNodeTransform = other.m_IndexToNodeTransform;
  // m_InvertedTransform = TransformType::New();
  // m_InvertedTransform = TransformType::New();
  // m_InvertedTransform->DeepCopy(other.m_InvertedTransform);
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

  m_ImageGeometry = false;
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
