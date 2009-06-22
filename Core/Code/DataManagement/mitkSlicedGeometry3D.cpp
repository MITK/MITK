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

#include "mitkSlicedGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkRotationOperation.h"
#include "mitkPlaneOperation.h"
#include "mitkInteractionConst.h"
#include "mitkSliceNavigationController.h"


mitk::SlicedGeometry3D::SlicedGeometry3D()
: m_EvenlySpaced( true ),
  m_Slices( 0 ),
  m_ReferenceGeometry( NULL ),
  m_SliceNavigationController( NULL )
{
  this->Initialize( m_Slices );
}


mitk::SlicedGeometry3D::~SlicedGeometry3D()
{
}


mitk::Geometry2D *
mitk::SlicedGeometry3D::GetGeometry2D( int s ) const
{
  mitk::Geometry2D::Pointer geometry2D = NULL;
  
  if ( this->IsValidSlice(s) )
  {
    geometry2D = m_Geometry2Ds[s];

    // If (a) m_EvenlySpaced==true, (b) we don't have a Geometry2D stored
    // for the requested slice, and (c) the first slice (s=0) 
    // is a PlaneGeometry instance, then we calculate the geometry of the
    // requested as the plane of the first slice shifted by m_Spacing[2]*s
    // in the direction of m_DirectionVector.
    if ( (m_EvenlySpaced) && (geometry2D.IsNull()) )
    {
      PlaneGeometry *firstSlice = dynamic_cast< PlaneGeometry * > (
        m_Geometry2Ds[0].GetPointer() );

      if ( firstSlice != NULL )
      {
        if ( (m_DirectionVector[0] == 0.0)
          && (m_DirectionVector[1] == 0.0)
          && (m_DirectionVector[2] == 0.0) )
        {
          m_DirectionVector = firstSlice->GetNormal();
          m_DirectionVector.Normalize();
        }

        Vector3D direction;
        direction = m_DirectionVector * m_Spacing[2];

        mitk::PlaneGeometry::Pointer requestedslice;
        requestedslice = static_cast< mitk::PlaneGeometry * >(
          firstSlice->Clone().GetPointer() );

        requestedslice->SetOrigin(
          requestedslice->GetOrigin() + direction * s );

        geometry2D = requestedslice;
        m_Geometry2Ds[s] = geometry2D;
      }
    }
    return geometry2D;
  }
  else
  {
    return NULL;
  }
}

const mitk::BoundingBox *
mitk::SlicedGeometry3D::GetBoundingBox() const
{
  assert(m_BoundingBox.IsNotNull());
  return m_BoundingBox.GetPointer();
}


bool
mitk::SlicedGeometry3D::SetGeometry2D( mitk::Geometry2D *geometry2D, int s )
{
  if ( this->IsValidSlice(s) )
  {
    m_Geometry2Ds[s] = geometry2D;
    m_Geometry2Ds[s]->SetReferenceGeometry( m_ReferenceGeometry );
    return true;
  }
  return false;
}


void
mitk::SlicedGeometry3D::Initialize( unsigned int slices )
{
  Superclass::Initialize();
  m_Slices = slices;

  Geometry2D::Pointer gnull = NULL;
  m_Geometry2Ds.assign( m_Slices, gnull );
 
  Vector3D spacing;
  spacing.Fill( 1.0 );
  this->SetSpacing( spacing );

  m_DirectionVector.Fill( 0 );
}


void 
mitk::SlicedGeometry3D::InitializeEvenlySpaced(
  mitk::Geometry2D* geometry2D, unsigned int slices, bool flipped )
{
  assert( geometry2D != NULL );
  this->InitializeEvenlySpaced(
    geometry2D, geometry2D->GetExtentInMM(2)/geometry2D->GetExtent(2),
    slices, flipped );
}


