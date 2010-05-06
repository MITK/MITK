/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
#include "mitkPlaneOperation.h"
#include "mitkInteractionConst.h"
#include "mitkLine.h"

#include <vtkTransform.h>

#include <vnl/vnl_cross.h>


namespace mitk
{

mitk::PlaneGeometry::PlaneGeometry()
{
  Initialize();
}


mitk::PlaneGeometry::~PlaneGeometry()
{
}


void
PlaneGeometry::Initialize()
{
  Superclass::Initialize();
}


void
PlaneGeometry::EnsurePerpendicularNormal(mitk::AffineTransform3D *transform)
{
  //ensure row(2) of transform to be perpendicular to plane, keep length.
  VnlVector normal = vnl_cross_3d(
    transform->GetMatrix().GetVnlMatrix().get_column(0),
    transform->GetMatrix().GetVnlMatrix().get_column(1) );

  normal.normalize();
  ScalarType len = transform->GetMatrix()
    .GetVnlMatrix().get_column(2).two_norm();

  if (len==0) len = 1;
  normal*=len;
  Matrix3D matrix = transform->GetMatrix();
  matrix.GetVnlMatrix().set_column(2, normal);
  transform->SetMatrix(matrix);
}


void
PlaneGeometry::SetIndexToWorldTransform(mitk::AffineTransform3D *transform)
{
  EnsurePerpendicularNormal(transform);

  Superclass::SetIndexToWorldTransform(transform);
}


void
PlaneGeometry::SetBounds(const BoundingBox::BoundsArrayType &bounds)
{
  //currently the unit rectangle must be starting at the origin [0,0]
  assert(bounds[0]==0);
  assert(bounds[2]==0);
  //the unit rectangle must be two-dimensional
  assert(bounds[1]>0);
  assert(bounds[3]>0);

  Superclass::SetBounds(bounds);
}


void 
PlaneGeometry::IndexToWorld( const Point2D &pt_units, Point2D &pt_mm ) const
{
  pt_mm[0]=m_ScaleFactorMMPerUnitX*pt_units[0];
  pt_mm[1]=m_ScaleFactorMMPerUnitY*pt_units[1];
  
  if (m_ImageGeometry)
  {
    pt_mm[0]-=0.5;
    pt_mm[1]-=0.5;
  }
}


void PlaneGeometry::WorldToIndex( const Point2D &pt_mm, Point2D &pt_units ) const
{
  pt_units[0]=pt_mm[0]*(1.0/m_ScaleFactorMMPerUnitX);
  pt_units[1]=pt_mm[1]*(1.0/m_ScaleFactorMMPerUnitY);

  if (m_ImageGeometry)
  {
    pt_units[0]+=0.5;
    pt_units[1]+=0.5;
  }
}


void PlaneGeometry::IndexToWorld( const Point2D &/*atPt2d_units*/, 
  const Vector2D &vec_units, Vector2D &vec_mm) const
{
  vec_mm[0] = m_ScaleFactorMMPerUnitX * vec_units[0];
  vec_mm[1] = m_ScaleFactorMMPerUnitY * vec_units[1];
}


void
PlaneGeometry::WorldToIndex( const Point2D &/*atPt2d_mm*/,
  const Vector2D &vec_mm, Vector2D &vec_units) const
{
  vec_units[0] = vec_mm[0] * ( 1.0 / m_ScaleFactorMMPerUnitX );
  vec_units[1] = vec_mm[1] * ( 1.0 / m_ScaleFactorMMPerUnitY );
}


void
PlaneGeometry::InitializeStandardPlane( mitk::ScalarType width, 
  ScalarType height, const Vector3D & spacing, 
  PlaneGeometry::PlaneOrientation planeorientation, 
  ScalarType zPosition, bool frontside, bool rotated )
{
  AffineTransform3D::Pointer transform;

  transform = AffineTransform3D::New();
  AffineTransform3D::MatrixType matrix;
  AffineTransform3D::MatrixType::InternalMatrixType &vnlmatrix = 
    matrix.GetVnlMatrix();

  vnlmatrix.set_identity();
  vnlmatrix(0,0) = spacing[0];
  vnlmatrix(1,1) = spacing[1];
  vnlmatrix(2,2) = spacing[2];
  transform->SetIdentity();
  transform->SetMatrix(matrix);
  
  InitializeStandardPlane(width, height, transform.GetPointer(), 
    planeorientation, zPosition, frontside, rotated);
}


void
PlaneGeometry::InitializeStandardPlane( mitk::ScalarType width, 
  ScalarType height, const AffineTransform3D* transform,
  PlaneGeometry::PlaneOrientation planeorientation, ScalarType zPosition, 
  bool frontside, bool rotated )
{
  Superclass::Initialize();

  //construct standard view
  Point3D origin; 
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
  if ( transform != NULL )
  {
    origin = transform->TransformPoint( origin );
    rightDV = transform->TransformVector( rightDV );
    bottomDV = transform->TransformVector( bottomDV );
  }

  ScalarType bounds[6]= { 0, width, 0, height, 0, 1 };
  this->SetBounds( bounds );

  if ( transform == NULL )
  {
    this->SetMatrixByVectors( rightDV, bottomDV );
  }
  else
  {
    this->SetMatrixByVectors(
      rightDV, bottomDV,
      transform->GetMatrix().GetVnlMatrix()
        .get_column(normalDirection).magnitude()
    );
  }

  this->SetOrigin(origin);
}


void
PlaneGeometry::InitializeStandardPlane( const Geometry3D *geometry3D, 
  PlaneOrientation planeorientation, ScalarType zPosition, 
  bool frontside, bool rotated )
{
  this->SetReferenceGeometry( const_cast< Geometry3D * >( geometry3D ) );

  ScalarType width, height;

  const BoundingBox::BoundsArrayType& boundsarray = 
    geometry3D->GetBoundingBox()->GetBounds();

  Vector3D  originVector; 
  FillVector3D(originVector,  boundsarray[0], boundsarray[2], boundsarray[4]);
  if(geometry3D->GetImageGeometry())
  {
    FillVector3D( originVector,
      originVector[0] - 0.5, 
      originVector[1] - 0.5, 
      originVector[2] - 0.5 );
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

  InitializeStandardPlane( width, height, 
    geometry3D->GetIndexToWorldTransform(),
    planeorientation, zPosition, frontside, rotated );

  ScalarType bounds[6]= { 0, width, 0, height, 0, 1 };
  this->SetBounds( bounds );

  Point3D origin; 
  originVector = geometry3D->GetIndexToWorldTransform()
    ->TransformVector( originVector );

  origin = GetOrigin() + originVector;
  SetOrigin(origin);
}


void 
PlaneGeometry::InitializeStandardPlane( const Geometry3D *geometry3D, 
  bool top, PlaneOrientation planeorientation, bool frontside, bool rotated )
{
  ScalarType zPosition;

  switch(planeorientation)
  {
    case Transversal:
      zPosition = (top ? 0.5 : geometry3D->GetExtent(2)-1+0.5);
      break;
    case Frontal:
      zPosition = (top ? 0.5 : geometry3D->GetExtent(1)-1+0.5);
      break;
    case Sagittal:
      zPosition = (top ? 0.5 : geometry3D->GetExtent(0)-1+0.5);
      break;
    default:
      itkExceptionMacro("unknown PlaneOrientation");
  }

  InitializeStandardPlane( geometry3D, planeorientation, 
    zPosition, frontside, rotated );
}


void
PlaneGeometry::InitializeStandardPlane( const Vector3D &rightVector, 
  const Vector3D &downVector, const Vector3D *spacing )
{
  InitializeStandardPlane( rightVector.Get_vnl_vector(), 
    downVector.Get_vnl_vector(), spacing );
}


void
PlaneGeometry::InitializeStandardPlane( const VnlVector& rightVector, 
  const VnlVector &downVector, const Vector3D *spacing )
{
  ScalarType width  = rightVector.magnitude();
  ScalarType height = downVector.magnitude();

  InitializeStandardPlane( width, height, rightVector, downVector, spacing );
}


void 
PlaneGeometry::InitializeStandardPlane( mitk::ScalarType width, 
  ScalarType height, const Vector3D &rightVector, const Vector3D &downVector, 
  const Vector3D *spacing )
{
  InitializeStandardPlane( 
    width, height, 
    rightVector.Get_vnl_vector(), downVector.Get_vnl_vector(), 
    spacing );
}


void 
PlaneGeometry::InitializeStandardPlane( 
  mitk::ScalarType width, ScalarType height, 
  const VnlVector &rightVector, const VnlVector &downVector, 
  const Vector3D *spacing )
{
  assert(width > 0);
  assert(height > 0);

  VnlVector rightDV = rightVector; rightDV.normalize();
  VnlVector downDV  = downVector;  downDV.normalize();
  VnlVector normal  = vnl_cross_3d(rightVector, downVector); 
  normal.normalize();

  if(spacing!=NULL)
  {
    rightDV *= (*spacing)[0];
    downDV  *= (*spacing)[1];
    normal  *= (*spacing)[2];
  }

  AffineTransform3D::Pointer transform = AffineTransform3D::New();
  Matrix3D matrix;
  matrix.GetVnlMatrix().set_column(0, rightDV);
  matrix.GetVnlMatrix().set_column(1, downDV);
  matrix.GetVnlMatrix().set_column(2, normal);
  transform->SetMatrix(matrix);
  transform->SetOffset(m_IndexToWorldTransform->GetOffset());

  ScalarType bounds[6] = { 0, width, 0, height, 0, 1 };
  this->SetBounds( bounds );

  this->SetIndexToWorldTransform( transform );
}


void 
PlaneGeometry::InitializePlane( const Point3D &origin,
  const Vector3D &normal )
{
  VnlVector rightVectorVnl(3), downVectorVnl;

  if( Equal( normal[1], 0.0f ) == false )
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


void 
PlaneGeometry::SetMatrixByVectors( const VnlVector &rightVector, 
  const VnlVector &downVector, ScalarType thickness )
{
  VnlVector normal = vnl_cross_3d(rightVector, downVector);
  normal.normalize();
  normal *= thickness;

  AffineTransform3D::Pointer transform = AffineTransform3D::New();
  Matrix3D matrix;
  matrix.GetVnlMatrix().set_column(0, rightVector);
  matrix.GetVnlMatrix().set_column(1, downVector);
  matrix.GetVnlMatrix().set_column(2, normal);
  transform->SetMatrix(matrix);
  transform->SetOffset(m_IndexToWorldTransform->GetOffset());
  SetIndexToWorldTransform(transform);
}


Vector3D 
PlaneGeometry::GetNormal() const
{
  Vector3D frontToBack;
  frontToBack.Set_vnl_vector( m_IndexToWorldTransform
    ->GetMatrix().GetVnlMatrix().get_column(2) );

  return frontToBack;
}


VnlVector
PlaneGeometry::GetNormalVnl() const
{
  return m_IndexToWorldTransform
    ->GetMatrix().GetVnlMatrix().get_column(2);
}


ScalarType 
PlaneGeometry::DistanceFromPlane( const Point3D &pt3d_mm ) const
{
  return fabs(SignedDistance( pt3d_mm ));
}


ScalarType
PlaneGeometry::SignedDistance( const Point3D &pt3d_mm ) const
{
  return SignedDistanceFromPlane(pt3d_mm);
}


bool
PlaneGeometry::IsAbove( const Point3D &pt3d_mm ) const
{
  return SignedDistanceFromPlane(pt3d_mm) > 0;
}


bool
PlaneGeometry::IntersectionLine( 
  const PlaneGeometry* plane, Line3D& crossline ) const
{
  Vector3D normal = this->GetNormal();
  normal.Normalize();

  Vector3D planeNormal = plane->GetNormal();
  planeNormal.Normalize();

  Vector3D direction = itk::CrossProduct( normal, planeNormal );

  if ( direction.GetSquaredNorm() < eps )
    return false; 

  crossline.SetDirection( direction );

  double N1dN2 = normal * planeNormal;
  double determinant = 1.0 - N1dN2 * N1dN2;

  Vector3D origin = this->GetOrigin().GetVectorFromOrigin();
  Vector3D planeOrigin = plane->GetOrigin().GetVectorFromOrigin();

  double d1 = normal * origin;
  double d2 = planeNormal * planeOrigin;

  double c1 = ( d1 - d2 * N1dN2 ) / determinant;
  double c2 = ( d2 - d1 * N1dN2 ) / determinant;

  Vector3D p = normal * c1 + planeNormal * c2;
  crossline.GetPoint().Get_vnl_vector() = p.Get_vnl_vector();

  return true;
}


unsigned int
PlaneGeometry::IntersectWithPlane2D(
  const PlaneGeometry* plane, Point2D& lineFrom, Point2D &lineTo ) const
{
  Line3D crossline;
  if ( this->IntersectionLine( plane, crossline ) == false )
    return 0;

  Point2D  point2; 
  Vector2D direction2; 

  this->Map( crossline.GetPoint(), point2 );
  this->Map( crossline.GetPoint(), crossline.GetDirection(), direction2 );

  return 
    Line3D::RectangleLineIntersection( 
    0, 0, GetExtentInMM(0), GetExtentInMM(1), 
    point2, direction2, lineFrom, lineTo );
}


double PlaneGeometry::Angle( const PlaneGeometry *plane ) const
{
  return angle(plane->GetMatrixColumn(2), GetMatrixColumn(2));
}


double PlaneGeometry::Angle( const Line3D &line ) const
{
  return vnl_math::pi_over_2
    - angle( line.GetDirection().Get_vnl_vector(), GetMatrixColumn(2) );
}


bool PlaneGeometry::IntersectionPoint( 
  const Line3D &line, Point3D &intersectionPoint ) const
{
  Vector3D planeNormal = this->GetNormal();
  planeNormal.Normalize();

  Vector3D lineDirection = line.GetDirection();
  lineDirection.Normalize();

  double t = planeNormal * lineDirection;
  if ( fabs( t ) < eps )
  {
    return false;
  }

  Vector3D diff;
  diff = this->GetOrigin() - line.GetPoint();
  t = ( planeNormal * diff ) / t;

  intersectionPoint = line.GetPoint() + line.GetDirection() * t;
  return true;
}


bool
PlaneGeometry::IntersectionPointParam( const Line3D &line, double &t ) const
{
  Vector3D planeNormal = this->GetNormal();
  planeNormal.Normalize();

  Vector3D lineDirection = line.GetDirection();
  lineDirection.Normalize();

  t = planeNormal * lineDirection;

  if ( fabs( t ) < eps )
  {
    return false;
  }

  Vector3D diff;
  diff = this->GetOrigin() - line.GetPoint();
  t = ( planeNormal * diff  ) / t;
  return true;
}


bool
PlaneGeometry::IsParallel( const PlaneGeometry *plane ) const 
{
  return ( (Angle(plane) < eps ) || ( Angle(plane) > ( vnl_math::pi - eps ) ) ) ;
} 


bool
PlaneGeometry::IsOnPlane( const Point3D &point ) const
{
  return Distance(point) < eps;
}


bool
PlaneGeometry::IsOnPlane( const Line3D &line ) const
{
  return ( (Distance( line.GetPoint() ) < eps)
    && (Distance( line.GetPoint2() ) < eps) );
}


bool 
PlaneGeometry::IsOnPlane( const PlaneGeometry *plane ) const 
{
  return ( IsParallel( plane ) && (Distance( plane->GetOrigin() ) < eps) );
}


Point3D
PlaneGeometry::ProjectPointOntoPlane( const Point3D& pt ) const
{
  ScalarType len = this->GetNormalVnl().two_norm();
  return pt - this->GetNormal() * this->SignedDistanceFromPlane( pt ) / len;
}


AffineGeometryFrame3D::Pointer
PlaneGeometry::Clone() const
{
  Self::Pointer newGeometry = Self::New();
  newGeometry->Initialize();
  InitializeGeometry(newGeometry);
  return newGeometry.GetPointer();
}


void 
PlaneGeometry::ExecuteOperation( Operation *operation )
{
  vtkTransform *transform = vtkTransform::New();
  transform->SetMatrix( m_VtkMatrix );

  switch ( operation->GetOperationType() )
  {
  case OpORIENT:
    {
      mitk::PlaneOperation *planeOp = dynamic_cast< mitk::PlaneOperation * >( operation );
      if ( planeOp == NULL )
      {
        return;
      }

      Point3D center = planeOp->GetPoint();

      Vector3D orientationVector = planeOp->GetNormal();
      Vector3D defaultVector;
      FillVector3D( defaultVector, 0.0, 0.0, 1.0 );

      Vector3D rotationAxis = itk::CrossProduct( orientationVector, defaultVector );
      //vtkFloatingPointType rotationAngle = acos( orientationVector[2] / orientationVector.GetNorm() );

      vtkFloatingPointType rotationAngle = atan2( (double) rotationAxis.GetNorm(), (double) (orientationVector * defaultVector) );
      rotationAngle *= 180.0 / vnl_math::pi;

      transform->PostMultiply();
      transform->Identity();
      transform->Translate( center[0], center[1], center[2] );
      transform->RotateWXYZ( rotationAngle, rotationAxis[0], rotationAxis[1], rotationAxis[2] );
      transform->Translate( -center[0], -center[1], -center[2] );
      break;
    }


  default:
    Superclass::ExecuteOperation( operation );
    transform->Delete();
    return;
  }

  m_VtkMatrix->DeepCopy(transform->GetMatrix());
  this->TransferVtkToItkTransform();
  this->Modified();
  transform->Delete();
}


void PlaneGeometry::InitializeGeometry( Self *newGeometry ) const
{
  Superclass::InitializeGeometry(newGeometry);
}


void PlaneGeometry::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
  Superclass::PrintSelf(os,indent);
  os << indent << " Normal: " << GetNormal() << std::endl;
}


} // namespace
