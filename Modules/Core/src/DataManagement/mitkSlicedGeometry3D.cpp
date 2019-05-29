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

#include <itkSpatialOrientationAdapter.h>

#include "mitkSlicedGeometry3D.h"
#include "mitkAbstractTransformGeometry.h"
#include "mitkApplyTransformMatrixOperation.h"
#include "mitkInteractionConst.h"
#include "mitkPlaneGeometry.h"
#include "mitkPlaneOperation.h"
#include "mitkRestorePlanePositionOperation.h"
#include "mitkRotationOperation.h"
#include "mitkSliceNavigationController.h"

const mitk::ScalarType PI = 3.14159265359;

mitk::SlicedGeometry3D::SlicedGeometry3D()
  : m_EvenlySpaced(true), m_Slices(0), m_ReferenceGeometry(nullptr), m_SliceNavigationController(nullptr)
{
  m_DirectionVector.Fill(0);
  this->InitializeSlicedGeometry(m_Slices);
}

mitk::SlicedGeometry3D::SlicedGeometry3D(const SlicedGeometry3D &other)
  : Superclass(other),
    m_EvenlySpaced(other.m_EvenlySpaced),
    m_Slices(other.m_Slices),
    m_ReferenceGeometry(other.m_ReferenceGeometry),
    m_SliceNavigationController(other.m_SliceNavigationController)
{
  m_DirectionVector.Fill(0);
  SetSpacing(other.GetSpacing());
  SetDirectionVector(other.GetDirectionVector());

  if (m_EvenlySpaced)
  {
    assert(!other.m_PlaneGeometries.empty() && "This may happen when you use one of the old Initialize methods, which had a bool parameter that is implicitly casted to the number of slices now.");
    PlaneGeometry::Pointer geometry = other.m_PlaneGeometries[0]->Clone();
    assert(geometry.IsNotNull());
    SetPlaneGeometry(geometry, 0);
  }
  else
  {
    unsigned int s;
    for (s = 0; s < other.m_Slices; ++s)
    {
      if (other.m_PlaneGeometries[s].IsNull())
      {
        assert(other.m_EvenlySpaced);
        m_PlaneGeometries[s] = nullptr;
      }
      else
      {
        PlaneGeometry *geometry2D = other.m_PlaneGeometries[s]->Clone();
        assert(geometry2D != nullptr);
        SetPlaneGeometry(geometry2D, s);
      }
    }
  }
}

mitk::SlicedGeometry3D::~SlicedGeometry3D()
{
}

mitk::PlaneGeometry *mitk::SlicedGeometry3D::GetPlaneGeometry(int s) const
{
  mitk::PlaneGeometry::Pointer geometry2D = nullptr;

  if (this->IsValidSlice(s))
  {
    geometry2D = m_PlaneGeometries[s];

    // If (a) m_EvenlySpaced==true, (b) we don't have a PlaneGeometry stored
    // for the requested slice, and (c) the first slice (s=0)
    // is a PlaneGeometry instance, then we calculate the geometry of the
    // requested as the plane of the first slice shifted by m_Spacing[2]*s
    // in the direction of m_DirectionVector.
    if ((m_EvenlySpaced) && (geometry2D.IsNull()))
    {
      PlaneGeometry *firstSlice = m_PlaneGeometries[0];

      if (firstSlice != nullptr &&
          dynamic_cast<AbstractTransformGeometry *>(m_PlaneGeometries[0].GetPointer()) == nullptr)
      {
        if ((m_DirectionVector[0] == 0.0) && (m_DirectionVector[1] == 0.0) && (m_DirectionVector[2] == 0.0))
        {
          m_DirectionVector = firstSlice->GetNormal();
          m_DirectionVector.Normalize();
        }

        Vector3D direction;
        direction = m_DirectionVector * this->GetSpacing()[2];

        mitk::PlaneGeometry::Pointer requestedslice;
        requestedslice = static_cast<mitk::PlaneGeometry *>(firstSlice->Clone().GetPointer());

        requestedslice->SetOrigin(requestedslice->GetOrigin() + direction * s);

        geometry2D = requestedslice;
        m_PlaneGeometries[s] = geometry2D;
      }
    }
    return geometry2D;
  }
  else
  {
    return nullptr;
  }
}

const mitk::BoundingBox *mitk::SlicedGeometry3D::GetBoundingBox() const
{
  assert(this->IsBoundingBoxNull() == false);
  return Superclass::GetBoundingBox();
}

