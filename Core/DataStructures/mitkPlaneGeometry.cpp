/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlaneGeometry.h"
#include <vnl/vnl_cross.h>

//##ModelId=3E395F22035A
mitk::PlaneGeometry::PlaneGeometry()
{
  Initialize();
}


//##ModelId=3E395F220382
mitk::PlaneGeometry::~PlaneGeometry()
{
}

void mitk::PlaneGeometry::Initialize()
{
  Superclass::Initialize();
}

void mitk::PlaneGeometry::EnsurePerpendicularNormal(mitk::AffineTransform3D* transform)
{
  //ensure row(2) of transform to be perpendicular to plane, keep length.
  VnlVector normal = vnl_cross_3d(transform->GetMatrix().GetVnlMatrix().get_column(0),transform->GetMatrix().GetVnlMatrix().get_column(1));
  normal.normalize();
  ScalarType len = transform->GetMatrix().GetVnlMatrix().get_column(2).two_norm();
  if(len==0) len = 1;
  normal*=len;
  mitk::Matrix3D matrix = transform->GetMatrix();
  matrix.GetVnlMatrix().set_column(2, normal);
  transform->SetMatrix(matrix);
}

void mitk::PlaneGeometry::SetIndexToWorldTransform(mitk::AffineTransform3D* transform)
{
  EnsurePerpendicularNormal(transform);

  Superclass::SetIndexToWorldTransform(transform);
}

void mitk::PlaneGeometry::SetBounds(const BoundingBox::BoundsArrayType& bounds)
{
  //currently the unit rectangle must be starting at the origin [0,0]
  assert(bounds[0]==0);
  assert(bounds[2]==0);
  //the unit rectangle must be two-dimensional
  assert(bounds[1]>0);
  assert(bounds[3]>0);

  Superclass::SetBounds(bounds);
}