void
mitk::SlicedGeometry3D::InitializeEvenlySpaced(
  mitk::Geometry2D* geometry2D, mitk::ScalarType zSpacing,
  unsigned int slices, bool flipped )
{
  assert( geometry2D != NULL );
  assert( geometry2D->GetExtent(0) > 0 );
  assert( geometry2D->GetExtent(1) > 0 );

  geometry2D->Register();

  Superclass::Initialize();
  m_Slices = slices;

  BoundingBox::BoundsArrayType bounds = geometry2D->GetBounds();
  bounds[4] = 0;
  bounds[5] = slices;

  // clear and reserve
  Geometry2D::Pointer gnull = NULL;
  m_Geometry2Ds.assign( m_Slices, gnull );

  Vector3D directionVector = geometry2D->GetAxisVector(2);
  directionVector.Normalize();
  directionVector *= zSpacing;

  if ( flipped == false )
  {
    // Normally we should use the following four lines to create a copy of
    // the transform contrained in geometry2D, because it may not be changed
    // by us. But we know that SetSpacing creates a new transform without
    // changing the old (coming from geometry2D), so we can use the fifth
    // line instead. We check this at (**).
    //
    // AffineTransform3D::Pointer transform = AffineTransform3D::New();
    // transform->SetMatrix(geometry2D->GetIndexToWorldTransform()->GetMatrix());
    // transform->SetOffset(geometry2D->GetIndexToWorldTransform()->GetOffset());
    // SetIndexToWorldTransform(transform);

    m_IndexToWorldTransform = const_cast< AffineTransform3D * >(
      geometry2D->GetIndexToWorldTransform() );
  }
  else
  {
    directionVector *= -1.0;
    m_IndexToWorldTransform = AffineTransform3D::New();
    m_IndexToWorldTransform->SetMatrix(
      geometry2D->GetIndexToWorldTransform()->GetMatrix() );

    AffineTransform3D::OutputVectorType scaleVector;
    FillVector3D(scaleVector, 1.0, 1.0, -1.0);
    m_IndexToWorldTransform->Scale(scaleVector, true);
    m_IndexToWorldTransform->SetOffset(
      geometry2D->GetIndexToWorldTransform()->GetOffset() );
  }

  mitk::Vector3D spacing;
  FillVector3D( spacing,
    geometry2D->GetExtentInMM(0) / bounds[1],
    geometry2D->GetExtentInMM(1) / bounds[3],
    zSpacing );

  // Ensure that spacing differs from m_Spacing to make SetSpacing change the
  // matrix.
  m_Spacing[2] = zSpacing - 1;

  this->SetDirectionVector( directionVector );
  this->SetBounds( bounds );
  this->SetGeometry2D( geometry2D, 0 );
  this->SetSpacing( spacing );
  this->SetEvenlySpaced();
  this->SetTimeBounds( geometry2D->GetTimeBounds() );

  assert(m_IndexToWorldTransform.GetPointer()
         != geometry2D->GetIndexToWorldTransform()); // (**) see above.

  this->SetFrameOfReferenceID( geometry2D->GetFrameOfReferenceID() );
  this->SetImageGeometry( geometry2D->GetImageGeometry() );

  geometry2D->UnRegister();
}


void
mitk::SlicedGeometry3D::InitializePlanes(
  const mitk::Geometry3D *geometry3D,
  mitk::PlaneGeometry::PlaneOrientation planeorientation,
  bool top, bool frontside, bool rotated )
{
  m_ReferenceGeometry = const_cast< Geometry3D * >( geometry3D );
  
  PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane(
    geometry3D, top, planeorientation, frontside, rotated );

  ScalarType viewSpacing = 1;
  unsigned int slices = 1;

  switch ( planeorientation )
  {
    case PlaneGeometry::Transversal:
      viewSpacing = geometry3D->GetSpacing()[2];
      slices = (unsigned int) geometry3D->GetExtent( 2 );
      break;

    case PlaneGeometry::Frontal:
      viewSpacing = geometry3D->GetSpacing()[1];
      slices = (unsigned int) geometry3D->GetExtent( 1 );
      break;

    case PlaneGeometry::Sagittal:
      viewSpacing = geometry3D->GetSpacing()[0];
      slices = (unsigned int) geometry3D->GetExtent( 0 );
      break;

    default:
      itkExceptionMacro("unknown PlaneOrientation");
  }

  mitk::Vector3D normal = this->AdjustNormal( planeGeometry->GetNormal() );


  ScalarType directedExtent =
      fabs( m_ReferenceGeometry->GetExtentInMM( 0 ) * normal[0] )
    + fabs( m_ReferenceGeometry->GetExtentInMM( 1 ) * normal[1] )
    + fabs( m_ReferenceGeometry->GetExtentInMM( 2 ) * normal[2] );

  if ( directedExtent >= viewSpacing )
  {
    slices = static_cast< int >(directedExtent / viewSpacing + 0.5);
  }
  else
  {
    slices = 1;
  }

  bool flipped = (top == false);
  
  if ( frontside == false )
  {
    flipped = !flipped;
  }
  if ( planeorientation == PlaneGeometry::Frontal )
  {
    flipped = !flipped;
  }

  this->InitializeEvenlySpaced( planeGeometry, viewSpacing, slices, flipped );
}