bool mitk::SlicedGeometry3D::SetPlaneGeometry(mitk::PlaneGeometry *geometry2D, int s)
{
  if (this->IsValidSlice(s))
  {
    m_PlaneGeometries[s] = geometry2D;
    m_PlaneGeometries[s]->SetReferenceGeometry(m_ReferenceGeometry);
    return true;
  }
  return false;
}

void mitk::SlicedGeometry3D::InitializeSlicedGeometry(unsigned int slices)
{
  Superclass::Initialize();
  m_Slices = slices;

  PlaneGeometry::Pointer gnull = nullptr;
  m_PlaneGeometries.assign(m_Slices, gnull);

  Vector3D spacing;
  spacing.Fill(1.0);
  this->SetSpacing(spacing);

  m_DirectionVector.Fill(0);
}

void mitk::SlicedGeometry3D::InitializeEvenlySpaced(mitk::PlaneGeometry *geometry2D, unsigned int slices)
{
  assert(geometry2D != nullptr);
  this->InitializeEvenlySpaced(geometry2D, geometry2D->GetExtentInMM(2) / geometry2D->GetExtent(2), slices);
}

void mitk::SlicedGeometry3D::InitializeEvenlySpaced(mitk::PlaneGeometry *geometry2D,
                                                    mitk::ScalarType zSpacing,
                                                    unsigned int slices)
{
  assert(geometry2D != nullptr);
  assert(geometry2D->GetExtent(0) > 0);
  assert(geometry2D->GetExtent(1) > 0);

  geometry2D->Register();

  Superclass::Initialize();
  m_Slices = slices;

  BoundingBox::BoundsArrayType bounds = geometry2D->GetBounds();
  bounds[4] = 0;
  bounds[5] = slices;

  // clear and reserve
  PlaneGeometry::Pointer gnull = nullptr;
  m_PlaneGeometries.assign(m_Slices, gnull);

  Vector3D directionVector = geometry2D->GetAxisVector(2);
  directionVector.Normalize();
  directionVector *= zSpacing;

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

  this->SetIndexToWorldTransform(geometry2D->GetIndexToWorldTransform());

  mitk::Vector3D spacing;
  FillVector3D(spacing, geometry2D->GetExtentInMM(0) / bounds[1], geometry2D->GetExtentInMM(1) / bounds[3], zSpacing);

  this->SetDirectionVector(directionVector);
  this->SetBounds(bounds);
  this->SetPlaneGeometry(geometry2D, 0);
  this->SetSpacing(spacing, true);
  this->SetEvenlySpaced();

  // this->SetTimeBounds( geometry2D->GetTimeBounds() );

  assert(this->GetIndexToWorldTransform() != geometry2D->GetIndexToWorldTransform()); // (**) see above.

  this->SetFrameOfReferenceID(geometry2D->GetFrameOfReferenceID());
  this->SetImageGeometry(geometry2D->GetImageGeometry());

  geometry2D->UnRegister();
}