//##ModelId=3E3B9C730262
void mitk::PlaneGeometry::IndexToWorld(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const
{
  pt_mm[0]=m_ScaleFactorMMPerUnitX*pt_units[0];
  pt_mm[1]=m_ScaleFactorMMPerUnitY*pt_units[1];
}

//##ModelId=3E3B9C760112
void mitk::PlaneGeometry::WorldToIndex(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const
{
  pt_units[0]=pt_mm[0]*(1.0/m_ScaleFactorMMPerUnitX);
  pt_units[1]=pt_mm[1]*(1.0/m_ScaleFactorMMPerUnitY);
}

//##ModelId=3E3B9C8C0145
void mitk::PlaneGeometry::IndexToWorld(const mitk::Point2D &atPt2d_units, const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const
{
  vec_mm[0]=m_ScaleFactorMMPerUnitX*vec_units[0];
  vec_mm[1]=m_ScaleFactorMMPerUnitY*vec_units[1];
}

//##ModelId=3E3B9C8E0152
void mitk::PlaneGeometry::WorldToIndex(const mitk::Point2D &atPt2d_mm, const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const
{
  vec_units[0]=vec_mm[0]*(1.0/m_ScaleFactorMMPerUnitX);
  vec_units[1]=vec_mm[1]*(1.0/m_ScaleFactorMMPerUnitY);
}

void mitk::PlaneGeometry::InitializeStandardPlane(mitk::ScalarType width, mitk::ScalarType height, const Vector3D & spacing, mitk::PlaneGeometry::PlaneOrientation planeorientation, mitk::ScalarType zPosition, bool frontside, bool rotated)
{
  AffineTransform3D::Pointer transform;

  transform = AffineTransform3D::New();
  AffineTransform3D::MatrixType matrix;
  AffineTransform3D::MatrixType::InternalMatrixType & vnlmatrix = matrix.GetVnlMatrix();
  vnlmatrix.set_identity();
  vnlmatrix(0,0) = spacing[0];
  vnlmatrix(1,1) = spacing[1];
  vnlmatrix(2,2) = spacing[2];
  transform->SetIdentity();
  transform->SetMatrix(matrix);
  
  InitializeStandardPlane(width, height, transform.GetPointer(), planeorientation, zPosition, frontside, rotated);
}

void mitk::PlaneGeometry::InitializeStandardPlane(mitk::ScalarType width, mitk::ScalarType height, const mitk::AffineTransform3D* transform, mitk::PlaneGeometry::PlaneOrientation planeorientation, mitk::ScalarType zPosition, bool frontside, bool rotated)
{
  Superclass::Initialize();

  //construct standard view
  mitk::Point3D  origin; 
  VnlVector rightDV(3), bottomDV(3);
  origin.Fill(0);
  int normalDirection;
  switch(planeorientation)
  {
    case Transversal:
      if(frontside)
      {
        if(rotated==false)
        {
          FillVector3D(origin,   0,  0, zPosition);
          FillVector3D(rightDV,  1,  0,         0);
          FillVector3D(bottomDV, 0,  1,         0);
        }
        else
        {
          FillVector3D(origin,   width,  height, zPosition);
          FillVector3D(rightDV,     -1,       0,         0);
          FillVector3D(bottomDV,     0,      -1,         0);
        }
      }
      else
      {
        if(rotated==false)
        {
          FillVector3D(origin,   width,  0, zPosition);
          FillVector3D(rightDV,     -1,  0,         0);
          FillVector3D(bottomDV,     0,  1,         0);
        }
        else
        {
          FillVector3D(origin,   0,  height, zPosition);
          FillVector3D(rightDV,  1,       0,         0);
          FillVector3D(bottomDV, 0,      -1,         0);
        }
      }
      normalDirection = 2;
      break;
    case Frontal:
      if(frontside)
      {
        if(rotated==false)
        {
          FillVector3D(origin,   0, zPosition, 0);
          FillVector3D(rightDV,  1, 0,         0);
          FillVector3D(bottomDV, 0, 0,         1);
        }
        else
        {
          FillVector3D(origin,   width, zPosition, height);
          FillVector3D(rightDV,     -1,         0,      0);
          FillVector3D(bottomDV,     0,         0,     -1);
        }
      }
      else
      {
        if(rotated==false)
        {
          FillVector3D(origin,    width, zPosition,  0);
          FillVector3D(rightDV,      -1,         0,  0);
          FillVector3D(bottomDV,      0,         0,  1);
        }
        else
        {
          FillVector3D(origin,   0, zPosition,  height);
          FillVector3D(rightDV,  1,         0,       0);
          FillVector3D(bottomDV, 0,         0,      -1);
        }
      }
      normalDirection = 1;
      break;
    case Sagittal:
      if(frontside)
      {
        if(rotated==false)
        {
          FillVector3D(origin,   zPosition, 0, 0);
          FillVector3D(rightDV,  0,         1, 0);
          FillVector3D(bottomDV, 0,         0, 1);
        }
        else
        {
          FillVector3D(origin,   zPosition, width, height);
          FillVector3D(rightDV,          0,    -1,      0);
          FillVector3D(bottomDV,         0,     0,     -1);
        }
      }
      else
      {
        if(rotated==false)
        {
          FillVector3D(origin,   zPosition,  width, 0);
          FillVector3D(rightDV,          0,     -1, 0);
          FillVector3D(bottomDV,         0,      0, 1);
        }
        else
        {
          FillVector3D(origin,   zPosition,  0, height);
          FillVector3D(rightDV,          0,  1,      0);
          FillVector3D(bottomDV,         0,  0,     -1);
        }
      }
      normalDirection = 0;
      break;
    default:
      itkExceptionMacro("unknown PlaneOrientation");
  }
  if(transform!=NULL)
  {
    origin   = transform->TransformPoint(origin);
    rightDV  = transform->TransformVector(rightDV);
    bottomDV = transform->TransformVector(bottomDV);
  }

  ScalarType bounds[6]={0, width, 0, height, 0, 1};
  SetBounds(bounds);

  if(transform==NULL)
    SetMatrixByVectors(rightDV, bottomDV);
  else
    SetMatrixByVectors(rightDV, bottomDV, transform->GetMatrix().GetVnlMatrix().get_column(normalDirection).magnitude());

  SetOrigin(origin);
}

void mitk::PlaneGeometry::InitializeStandardPlane(const mitk::Geometry3D* geometry3D, PlaneOrientation planeorientation, mitk::ScalarType zPosition, bool frontside, bool rotated)
{
  mitk::ScalarType width, height;

  const BoundingBox::BoundsArrayType& boundsarray = geometry3D->GetBoundingBox()->GetBounds();
  mitk::Vector3D  originVector; 
  FillVector3D(originVector,  boundsarray[0], boundsarray[2], boundsarray[4]);
  if(geometry3D->GetImageGeometry())
  {
    FillVector3D(originVector, originVector[0]-0.5, originVector[1]-0.5, originVector[2]-0.5);
  }
  switch(planeorientation)
  {
    case Transversal:
      width  = geometry3D->GetExtent(0);
      height = geometry3D->GetExtent(1);
      break;
    case Frontal:
      width  = geometry3D->GetExtent(0);
      height = geometry3D->GetExtent(2);
      break;
    case Sagittal:
      width  = geometry3D->GetExtent(1);
      height = geometry3D->GetExtent(2);
      break;
    default:
      itkExceptionMacro("unknown PlaneOrientation");
  }

  InitializeStandardPlane(width, height, geometry3D->GetIndexToWorldTransform(), planeorientation, zPosition, frontside, rotated);

  ScalarType bounds[6]={0, width, 0, height, 0, 1};
  SetBounds(bounds);

  mitk::Point3D  origin; 
  originVector = geometry3D->GetIndexToWorldTransform()->TransformVector(originVector);
  origin = GetOrigin()+originVector;
  SetOrigin(origin);
}

void mitk::PlaneGeometry::InitializeStandardPlane(const mitk::Geometry3D* geometry3D, bool top, PlaneOrientation planeorientation, bool frontside, bool rotated)
{
  mitk::ScalarType zPosition;

  switch(planeorientation)
  {
    case Transversal:
      zPosition = (top ? 0 : geometry3D->GetExtent(2)-1+0.5);
      break;
    case Frontal:
      zPosition = (top ? 0 : geometry3D->GetExtent(1)-1+0.5);
      break;
    case Sagittal:
      zPosition = (top ? 0 : geometry3D->GetExtent(0)-1+0.5);
      break;
    default:
      itkExceptionMacro("unknown PlaneOrientation");
  }

  InitializeStandardPlane(geometry3D, planeorientation, zPosition, frontside, rotated);
}

void mitk::PlaneGeometry::InitializeStandardPlane(const Vector3D& rightVector, const Vector3D& downVector, const Vector3D * spacing)
{
  InitializeStandardPlane(rightVector.Get_vnl_vector(), downVector.Get_vnl_vector(), spacing);
}

void mitk::PlaneGeometry::InitializeStandardPlane(const VnlVector& rightVector, const VnlVector& downVector, const Vector3D * spacing)
{
  mitk::ScalarType width  = rightVector.magnitude();
  mitk::ScalarType height = downVector.magnitude();

  InitializeStandardPlane(width, height, rightVector, downVector, spacing);
}

void mitk::PlaneGeometry::InitializeStandardPlane(mitk::ScalarType width, mitk::ScalarType height, const Vector3D& rightVector, const Vector3D& downVector, const Vector3D * spacing)
{
  InitializeStandardPlane(width, height, rightVector.Get_vnl_vector(), downVector.Get_vnl_vector(), spacing);
}

void mitk::PlaneGeometry::InitializeStandardPlane(mitk::ScalarType width, mitk::ScalarType height, const VnlVector& rightVector, const VnlVector& downVector, const Vector3D * spacing)
{
  assert(width > 0);
  assert(height > 0);

  VnlVector rightDV = rightVector; rightDV.normalize();
  VnlVector downDV  = downVector;  downDV.normalize();
  VnlVector normal  = vnl_cross_3d(rightVector, downVector); normal.normalize();

  if(spacing!=NULL)
  {
    rightDV *= (*spacing)[0];
    downDV  *= (*spacing)[1];
    normal  *= (*spacing)[2];
  }

  mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
  mitk::Matrix3D matrix;
  matrix.GetVnlMatrix().set_column(0, rightDV);
  matrix.GetVnlMatrix().set_column(1, downDV);
  matrix.GetVnlMatrix().set_column(2, normal);
  transform->SetMatrix(matrix);
  transform->SetOffset(m_IndexToWorldTransform->GetOffset());
  ScalarType bounds[6]={0, width, 0, height, 0, 1};
  SetBounds(bounds);
  SetIndexToWorldTransform(transform);
}

void mitk::PlaneGeometry::InitializePlane(const mitk::Point3D& origin, const mitk::Vector3D& normal)
{
  VnlVector rightVectorVnl(3), downVectorVnl;

  if( mitk::Equal( normal[1], 0.0f ) == false )
  {
    FillVector3D( rightVectorVnl, 1.0f, -normal[0]/normal[1], 0.0f ); 
    rightVectorVnl.normalize();
  }
  else
  {
    FillVector3D( rightVectorVnl, 0.0f, 1.0f, 0.0f );
  }
  downVectorVnl = vnl_cross_3d( normal.Get_vnl_vector(), rightVectorVnl );
  downVectorVnl.normalize();

  InitializeStandardPlane( rightVectorVnl, downVectorVnl );

  SetOrigin(origin);
}

mitk::ScalarType mitk::PlaneGeometry::SignedDistance(const mitk::Point3D& pt3d_mm) const
{
  return SignedDistanceFromPlane(pt3d_mm);
}

bool mitk::PlaneGeometry::IsAbove(const mitk::Point3D& pt3d_mm) const
{
  return SignedDistanceFromPlane(pt3d_mm)>0;
}

mitk::AffineGeometryFrame3D::Pointer mitk::PlaneGeometry::Clone() const
{
  Self::Pointer newGeometry = Self::New();
  newGeometry->Initialize();
  InitializeGeometry(newGeometry);
  return newGeometry.GetPointer();
}

void mitk::PlaneGeometry::InitializeGeometry(Self * newGeometry) const
{
  Superclass::InitializeGeometry(newGeometry);
}

void mitk::PlaneGeometry::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  os << indent << " Normal: " << GetNormal() << std::endl;
}