void
mitk::SlicedGeometry3D
::ReinitializePlanes( const Point3D &center, const Point3D &referencePoint )
{
  // Need a reference frame to align the rotated planes
  if ( !m_ReferenceGeometry )
  {
    return;
  }

  // Get first plane of plane stack
  PlaneGeometry *firstPlane = 
    dynamic_cast< PlaneGeometry * >( m_Geometry2Ds[0].GetPointer() );

  // If plane stack is empty, exit
  if ( firstPlane == NULL )
  {
    return;
  }

  // Calculate the "directed" spacing when taking the plane (defined by its axes
  // vectors and normal) as the reference coordinate frame.
  //
  // This is done by calculating the radius of the ellipsoid defined by the
  // original volume spacing axes, in the direction of the respective axis of the
  // reference frame.
  mitk::Vector3D axis0 = firstPlane->GetAxisVector(0);
  mitk::Vector3D axis1 = firstPlane->GetAxisVector(1);
  mitk::Vector3D normal = firstPlane->GetNormal();
  normal.Normalize();

  Vector3D spacing;
  spacing[0] = this->CalculateSpacing( axis0 );
  spacing[1] = this->CalculateSpacing( axis1 );
  spacing[2] = this->CalculateSpacing( normal );

  Superclass::SetSpacing( spacing );

  
  // Now we need to calculate the number of slices in the plane's normal
  // direction, so that the entire volume is covered. This is done by first
  // calculating the dot product between the volume diagonal (the maximum
  // distance inside the volume) and the normal, and dividing this value by
  // the directed spacing calculated above.
  ScalarType directedExtent =
      fabs( m_ReferenceGeometry->GetExtentInMM( 0 ) * normal[0] )
    + fabs( m_ReferenceGeometry->GetExtentInMM( 1 ) * normal[1] )
    + fabs( m_ReferenceGeometry->GetExtentInMM( 2 ) * normal[2] );

  if ( directedExtent >= spacing[2] )
  {
    m_Slices = static_cast< unsigned int >(directedExtent / spacing[2] + 0.5);
  }
  else
  {
    m_Slices = 1;
  }

  // The origin of our "first plane" needs to be adapted to this new extent.
  // To achieve this, we first calculate the current distance to the volume's
  // center, and then shift the origin in the direction of the normal by the
  // difference between this distance and half of the new extent.
  double centerOfRotationDistance =
    firstPlane->SignedDistanceFromPlane( center );
    
  if ( centerOfRotationDistance > 0 )
  {
    firstPlane->SetOrigin( firstPlane->GetOrigin()
      + normal * (centerOfRotationDistance - directedExtent / 2.0)
    );
    m_DirectionVector = normal;
  }
  else
  {
    firstPlane->SetOrigin( firstPlane->GetOrigin()
      + normal * (directedExtent / 2.0 + centerOfRotationDistance)
    );
    m_DirectionVector = -normal;
  }

  // Now we adjust this distance according with respect to the given reference
  // point: we need to make sure that the point is touched by one slice of the
  // new slice stack.
  double referencePointDistance =
    firstPlane->SignedDistanceFromPlane( referencePoint );

  int referencePointSlice = static_cast< int >(
    referencePointDistance / spacing[2]);

  double alignmentValue =
    referencePointDistance / spacing[2] - referencePointSlice;

  firstPlane->SetOrigin(
    firstPlane->GetOrigin() + normal * alignmentValue * spacing[2] );

  
  // Finally, we can clear the previous geometry stack and initialize it with
  // our re-initialized "first plane".
  m_Geometry2Ds.assign( m_Slices, Geometry2D::Pointer( NULL ) );

  if ( m_Slices > 0 )
  {
    m_Geometry2Ds[0] = firstPlane;
  }

  // Reinitialize SNC with new number of slices
  m_SliceNavigationController->GetSlice()->SetSteps( m_Slices );

  this->Modified();
}