void mitk::SlicedGeometry3D::InitializePlanes(const mitk::BaseGeometry *geometry3D,
                                              mitk::PlaneGeometry::PlaneOrientation planeorientation,
                                              bool top,
                                              bool frontside,
                                              bool rotated)
{
  m_ReferenceGeometry = geometry3D;

  PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane(geometry3D, top, planeorientation, frontside, rotated);

  int worldAxis =
      planeorientation == PlaneGeometry::Sagittal ? 0 :
      planeorientation == PlaneGeometry::Frontal  ? 1 : 2;

  // Inspired by:
  // http://www.na-mic.org/Wiki/index.php/Coordinate_System_Conversion_Between_ITK_and_Slicer3

  mitk::AffineTransform3D::MatrixType matrix = geometry3D->GetIndexToWorldTransform()->GetMatrix();
  matrix.GetVnlMatrix().normalize_columns();
  mitk::AffineTransform3D::MatrixType::InternalMatrixType inverseMatrix = matrix.GetTranspose();

  int dominantAxis = planeGeometry->CalculateDominantAxes(inverseMatrix).at(worldAxis);
  ScalarType viewSpacing = geometry3D->GetSpacing()[dominantAxis];

  /// Although the double value returned by GetExtent() holds a round number,
  /// you need to add 0.5 to safely convert it to unsigned it. I have seen a
  /// case when the result was less by one without this.
  auto slices = static_cast<unsigned int>(geometry3D->GetExtent(dominantAxis) + 0.5);
  if ( slices == 0 && geometry3D->GetExtent(dominantAxis) > 0) {
      // require at least one slice if there is _some_ extent
      slices = 1;
  }

#ifndef NDEBUG
  int upDirection = itk::Function::Sign(inverseMatrix[dominantAxis][worldAxis]);

  /// The normal vector of an imaginary plane that points from the world origin (bottom left back
  /// corner or the world, with the lowest physical coordinates) towards the inside of the volume,
  /// along the renderer axis. Length is the slice thickness.
  Vector3D worldPlaneNormal = inverseMatrix.get_row(dominantAxis) * (upDirection * viewSpacing);

  /// The normal of the standard plane geometry just created.
  Vector3D standardPlaneNormal = planeGeometry->GetNormal();

  /// The standard plane must be parallel to the 'world plane'. The normal of the standard plane
  /// must point against the world plane if and only if 'top' is 'false'. The length of the
  /// standard plane normal must be equal to the slice thickness.
  assert((standardPlaneNormal - (top ? 1.0 : -1.0) * worldPlaneNormal).GetSquaredNorm() < 0.000001);
#endif

  this->InitializeEvenlySpaced(planeGeometry, viewSpacing, slices);

#ifndef NDEBUG
  /// The standard plane normal and the z axis vector of the sliced geometry must point in
  /// the same direction.
  Vector3D zAxisVector = this->GetAxisVector(2);
  Vector3D upscaledStandardPlaneNormal = standardPlaneNormal;
  upscaledStandardPlaneNormal *= slices;
  assert((zAxisVector - upscaledStandardPlaneNormal).GetSquaredNorm() < 0.000001);

  /// You can use this test is to check the handedness of the coordinate system of the current
  /// geometry. In principle, you can use either left- or right-handed coordinate systems, but
  /// you normally want it to be consistent, that is the handedness should be the same across
  /// the renderers of the same viewer.
//  ScalarType det = vnl_det(this->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix());
//  MITK_DEBUG << "world axis: " << worldAxis << (det > 0 ? " ; right-handed" : " ; left-handed");
#endif
}

void mitk::SlicedGeometry3D::ReinitializePlanes(const Point3D &center, const Point3D &referencePoint)
{
  // Need a reference frame to align the rotated planes
  if (!m_ReferenceGeometry)
  {
    return;
  }

  // Get first plane of plane stack
  PlaneGeometry *firstPlane = m_PlaneGeometries[0];

  // If plane stack is empty, exit
  if (!firstPlane || dynamic_cast<AbstractTransformGeometry *>(firstPlane))
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
  spacing[0] = this->CalculateSpacing(axis0);
  spacing[1] = this->CalculateSpacing(axis1);
  spacing[2] = this->CalculateSpacing(normal);

  Superclass::SetSpacing(spacing);

  // Now we need to calculate the number of slices in the plane's normal
  // direction, so that the entire volume is covered. This is done by first
  // calculating the dot product between the volume diagonal (the maximum
  // distance inside the volume) and the normal, and dividing this value by
  // the directed spacing calculated above.
  ScalarType directedExtent = std::abs(m_ReferenceGeometry->GetExtentInMM(0) * normal[0]) +
                              std::abs(m_ReferenceGeometry->GetExtentInMM(1) * normal[1]) +
                              std::abs(m_ReferenceGeometry->GetExtentInMM(2) * normal[2]);

  if (directedExtent >= spacing[2])
  {
    m_Slices = static_cast<unsigned int>(directedExtent / spacing[2] + 0.5);
  }
  else
  {
    m_Slices = 1;
  }

  // The origin of our "first plane" needs to be adapted to this new extent.
  // To achieve this, we first calculate the current distance to the volume's
  // center, and then shift the origin in the direction of the normal by the
  // difference between this distance and half of the new extent.
  double centerOfRotationDistance = firstPlane->SignedDistanceFromPlane(center);

  if (centerOfRotationDistance > 0)
  {
    firstPlane->SetOrigin(firstPlane->GetOrigin() + normal * (centerOfRotationDistance - directedExtent / 2.0));
    m_DirectionVector = normal;
  }
  else
  {
    firstPlane->SetOrigin(firstPlane->GetOrigin() + normal * (directedExtent / 2.0 + centerOfRotationDistance));
    m_DirectionVector = -normal;
  }

  // Now we adjust this distance according with respect to the given reference
  // point: we need to make sure that the point is touched by one slice of the
  // new slice stack.
  double referencePointDistance = firstPlane->SignedDistanceFromPlane(referencePoint);

  auto referencePointSlice = static_cast<int>(referencePointDistance / spacing[2]);

  double alignmentValue = referencePointDistance / spacing[2] - referencePointSlice;

  firstPlane->SetOrigin(firstPlane->GetOrigin() + normal * alignmentValue * spacing[2]);

  // Finally, we can clear the previous geometry stack and initialize it with
  // our re-initialized "first plane".
  m_PlaneGeometries.assign(m_Slices, PlaneGeometry::Pointer(nullptr));

  if (m_Slices > 0)
  {
    m_PlaneGeometries[0] = firstPlane;
  }

  // Reinitialize SNC with new number of slices
  m_SliceNavigationController->GetSlice()->SetSteps(m_Slices);

  this->Modified();
}

double mitk::SlicedGeometry3D::CalculateSpacing(const mitk::Vector3D &d) const
{
  // Need the spacing of the underlying dataset / geometry
  if (!m_ReferenceGeometry)
  {
    return 1.0;
  }

  const mitk::Vector3D &spacing = m_ReferenceGeometry->GetSpacing();
  return SlicedGeometry3D::CalculateSpacing(spacing, d);
}

double mitk::SlicedGeometry3D::CalculateSpacing(const mitk::Vector3D &spacing, const mitk::Vector3D &d)
{
  // The following can be derived from the ellipsoid equation
  //
  //   1 = x^2/a^2 + y^2/b^2 + z^2/c^2
  //
  // where (a,b,c) = spacing of original volume (ellipsoid radii)
  // and   (x,y,z) = scaled coordinates of vector d (according to ellipsoid)
  //
  double scaling = d[0] * d[0] / (spacing[0] * spacing[0]) + d[1] * d[1] / (spacing[1] * spacing[1]) +
                   d[2] * d[2] / (spacing[2] * spacing[2]);

  scaling = sqrt(scaling);

  return (sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]) / scaling);
}

mitk::Vector3D mitk::SlicedGeometry3D::AdjustNormal(const mitk::Vector3D &normal) const
{
  TransformType::Pointer inverse = TransformType::New();
  m_ReferenceGeometry->GetIndexToWorldTransform()->GetInverse(inverse);

  Vector3D transformedNormal = inverse->TransformVector(normal);

  transformedNormal.Normalize();
  return transformedNormal;
}

void mitk::SlicedGeometry3D::SetImageGeometry(const bool isAnImageGeometry)
{
  Superclass::SetImageGeometry(isAnImageGeometry);

  unsigned int s;
  for (s = 0; s < m_Slices; ++s)
  {
    mitk::BaseGeometry *geometry = m_PlaneGeometries[s];
    if (geometry != nullptr)
    {
      geometry->SetImageGeometry(isAnImageGeometry);
    }
  }
}

void mitk::SlicedGeometry3D::ChangeImageGeometryConsideringOriginOffset(const bool isAnImageGeometry)
{
  unsigned int s;
  for (s = 0; s < m_Slices; ++s)
  {
    mitk::BaseGeometry *geometry = m_PlaneGeometries[s];
    if (geometry != nullptr)
    {
      geometry->ChangeImageGeometryConsideringOriginOffset(isAnImageGeometry);
    }
  }

  Superclass::ChangeImageGeometryConsideringOriginOffset(isAnImageGeometry);
}

bool mitk::SlicedGeometry3D::IsValidSlice(int s) const
{
  return ((s >= 0) && (s < (int)m_Slices));
}

const mitk::BaseGeometry *mitk::SlicedGeometry3D::GetReferenceGeometry() const
{
  return m_ReferenceGeometry;
}

void mitk::SlicedGeometry3D::SetReferenceGeometry(const BaseGeometry *referenceGeometry)
{
  m_ReferenceGeometry = referenceGeometry;

  std::vector<PlaneGeometry::Pointer>::iterator it;

  for (it = m_PlaneGeometries.begin(); it != m_PlaneGeometries.end(); ++it)
  {
    (*it)->SetReferenceGeometry(referenceGeometry);
  }
}