double
mitk::SlicedGeometry3D::CalculateSpacing( const mitk::Vector3D &d ) const
{
  // Need the spacing of the underlying dataset / geometry
  if ( !m_ReferenceGeometry )
  {
    return 1.0;
  }

  // The following can be derived from the ellipsoid equation
  //
  //   1 = x^2/a^2 + y^2/b^2 + z^2/c^2
  //
  // where (a,b,c) = spacing of original volume (ellipsoid radii)
  // and   (x,y,z) = scaled coordinates of vector d (according to ellipsoid)
  //
  const mitk::Vector3D &spacing = m_ReferenceGeometry->GetSpacing();

  double scaling = d[0]*d[0] / (spacing[0] * spacing[0])
    + d[1]*d[1] / (spacing[1] * spacing[1])
    + d[2]*d[2] / (spacing[2] * spacing[2]);

  scaling = sqrt( scaling );

  return ( sqrt( d[0]*d[0] + d[1]*d[1] + d[2]*d[2] ) / scaling );
}

mitk::Vector3D 
mitk::SlicedGeometry3D::AdjustNormal( const mitk::Vector3D &normal ) const
{
  Geometry3D::TransformType::Pointer inverse = Geometry3D::TransformType::New();
  m_ReferenceGeometry->GetIndexToWorldTransform()->GetInverse( inverse );
  
  Vector3D transformedNormal = inverse->TransformVector( normal );

  transformedNormal.Normalize();
  return transformedNormal;
}


void
mitk::SlicedGeometry3D::SetImageGeometry( const bool isAnImageGeometry )
{
  Superclass::SetImageGeometry( isAnImageGeometry );

  mitk::Geometry3D* geometry;
  
  unsigned int s;
  for ( s = 0; s < m_Slices; ++s )
  {
    geometry = m_Geometry2Ds[s];
    if ( geometry!=NULL )
    {
      geometry->SetImageGeometry( isAnImageGeometry );
    }
  }
}

bool
mitk::SlicedGeometry3D::IsValidSlice( int s ) const
{
  return ((s >= 0) && (s < (int)m_Slices));
}

void
mitk::SlicedGeometry3D::SetReferenceGeometry( Geometry3D *referenceGeometry )
{
  m_ReferenceGeometry = referenceGeometry;

  std::vector<Geometry2D::Pointer>::iterator it;

  for ( it = m_Geometry2Ds.begin(); it != m_Geometry2Ds.end(); ++it )
  {
    (*it)->SetReferenceGeometry( referenceGeometry );
  }
}

void
mitk::SlicedGeometry3D::SetSpacing( const mitk::Vector3D &aSpacing )
{
  bool hasEvenlySpacedPlaneGeometry = false;
  mitk::Point3D origin;
  mitk::Vector3D rightDV, bottomDV;
  BoundingBox::BoundsArrayType bounds;

  assert(aSpacing[0]>0 && aSpacing[1]>0 && aSpacing[2]>0);

  // In case of evenly-spaced data: re-initialize instances of Geometry2D,
  // since the spacing influences them
  if ((m_EvenlySpaced) && (m_Geometry2Ds.size() > 0))
  {
    mitk::Geometry2D::ConstPointer firstGeometry =
      m_Geometry2Ds[0].GetPointer();

    const PlaneGeometry *planeGeometry =
      dynamic_cast< const PlaneGeometry * >( firstGeometry.GetPointer() );

    if (planeGeometry != NULL )
    {
      this->WorldToIndex( planeGeometry->GetOrigin(), origin );
      this->WorldToIndex( planeGeometry->GetOrigin(),
        planeGeometry->GetAxisVector(0), rightDV );
      this->WorldToIndex( planeGeometry->GetOrigin(),
        planeGeometry->GetAxisVector(1), bottomDV );

      bounds = planeGeometry->GetBounds();
      hasEvenlySpacedPlaneGeometry = true;
    }
  }

  Superclass::SetSpacing(aSpacing);

  mitk::Geometry2D::Pointer firstGeometry;

  // In case of evenly-spaced data: re-initialize instances of Geometry2D,
  // since the spacing influences them
  if ( hasEvenlySpacedPlaneGeometry )
  {
    //create planeGeometry according to new spacing
    this->IndexToWorld( origin, origin );
    this->IndexToWorld( origin, rightDV, rightDV );
    this->IndexToWorld( origin, bottomDV, bottomDV );

    mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();

    planeGeometry->SetReferenceGeometry( m_ReferenceGeometry );
    planeGeometry->InitializeStandardPlane(
      rightDV.Get_vnl_vector(), bottomDV.Get_vnl_vector(), &m_Spacing );
    planeGeometry->SetOrigin(origin);
    planeGeometry->SetBounds(bounds);

    firstGeometry = planeGeometry;
  }
  else if ( (m_EvenlySpaced) && (m_Geometry2Ds.size() > 0) )
  {
    firstGeometry = m_Geometry2Ds[0].GetPointer();
  }

  //clear and reserve
  Geometry2D::Pointer gnull=NULL;
  m_Geometry2Ds.assign(m_Slices, gnull);

  if ( m_Slices > 0 )
  {
    m_Geometry2Ds[0] = firstGeometry;
  }

  this->Modified();
}