bool mitk::SlicedGeometry3D::HasReferenceGeometry() const
{
  return ( m_ReferenceGeometry != nullptr );
}

void mitk::SlicedGeometry3D::PreSetSpacing(const mitk::Vector3D &aSpacing)
{
  bool hasEvenlySpacedPlaneGeometry = false;
  mitk::Point3D origin;
  mitk::Vector3D rightDV, bottomDV;
  BoundingBox::BoundsArrayType bounds;

  // Check for valid spacing
  if (!(aSpacing[0] > 0 && aSpacing[1] > 0 && aSpacing[2] > 0))
  {
    mitkThrow() << "You try to set a spacing with at least one element equal or "
                   "smaller to \"0\". This might lead to a crash during rendering. Please double"
                   " check your data!";
  }

  // In case of evenly-spaced data: re-initialize instances of PlaneGeometry,
  // since the spacing influences them
  if ((m_EvenlySpaced) && (m_PlaneGeometries.size() > 0))
  {
    const PlaneGeometry *planeGeometry = m_PlaneGeometries[0];

    if (planeGeometry && !dynamic_cast<const AbstractTransformGeometry *>(planeGeometry))
    {
      this->WorldToIndex(planeGeometry->GetOrigin(), origin);
      this->WorldToIndex(planeGeometry->GetAxisVector(0), rightDV);
      this->WorldToIndex(planeGeometry->GetAxisVector(1), bottomDV);

      bounds = planeGeometry->GetBounds();
      hasEvenlySpacedPlaneGeometry = true;
    }
  }

  BaseGeometry::_SetSpacing(aSpacing);

  mitk::PlaneGeometry::Pointer firstGeometry;

  // In case of evenly-spaced data: re-initialize instances of PlaneGeometry,
  // since the spacing influences them
  if (hasEvenlySpacedPlaneGeometry)
  {
    // create planeGeometry according to new spacing
    this->IndexToWorld(origin, origin);
    this->IndexToWorld(rightDV, rightDV);
    this->IndexToWorld(bottomDV, bottomDV);

    mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
    planeGeometry->SetImageGeometry(this->GetImageGeometry());

    planeGeometry->SetReferenceGeometry(m_ReferenceGeometry);

    // Store spacing, as Initialize... needs a pointer
    mitk::Vector3D lokalSpacing = this->GetSpacing();
    planeGeometry->InitializeStandardPlane(rightDV.GetVnlVector(), bottomDV.GetVnlVector(), &lokalSpacing);
    planeGeometry->SetOrigin(origin);
    planeGeometry->SetBounds(bounds);

    firstGeometry = planeGeometry;
  }
  else if ((m_EvenlySpaced) && (m_PlaneGeometries.size() > 0))
  {
    firstGeometry = m_PlaneGeometries[0].GetPointer();
  }

  // clear and reserve
  PlaneGeometry::Pointer gnull = nullptr;
  m_PlaneGeometries.assign(m_Slices, gnull);

  if (m_Slices > 0)
  {
    m_PlaneGeometries[0] = firstGeometry;
  }

  this->Modified();
}

void mitk::SlicedGeometry3D::SetSliceNavigationController(SliceNavigationController *snc)
{
  m_SliceNavigationController = snc;
}

mitk::SliceNavigationController *mitk::SlicedGeometry3D::GetSliceNavigationController()
{
  return m_SliceNavigationController;
}

void mitk::SlicedGeometry3D::SetEvenlySpaced(bool on)
{
  if (m_EvenlySpaced != on)
  {
    m_EvenlySpaced = on;
    this->Modified();
  }
}

void mitk::SlicedGeometry3D::SetDirectionVector(const mitk::Vector3D &directionVector)
{
  Vector3D newDir = directionVector;
  newDir.Normalize();
  if (newDir != m_DirectionVector)
  {
    m_DirectionVector = newDir;
    this->Modified();
  }
}

// void
// mitk::SlicedGeometry3D::SetTimeBounds( const mitk::TimeBounds& timebounds )
//{
//  Superclass::SetTimeBounds( timebounds );
//
//  unsigned int s;
//  for ( s = 0; s < m_Slices; ++s )
//  {
//    if(m_Geometry2Ds[s].IsNotNull())
//    {
//      m_Geometry2Ds[s]->SetTimeBounds( timebounds );
//    }
//  }
//  m_TimeBounds = timebounds;
//}