void
mitk::SlicedGeometry3D
::SetSliceNavigationController( SliceNavigationController *snc )
{
  m_SliceNavigationController = snc;
}


mitk::SliceNavigationController *
mitk::SlicedGeometry3D::GetSliceNavigationController()
{
  return m_SliceNavigationController;
}

void
mitk::SlicedGeometry3D::SetEvenlySpaced(bool on)
{
  if(m_EvenlySpaced!=on)
  {
    m_EvenlySpaced=on;
    this->Modified();
  }
}


void
mitk::SlicedGeometry3D
::SetDirectionVector( const mitk::Vector3D& directionVector ) 
{
  VnlVector diff = m_DirectionVector.Get_vnl_vector()
    - directionVector.Get_vnl_vector();

  if ( diff.squared_magnitude()>=vnl_math::float_sqrteps )
  {
    m_DirectionVector = directionVector;
    m_DirectionVector.Normalize();
    this->Modified();
  }
}


void
mitk::SlicedGeometry3D::SetTimeBounds( const mitk::TimeBounds& timebounds )
{
  Superclass::SetTimeBounds( timebounds );

  unsigned int s;
  for ( s = 0; s < m_Slices; ++s )
  {
    if(m_Geometry2Ds[s].IsNotNull())
    {
      m_Geometry2Ds[s]->SetTimeBounds( timebounds );
    }
  }
  m_TimeBounds = timebounds;
}


mitk::AffineGeometryFrame3D::Pointer
mitk::SlicedGeometry3D::Clone() const
{
  Self::Pointer newGeometry = Self::New();
  newGeometry->Initialize(m_Slices);
  InitializeGeometry(newGeometry);
  return newGeometry.GetPointer();
}


void
mitk::SlicedGeometry3D::InitializeGeometry( Self *newGeometry ) const
{
  Superclass::InitializeGeometry( newGeometry );

  newGeometry->SetEvenlySpaced( m_EvenlySpaced );
  newGeometry->SetSpacing( this->GetSpacing() );
  newGeometry->SetDirectionVector( this->GetDirectionVector() );

  newGeometry->SetSliceNavigationController( m_SliceNavigationController );
  newGeometry->m_ReferenceGeometry = m_ReferenceGeometry;

  if ( m_EvenlySpaced )
  {
    AffineGeometryFrame3D::Pointer geometry = m_Geometry2Ds[0]->Clone();
    Geometry2D* geometry2D = dynamic_cast<Geometry2D*>(geometry.GetPointer());
    assert(geometry2D!=NULL);
    newGeometry->SetGeometry2D(geometry2D, 0);
  }
  else
  {
    unsigned int s;
    for ( s = 0; s < m_Slices; ++s )
    {
      if ( m_Geometry2Ds[s].IsNull() )
      {
        assert(m_EvenlySpaced);
      }
      else
      {
        AffineGeometryFrame3D::Pointer geometry = m_Geometry2Ds[s]->Clone();
        Geometry2D* geometry2D = dynamic_cast<Geometry2D*>(geometry.GetPointer());
        assert(geometry2D!=NULL);
        newGeometry->SetGeometry2D(geometry2D, s);
      }
    }
  }
}


void
mitk::SlicedGeometry3D::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
  Superclass::PrintSelf(os,indent);
  os << indent << " EvenlySpaced: " << m_EvenlySpaced << std::endl;
  if ( m_EvenlySpaced )
  {
    os << indent << " DirectionVector: " << m_DirectionVector << std::endl;
  }
  os << indent << " Slices: " << m_Slices << std::endl;

  os << indent << " GetGeometry2D(0): ";
  if ( this->GetGeometry2D(0) == NULL )
  {
    os << "NULL" << std::endl;
  }
  else
  {
    this->GetGeometry2D(0)->Print(os, indent);
  }
}