itk::LightObject::Pointer mitk::SlicedGeometry3D::InternalClone() const
{
  Self::Pointer newGeometry = new SlicedGeometry3D(*this);
  newGeometry->UnRegister();
  return newGeometry.GetPointer();
}

void mitk::SlicedGeometry3D::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << " EvenlySpaced: " << m_EvenlySpaced << std::endl;
  if (m_EvenlySpaced)
  {
    os << indent << " DirectionVector: " << m_DirectionVector << std::endl;
  }
  os << indent << " Slices: " << m_Slices << std::endl;

  os << std::endl;
  os << indent << " GetPlaneGeometry(0): ";
  if (this->GetPlaneGeometry(0) == nullptr)
  {
    os << "nullptr" << std::endl;
  }
  else
  {
    this->GetPlaneGeometry(0)->Print(os, indent);
  }
}

void mitk::SlicedGeometry3D::ExecuteOperation(Operation *operation)
{
  PlaneGeometry::Pointer geometry2D;
  ApplyTransformMatrixOperation *applyMatrixOp;
  Point3D center;

  switch (operation->GetOperationType())
  {
    case OpNOTHING:
      break;

    case OpROTATE:
      if (m_EvenlySpaced)
      {
        // Need a reference frame to align the rotation
        if (m_ReferenceGeometry)
        {
          // Clear all generated geometries and then rotate only the first slice.
          // The other slices will be re-generated on demand

          // Save first slice
          PlaneGeometry::Pointer geometry2D = m_PlaneGeometries[0];

          auto *rotOp = dynamic_cast<RotationOperation *>(operation);

          // Generate a RotationOperation using the dataset center instead of
          // the supplied rotation center. This is necessary so that the rotated
          // zero-plane does not shift away. The supplied center is instead used
          // to adjust the slice stack afterwards.
          Point3D center = m_ReferenceGeometry->GetCenter();

          RotationOperation centeredRotation(
            rotOp->GetOperationType(), center, rotOp->GetVectorOfRotation(), rotOp->GetAngleOfRotation());

          // Rotate first slice
          geometry2D->ExecuteOperation(&centeredRotation);

          // Clear the slice stack and adjust it according to the center of
          // the dataset and the supplied rotation center (see documentation of
          // ReinitializePlanes)
          this->ReinitializePlanes(center, rotOp->GetCenterOfRotation());

          geometry2D->SetSpacing(this->GetSpacing());

          if (m_SliceNavigationController)
          {
            m_SliceNavigationController->SelectSliceByPoint(rotOp->GetCenterOfRotation());
            m_SliceNavigationController->AdjustSliceStepperRange();
          }

          BaseGeometry::ExecuteOperation(&centeredRotation);
        }
        else
        {
          // we also have to consider the case, that there is no reference geometry available.
          if (m_PlaneGeometries.size() > 0)
          {
            // Reach through to all slices in my container
            for (auto iter = m_PlaneGeometries.begin(); iter != m_PlaneGeometries.end(); ++iter)
            {
              // Test for empty slices, which can happen if evenly spaced geometry
              if ((*iter).IsNotNull())
              {
                (*iter)->ExecuteOperation(operation);
              }
            }

            // rotate overall geometry
            auto *rotOp = dynamic_cast<RotationOperation *>(operation);
            BaseGeometry::ExecuteOperation(rotOp);
          }
        }
      }
      else
      {
        // Reach through to all slices
        for (auto iter = m_PlaneGeometries.begin(); iter != m_PlaneGeometries.end(); ++iter)
        {
          (*iter)->ExecuteOperation(operation);
        }
      }
      break;

    case OpORIENT:
      if (m_EvenlySpaced)
      {
        // get operation data
        auto *planeOp = dynamic_cast<PlaneOperation *>(operation);

        // Get first slice
        PlaneGeometry::Pointer planeGeometry = m_PlaneGeometries[0];

        // Need a PlaneGeometry, a PlaneOperation and a reference frame to
        // carry out the re-orientation. If not all avaialble, stop here
        if (!m_ReferenceGeometry ||
            (!planeGeometry || dynamic_cast<AbstractTransformGeometry *>(planeGeometry.GetPointer())) || !planeOp)
        {
          break;
        }

        // General Behavior:
        // Clear all generated geometries and then rotate only the first slice.
        // The other slices will be re-generated on demand

        //
        // 1st Step: Reorient Normal Vector of first plane
        //
        Point3D center = planeOp->GetPoint(); // m_ReferenceGeometry->GetCenter();
        mitk::Vector3D currentNormal = planeGeometry->GetNormal();
        mitk::Vector3D newNormal;
        if (planeOp->AreAxisDefined())
        {
          // If planeOp was defined by one centerpoint and two axis vectors
          newNormal = CrossProduct(planeOp->GetAxisVec0(), planeOp->GetAxisVec1());
        }
        else
        {
          // If planeOp was defined by one centerpoint and one normal vector
          newNormal = planeOp->GetNormal();
        }

        // Get Rotation axis und angle
        currentNormal.Normalize();
        newNormal.Normalize();
        ScalarType rotationAngle = angle(currentNormal.GetVnlVector(), newNormal.GetVnlVector());

        rotationAngle *= 180.0 / vnl_math::pi; // from rad to deg
        Vector3D rotationAxis = itk::CrossProduct(currentNormal, newNormal);
        if (std::abs(rotationAngle - 180) < mitk::eps)
        {
          // current Normal and desired normal are not linear independent!!(e.g 1,0,0 and -1,0,0).
          // Rotation Axis should be ANY vector that is 90� to current Normal
          mitk::Vector3D helpNormal;
          helpNormal = currentNormal;
          helpNormal[0] += 1;
          helpNormal[1] -= 1;
          helpNormal[2] += 1;
          helpNormal.Normalize();
          rotationAxis = itk::CrossProduct(helpNormal, currentNormal);
        }

        RotationOperation centeredRotation(mitk::OpROTATE, center, rotationAxis, rotationAngle);

        // Rotate first slice
        planeGeometry->ExecuteOperation(&centeredRotation);

        // Reinitialize planes and select slice, if my rotations are all done.
        if (!planeOp->AreAxisDefined())
        {
          // Clear the slice stack and adjust it according to the center of
          // rotation and plane position (see documentation of ReinitializePlanes)
          this->ReinitializePlanes(center, planeOp->GetPoint());
          planeGeometry->SetSpacing(this->GetSpacing());

          if (m_SliceNavigationController)
          {
            m_SliceNavigationController->SelectSliceByPoint(planeOp->GetPoint());
            m_SliceNavigationController->AdjustSliceStepperRange();
          }
        }

        // Also apply rotation on the slicedGeometry - Geometry3D (Bounding geometry)
        BaseGeometry::ExecuteOperation(&centeredRotation);

        //
        // 2nd step. If axis vectors were defined, rotate the plane around its normal to fit these
        //

        if (planeOp->AreAxisDefined())
        {
          mitk::Vector3D vecAxixNew = planeOp->GetAxisVec0();
          vecAxixNew.Normalize();
          mitk::Vector3D VecAxisCurr = planeGeometry->GetAxisVector(0);
          VecAxisCurr.Normalize();

          ScalarType rotationAngle = angle(VecAxisCurr.GetVnlVector(), vecAxixNew.GetVnlVector());
          rotationAngle = rotationAngle * 180 / PI; // Rad to Deg

          // we rotate around the normal of the plane, but we do not know, if we need to rotate clockwise
          // or anti-clockwise. So we rotate around the crossproduct of old and new Axisvector.
          // Since both axis vectors lie in the plane, the crossproduct is the planes normal or the negative planes
          // normal

          rotationAxis = itk::CrossProduct(VecAxisCurr, vecAxixNew);
          if (std::abs(rotationAngle - 180) < mitk::eps)
          {
            // current axisVec and desired axisVec are not linear independent!!(e.g 1,0,0 and -1,0,0).
            // Rotation Axis can be just plane Normal. (have to rotate by 180�)
            rotationAxis = newNormal;
          }

          // Perfom Rotation
          mitk::RotationOperation op(mitk::OpROTATE, center, rotationAxis, rotationAngle);
          planeGeometry->ExecuteOperation(&op);

          // Apply changes on first slice to whole slice stack
          this->ReinitializePlanes(center, planeOp->GetPoint());
          planeGeometry->SetSpacing(this->GetSpacing());

          if (m_SliceNavigationController)
          {
            m_SliceNavigationController->SelectSliceByPoint(planeOp->GetPoint());
            m_SliceNavigationController->AdjustSliceStepperRange();
          }

          // Also apply rotation on the slicedGeometry - Geometry3D (Bounding geometry)
          BaseGeometry::ExecuteOperation(&op);
        }
      }
      else
      {
        // Reach through to all slices
        for (auto iter = m_PlaneGeometries.begin(); iter != m_PlaneGeometries.end(); ++iter)
        {
          (*iter)->ExecuteOperation(operation);
        }
      }
      break;

    case OpRESTOREPLANEPOSITION:
      if (m_EvenlySpaced)
      {
        // Save first slice
        PlaneGeometry::Pointer planeGeometry = m_PlaneGeometries[0];

        auto *restorePlaneOp = dynamic_cast<RestorePlanePositionOperation *>(operation);

        // Need a PlaneGeometry, a PlaneOperation and a reference frame to
        // carry out the re-orientation
        if (m_ReferenceGeometry &&
            (planeGeometry && dynamic_cast<AbstractTransformGeometry *>(planeGeometry.GetPointer()) == nullptr) &&
            restorePlaneOp)
        {
          // Clear all generated geometries and then rotate only the first slice.
          // The other slices will be re-generated on demand

          // Rotate first slice
          planeGeometry->ExecuteOperation(restorePlaneOp);

          m_DirectionVector = restorePlaneOp->GetDirectionVector();

          double centerOfRotationDistance = planeGeometry->SignedDistanceFromPlane(m_ReferenceGeometry->GetCenter());

          if (centerOfRotationDistance <= 0)
          {
            m_DirectionVector = -m_DirectionVector;
          }

          Vector3D spacing = restorePlaneOp->GetSpacing();

          Superclass::SetSpacing(spacing);

          // /*Now we need to calculate the number of slices in the plane's normal
          // direction, so that the entire volume is covered. This is done by first
          // calculating the dot product between the volume diagonal (the maximum
          // distance inside the volume) and the normal, and dividing this value by
          // the directed spacing calculated above.*/
          ScalarType directedExtent = std::abs(m_ReferenceGeometry->GetExtentInMM(0) * m_DirectionVector[0]) +
                                      std::abs(m_ReferenceGeometry->GetExtentInMM(1) * m_DirectionVector[1]) +
                                      std::abs(m_ReferenceGeometry->GetExtentInMM(2) * m_DirectionVector[2]);

          if (directedExtent >= spacing[2])
          {
            m_Slices = static_cast<unsigned int>(directedExtent / spacing[2] + 0.5);
          }
          else
          {
            m_Slices = 1;
          }

          m_PlaneGeometries.assign(m_Slices, PlaneGeometry::Pointer(nullptr));

          if (m_Slices > 0)
          {
            m_PlaneGeometries[0] = planeGeometry;
          }

          m_SliceNavigationController->GetSlice()->SetSteps(m_Slices);

          this->Modified();

          // End Reinitialization

          if (m_SliceNavigationController)
          {
            m_SliceNavigationController->GetSlice()->SetPos(restorePlaneOp->GetPos());
            m_SliceNavigationController->AdjustSliceStepperRange();
          }
          BaseGeometry::ExecuteOperation(restorePlaneOp);
        }
      }
      else
      {
        // Reach through to all slices
        for (auto iter = m_PlaneGeometries.begin(); iter != m_PlaneGeometries.end(); ++iter)
        {
          (*iter)->ExecuteOperation(operation);
        }
      }
      break;

    case OpAPPLYTRANSFORMMATRIX:

      // Clear all generated geometries and then transform only the first slice.
      // The other slices will be re-generated on demand

      // Save first slice
      geometry2D = m_PlaneGeometries[0];

      applyMatrixOp = dynamic_cast<ApplyTransformMatrixOperation *>(operation);

      // Apply transformation to first plane
      geometry2D->ExecuteOperation(applyMatrixOp);

      // Generate a ApplyTransformMatrixOperation using the dataset center instead of
      // the supplied rotation center. The supplied center is instead used to adjust the
      // slice stack afterwards (see OpROTATE).
      center = m_ReferenceGeometry->GetCenter();

      // Clear the slice stack and adjust it according to the center of
      // the dataset and the supplied rotation center (see documentation of
      // ReinitializePlanes)
      this->ReinitializePlanes(center, applyMatrixOp->GetReferencePoint());

      BaseGeometry::ExecuteOperation(applyMatrixOp);
      break;

    default: // let handle by base class if we don't do anything
      BaseGeometry::ExecuteOperation(operation);
  }

  this->Modified();
}