void
mitk::SlicedGeometry3D::ExecuteOperation(Operation* operation)
{
  switch ( operation->GetOperationType() )
  {
  case OpNOTHING:
    break;
  
  case OpROTATE:
    if ( m_EvenlySpaced )
    {
      // Need a reference frame to align the rotation
      if ( m_ReferenceGeometry )
      {
        // Clear all generated geometries and then rotate only the first slice.
        // The other slices will be re-generated on demand
        
        // Save first slice
        Geometry2D::Pointer geometry2D = m_Geometry2Ds[0];

        RotationOperation *rotOp = dynamic_cast< RotationOperation * >( operation );

        // Generate a RotationOperation using the dataset center instead of
        // the supplied rotation center. This is necessary so that the rotated 
        // zero-plane does not shift away. The supplied center is instead used
        // to adjust the slice stack afterwards.
        Point3D center = m_ReferenceGeometry->GetCenter();

        RotationOperation centeredRotation(
          rotOp->GetOperationType(),
          center,
          rotOp->GetVectorOfRotation(),
          rotOp->GetAngleOfRotation()
        );

        // Rotate first slice
        geometry2D->ExecuteOperation( &centeredRotation );

        // Clear the slice stack and adjust it according to the center of
        // the dataset and the supplied rotation center (see documentation of 
        // ReinitializePlanes)
        this->ReinitializePlanes( center, rotOp->GetCenterOfRotation() );

        if ( m_SliceNavigationController )
        {
          m_SliceNavigationController->SelectSliceByPoint(
            rotOp->GetCenterOfRotation() );
          m_SliceNavigationController->AdjustSliceStepperRange();
        }

        Geometry3D::ExecuteOperation( &centeredRotation );
      }
    }
    else
    {
      // Reach through to all slices
      for (std::vector<Geometry2D::Pointer>::iterator iter = m_Geometry2Ds.begin();
          iter != m_Geometry2Ds.end();
          ++iter)
      {
        (*iter)->ExecuteOperation(operation);
      }
    }
    break;

  case OpORIENT:
    if ( m_EvenlySpaced )
    {
      // Save first slice
      Geometry2D::Pointer geometry2D = m_Geometry2Ds[0];

      PlaneGeometry *planeGeometry = dynamic_cast< PlaneGeometry * >( 
        geometry2D.GetPointer() );

      PlaneOperation *planeOp = dynamic_cast< PlaneOperation * >( operation );

      // Need a PlaneGeometry, a PlaneOperation and a reference frame to
      // carry out the re-orientation
      if ( m_ReferenceGeometry && planeGeometry && planeOp )
      {
        // Clear all generated geometries and then rotate only the first slice.
        // The other slices will be re-generated on demand

        // Generate a RotationOperation by calculating the angle between
        // the current and the requested slice orientation
        Point3D center = m_ReferenceGeometry->GetCenter();

        const mitk::Vector3D &currentNormal = planeGeometry->GetNormal();
        const mitk::Vector3D &newNormal = planeOp->GetNormal();

        Vector3D rotationAxis = itk::CrossProduct( newNormal, currentNormal );

        vtkFloatingPointType rotationAngle = - atan2( 
          (double) rotationAxis.GetNorm(),
          (double) (newNormal * currentNormal) );

        rotationAngle *= 180.0 / vnl_math::pi;

        RotationOperation centeredRotation(
          mitk::OpROTATE,
          center,
          rotationAxis,
          rotationAngle
        );

        // Rotate first slice
        geometry2D->ExecuteOperation( &centeredRotation );

        // Clear the slice stack and adjust it according to the center of
        // rotation and plane position (see documentation of ReinitializePlanes)
        this->ReinitializePlanes( center, planeOp->GetPoint() );

        if ( m_SliceNavigationController )
        {
          m_SliceNavigationController->SelectSliceByPoint( planeOp->GetPoint() );
          m_SliceNavigationController->AdjustSliceStepperRange();
        }

        Geometry3D::ExecuteOperation( &centeredRotation );
      }
    }
    else
    {
      // Reach through to all slices
      for (std::vector<Geometry2D::Pointer>::iterator iter = m_Geometry2Ds.begin();
          iter != m_Geometry2Ds.end();
          ++iter)
      {
        (*iter)->ExecuteOperation(operation);
      }
    }
    break;
  }

  this->Modified();
}

