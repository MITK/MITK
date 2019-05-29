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

#include "mitkPlaneGeometry.h"
#include "mitkInteractionConst.h"
#include "mitkLine.h"
#include "mitkPlaneOperation.h"

#include <itkSpatialOrientationAdapter.h>

#include <vtkTransform.h>

#include <vnl/vnl_cross.h>

namespace mitk
{
  PlaneGeometry::PlaneGeometry() : Superclass(), m_ReferenceGeometry(nullptr) { Initialize(); }
  PlaneGeometry::~PlaneGeometry() {}
  PlaneGeometry::PlaneGeometry(const PlaneGeometry &other)
    : Superclass(other), m_ReferenceGeometry(other.m_ReferenceGeometry)
  {
  }

  bool PlaneGeometry::CheckRotationMatrix(mitk::AffineTransform3D *transform, double epsilon)
  {
    bool rotation = true;

    auto matrix = transform->GetMatrix().GetVnlMatrix();
    matrix.normalize_columns();

    auto det = vnl_determinant(matrix);
    if (fabs(det-1.0) > epsilon)
    {
      MITK_WARN << "Invalid rotation matrix! Determinant != 1 (" << det << ")";
      rotation = false;
    }

    vnl_matrix_fixed<double, 3, 3> id; id.set_identity();
    auto should_be_id = matrix*matrix.transpose();
    should_be_id -= id;
    auto max = should_be_id.absolute_value_max();
    if (max > epsilon)
    {
      MITK_WARN << "Invalid rotation matrix! R*R^T != ID. Max value: " << max << " (should be 0)";
      rotation = false;
    }

    return rotation;
  }

  void PlaneGeometry::CheckIndexToWorldTransform(mitk::AffineTransform3D *transform)
  {
    CheckRotationMatrix(transform);
  }

  void PlaneGeometry::CheckBounds(const BoundingBox::BoundsArrayType &bounds)
  {
    // error: unused parameter 'bounds'
    // this happens in release mode, where the assert macro is defined empty
    // hence we "use" the parameter:
    (void)bounds;

    // currently the unit rectangle must be starting at the origin [0,0]
    assert(bounds[0] == 0);
    assert(bounds[2] == 0);
    // the unit rectangle must be two-dimensional
    assert(bounds[1] > 0);
    assert(bounds[3] > 0);
  }

  void PlaneGeometry::IndexToWorld(const Point2D &pt_units, Point2D &pt_mm) const
  {
    pt_mm[0] = GetExtentInMM(0) / GetExtent(0) * pt_units[0];
    pt_mm[1] = GetExtentInMM(1) / GetExtent(1) * pt_units[1];
  }

  void PlaneGeometry::WorldToIndex(const Point2D &pt_mm, Point2D &pt_units) const
  {
    pt_units[0] = pt_mm[0] * (1.0 / (GetExtentInMM(0) / GetExtent(0)));
    pt_units[1] = pt_mm[1] * (1.0 / (GetExtentInMM(1) / GetExtent(1)));
  }

  void PlaneGeometry::IndexToWorld(const Point2D & /*atPt2d_units*/, const Vector2D &vec_units, Vector2D &vec_mm) const
  {
    MITK_WARN << "Warning! Call of the deprecated function PlaneGeometry::IndexToWorld(point, vec, vec). Use "
                 "PlaneGeometry::IndexToWorld(vec, vec) instead!";
    this->IndexToWorld(vec_units, vec_mm);
  }

  void PlaneGeometry::IndexToWorld(const Vector2D &vec_units, Vector2D &vec_mm) const
  {
    vec_mm[0] = (GetExtentInMM(0) / GetExtent(0)) * vec_units[0];
    vec_mm[1] = (GetExtentInMM(1) / GetExtent(1)) * vec_units[1];
  }

  void PlaneGeometry::WorldToIndex(const Point2D & /*atPt2d_mm*/, const Vector2D &vec_mm, Vector2D &vec_units) const
  {
    MITK_WARN << "Warning! Call of the deprecated function PlaneGeometry::WorldToIndex(point, vec, vec). Use "
                 "PlaneGeometry::WorldToIndex(vec, vec) instead!";
    this->WorldToIndex(vec_mm, vec_units);
  }

  void PlaneGeometry::WorldToIndex(const Vector2D &vec_mm, Vector2D &vec_units) const
  {
    vec_units[0] = vec_mm[0] * (1.0 / (GetExtentInMM(0) / GetExtent(0)));
    vec_units[1] = vec_mm[1] * (1.0 / (GetExtentInMM(1) / GetExtent(1)));
  }

  void PlaneGeometry::InitializeStandardPlane(mitk::ScalarType width,
                                              ScalarType height,
                                              const Vector3D &spacing,
                                              PlaneGeometry::PlaneOrientation planeorientation,
                                              ScalarType zPosition,
                                              bool frontside,
                                              bool rotated,
                                              bool top)
  {
    AffineTransform3D::Pointer transform;

    transform = AffineTransform3D::New();
    AffineTransform3D::MatrixType matrix;
    AffineTransform3D::MatrixType::InternalMatrixType &vnlmatrix = matrix.GetVnlMatrix();

    vnlmatrix.set_identity();
    vnlmatrix(0, 0) = spacing[0];
    vnlmatrix(1, 1) = spacing[1];
    vnlmatrix(2, 2) = spacing[2];
    transform->SetIdentity();
    transform->SetMatrix(matrix);

    InitializeStandardPlane(width, height, transform.GetPointer(), planeorientation, zPosition, frontside, rotated, top);
  }

  void PlaneGeometry::InitializeStandardPlane(mitk::ScalarType width,
                                              mitk::ScalarType height,
                                              const AffineTransform3D *transform /* = nullptr */,
                                              PlaneGeometry::PlaneOrientation planeorientation /* = Axial */,
                                              mitk::ScalarType zPosition /* = 0 */,
                                              bool frontside /* = true */,
                                              bool rotated /* = false */,
                                              bool top /* = true */)
  {
    Superclass::Initialize();

    /// construct standard view.

    // We define at the moment "frontside" as: axial from above,
    // coronal from front (nose), saggital from right.
    // TODO: Double check with medicals doctors or radiologists [ ].

    // We define the orientation in patient's view, e.g. LAI is in a axial cut
    // (parallel to the triangle ear-ear-nose):
    // first axis: To the left ear of the patient
    // seecond axis: To the nose of the patient
    // third axis: To the legs of the patient.

    // Options are: L/R left/right; A/P anterior/posterior; I/S inferior/superior
    // (AKA caudal/cranial).
    // We note on all cases in the following switch block r.h. for right handed
    // or l.h. for left handed to describe the different cases.
    // However, which system is chosen is defined at the end of the switch block.

    // CAVE / be careful: the vectors right and bottom are relative to the plane
    // and do NOT describe e.g. the right side of the patient.

    Point3D origin;
    /** Bottom means downwards, DV means Direction Vector. Both relative to the image! */
    VnlVector rightDV(3), bottomDV(3);
    /** Origin of this plane is by default a zero vector and implicitly in the top-left corner: */
    origin.Fill(0);
    /** This is different to all definitions in MITK, except the QT mouse clicks.
    *   But it is like this here and we don't want to change a running system.
    *   Just be aware, that IN THIS FUNCTION we define the origin at the top left (e.g. your screen). */

    /** NormalDirection defines which axis (i.e. column index in the transform matrix)
    * is perpendicular to the plane: */
    int normalDirection;

    switch (planeorientation) // Switch through our limited choice of standard planes:
    {
      case None:
      /** Orientation 'None' shall be done like the axial plane orientation,
       *  for whatever reasons. */
      case Axial:
        if (frontside) // Radiologist's view from below. A cut along the triangle ear-ear-nose.
        {
          if (rotated == false)
          /** Origin in the top-left corner, x=[1; 0; 0], y=[0; 1; 0], z=[0; 0; 1],
          *   origin=[0,0,zpos]: LAI (r.h.)
          *
          *  0---rightDV---->                            |
          *  |                                           |
          *  |  Picture of a finite, rectangular plane   |
          *  |  ( insert LOLCAT-scan here ^_^ )          |
          *  |                                           |
          *  v  _________________________________________|
          *
          */
          {
            FillVector3D(origin, 0, 0, zPosition);
            FillVector3D(rightDV, 1, 0, 0);
            FillVector3D(bottomDV, 0, 1, 0);
          }
          else // Origin rotated to the bottom-right corner, x=[-1; 0; 0], y=[0; -1; 0], z=[0; 0; 1],
               // origin=[w,h,zpos]: RPI (r.h.)
          {    // Caveat emptor:  Still  using  top-left  as  origin  of  index  coordinate  system!
            FillVector3D(origin, width, height, zPosition);
            FillVector3D(rightDV, -1, 0, 0);
            FillVector3D(bottomDV, 0, -1, 0);
          }
        }
        else // 'Backside, not frontside.' Neuro-Surgeons's view from above patient.
        {
          if (rotated == false) // x=[-1; 0; 0], y=[0; 1; 0], z=[0; 0; 1], origin=[w,0,zpos]:  RAS (r.h.)
          {
            FillVector3D(origin, width, 0, zPosition);
            FillVector3D(rightDV, -1, 0, 0);
            FillVector3D(bottomDV, 0, 1, 0);
          }
          else // Origin in the bottom-left corner, x=[1; 0; 0], y=[0; -1; 0], z=[0; 0; 1],
               // origin=[0,h,zpos]:  LPS (r.h.)
          {
            FillVector3D(origin, 0, height, zPosition);
            FillVector3D(rightDV, 1, 0, 0);
            FillVector3D(bottomDV, 0, -1, 0);
          }
        }
        normalDirection = 2; // That is S=Superior=z=third_axis=middlefinger in righthanded LPS-system.
        break;

      // Frontal is known as Coronal in mitk. Plane cuts through patient's ear-ear-heel-heel:
      case Frontal:
        if (frontside)
        {
          if (rotated == false) // x=[1; 0; 0], y=[0; 0; 1], z=[0; 1; 0], origin=[0,zpos,0]: LAI (r.h.)
          {
            FillVector3D(origin, 0, zPosition, 0);
            FillVector3D(rightDV, 1, 0, 0);
            FillVector3D(bottomDV, 0, 0, 1);
          }
          else // x=[-1;0;0], y=[0;0;-1], z=[0;1;0], origin=[w,zpos,h]:  RAS  (r.h.)
          {
            FillVector3D(origin, width, zPosition, height);
            FillVector3D(rightDV, -1, 0, 0);
            FillVector3D(bottomDV, 0, 0, -1);
          }
        }
        else
        {
          if (rotated == false) //  x=[-1;0;0], y=[0;0;1], z=[0;1;0], origin=[w,zpos,0]: RPI (r.h.)
          {
            FillVector3D(origin, width, zPosition, 0);
            FillVector3D(rightDV, -1, 0, 0);
            FillVector3D(bottomDV, 0, 0, 1);
          }
          else //  x=[1;0;0], y=[0;1;0], z=[0;0;-1], origin=[0,zpos,h]: LPS (r.h.)
          {
            FillVector3D(origin, 0, zPosition, height);
            FillVector3D(rightDV, 1, 0, 0);
            FillVector3D(bottomDV, 0, 0, -1);
          }
        }
        normalDirection = 1; // Normal vector = posterior direction.
        break;

      case Sagittal: // Sagittal=Medial plane, the symmetry-plane mirroring your face.
        if (frontside)
        {
          if (rotated == false) //  x=[0;1;0], y=[0;0;1], z=[1;0;0], origin=[zpos,0,0]:  LAI (r.h.)
          {
            FillVector3D(origin, zPosition, 0, 0);
            FillVector3D(rightDV, 0, 1, 0);
            FillVector3D(bottomDV, 0, 0, 1);
          }
          else //  x=[0;-1;0], y=[0;0;-1], z=[1;0;0], origin=[zpos,w,h]:  LPS (r.h.)
          {
            FillVector3D(origin, zPosition, width, height);
            FillVector3D(rightDV, 0, -1, 0);
            FillVector3D(bottomDV, 0, 0, -1);
          }
        }
        else
        {
          if (rotated == false) //  x=[0;-1;0], y=[0;0;1], z=[1;0;0], origin=[zpos,w,0]:  RPI (r.h.)
          {
            FillVector3D(origin, zPosition, width, 0);
            FillVector3D(rightDV, 0, -1, 0);
            FillVector3D(bottomDV, 0, 0, 1);
          }
          else //  x=[0;1;0], y=[0;0;-1], z=[1;0;0], origin=[zpos,0,h]:  RAS (r.h.)
          {
            FillVector3D(origin, zPosition, 0, height);
            FillVector3D(rightDV, 0, 1, 0);
            FillVector3D(bottomDV, 0, 0, -1);
          }
        }
        normalDirection = 0; // Normal vector = Lateral direction: Left in a LPS-system.
        break;

      default:
        itkExceptionMacro("unknown PlaneOrientation");
    }

    VnlVector normal(3);
    FillVector3D(normal, 0, 0, 0);
    normal[normalDirection] = top ? 1 : -1;

    if ( transform != nullptr )
    {
      origin = transform->TransformPoint( origin );
      rightDV = transform->TransformVector( rightDV );
      bottomDV = transform->TransformVector( bottomDV );
      normal = transform->TransformVector( normal );
    }

    ScalarType bounds[6] = {0, width, 0, height, 0, 1};
    this->SetBounds(bounds);

    AffineTransform3D::Pointer planeTransform = AffineTransform3D::New();
    Matrix3D matrix;
    matrix.GetVnlMatrix().set_column(0, rightDV);
    matrix.GetVnlMatrix().set_column(1, bottomDV);
    matrix.GetVnlMatrix().set_column(2, normal);
    planeTransform->SetMatrix(matrix);
    planeTransform->SetOffset(this->GetIndexToWorldTransform()->GetOffset());
    this->SetIndexToWorldTransform(planeTransform);

    this->SetOrigin(origin);
  }

  std::vector< int > PlaneGeometry::CalculateDominantAxes(mitk::AffineTransform3D::MatrixType::InternalMatrixType& rotation_matrix)
  {
    std::vector< int > axes;

    bool dominant_axis_error = false;
    for (int i = 0; i < 3; ++i)
    {
      int dominantAxis = itk::Function::Max3(
          rotation_matrix[0][i],
          rotation_matrix[1][i],
          rotation_matrix[2][i]
      );

      for (int j=0; j<i; ++j)
        if (axes[j] == dominantAxis)
        {
          dominant_axis_error = true;
          break;
        }
      if (dominant_axis_error)
        break;

      axes.push_back(dominantAxis);
    }

    if (dominant_axis_error)
    {
      MITK_DEBUG << "Error during dominant axis calculation. Using default.";
      MITK_DEBUG << "This is either caused by an imperfect rotation matrix or if the rotation is axactly 45° around one or more axis.";
      axes.clear();
      for (int i = 0; i < 3; ++i)
        axes.push_back(i);
    }

    return axes;
  }

  void PlaneGeometry::InitializeStandardPlane(const BaseGeometry *geometry3D,
                                              PlaneOrientation planeorientation,
                                              ScalarType zPosition,
                                              bool frontside,
                                              bool rotated,
                                              bool top)
  {
    this->SetReferenceGeometry(geometry3D);

    ScalarType width, height;

    // Inspired by:
    // http://www.na-mic.org/Wiki/index.php/Coordinate_System_Conversion_Between_ITK_and_Slicer3

    mitk::AffineTransform3D::MatrixType matrix = geometry3D->GetIndexToWorldTransform()->GetMatrix();

    matrix.GetVnlMatrix().normalize_columns();
    mitk::AffineTransform3D::MatrixType::InternalMatrixType inverseMatrix = matrix.GetTranspose();

    /// The index of the sagittal, coronal and axial axes in the reference geometry.
    auto axes = CalculateDominantAxes(inverseMatrix);
    /// The direction of the sagittal, coronal and axial axes in the reference geometry.
    /// +1 means that the direction is straight, i.e. greater index translates to greater
    /// world coordinate. -1 means that the direction is inverted.
    int directions[3];
    ScalarType extents[3];
    ScalarType spacings[3];
    for (int i=0; i<3; ++i)
    {
      int dominantAxis = axes.at(i);
      directions[i] = itk::Function::Sign(inverseMatrix[dominantAxis][i]);
      extents[i] = geometry3D->GetExtent(dominantAxis);
      spacings[i] = geometry3D->GetSpacing()[dominantAxis];
    }

    // matrix(column) = inverseTransformMatrix(row) * flippedAxes * spacing
    matrix[0][0] = inverseMatrix[axes[0]][0] * directions[0] * spacings[0];
    matrix[1][0] = inverseMatrix[axes[0]][1] * directions[0] * spacings[0];
    matrix[2][0] = inverseMatrix[axes[0]][2] * directions[0] * spacings[0];
    matrix[0][1] = inverseMatrix[axes[1]][0] * directions[1] * spacings[1];
    matrix[1][1] = inverseMatrix[axes[1]][1] * directions[1] * spacings[1];
    matrix[2][1] = inverseMatrix[axes[1]][2] * directions[1] * spacings[1];
    matrix[0][2] = inverseMatrix[axes[2]][0] * directions[2] * spacings[2];
    matrix[1][2] = inverseMatrix[axes[2]][1] * directions[2] * spacings[2];
    matrix[2][2] = inverseMatrix[axes[2]][2] * directions[2] * spacings[2];

    /// The "world origin" is the corner with the lowest physical coordinates.
    /// We use it as a reference point so that we get the correct anatomical
    /// orientations.
    Point3D worldOrigin = geometry3D->GetOrigin();
    for (int i = 0; i < 3; ++i)
    {
      /// The distance of the plane origin from the world origin in voxels.
      double offset = directions[i] > 0 ? 0.0 : extents[i];

      if (geometry3D->GetImageGeometry())
      {
        offset += directions[i] * 0.5;
      }

      for (int j = 0; j < 3; ++j)
      {
        worldOrigin[j] -= offset * matrix[j][i];
      }
    }

    switch(planeorientation)
    {
    case None:
    /** Orientation 'None' shall be done like the axial plane orientation,
     *  for whatever reasons. */
    case Axial:
      width  = extents[0];
      height = extents[1];
      break;
    case Frontal:
      width  = extents[0];
      height = extents[2];
      break;
    case Sagittal:
      width  = extents[1];
      height = extents[2];
      break;
    default:
      itkExceptionMacro("unknown PlaneOrientation");
    }

    ScalarType bounds[6]= { 0, width, 0, height, 0, 1 };
    this->SetBounds( bounds );

    AffineTransform3D::Pointer transform = AffineTransform3D::New();
    transform->SetMatrix(matrix);
    transform->SetOffset(worldOrigin.GetVectorFromOrigin());

    InitializeStandardPlane(
      width, height, transform, planeorientation, zPosition, frontside, rotated, top);
  }

  void PlaneGeometry::InitializeStandardPlane(
    const BaseGeometry *geometry3D, bool top, PlaneOrientation planeorientation, bool frontside, bool rotated)
  {
    /// The index of the sagittal, coronal and axial axes in world coordinate system.
    int worldAxis;
    switch(planeorientation)
    {
    case None:
    /** Orientation 'None' shall be done like the axial plane orientation,
     *  for whatever reasons. */
    case Axial:
      worldAxis = 2;
      break;
    case Frontal:
      worldAxis = 1;
      break;
    case Sagittal:
      worldAxis = 0;
      break;
    default:
      itkExceptionMacro("unknown PlaneOrientation");
    }

    // Inspired by:
    // http://www.na-mic.org/Wiki/index.php/Coordinate_System_Conversion_Between_ITK_and_Slicer3

    mitk::AffineTransform3D::ConstPointer affineTransform = geometry3D->GetIndexToWorldTransform();
    mitk::AffineTransform3D::MatrixType matrix = affineTransform->GetMatrix();
    matrix.GetVnlMatrix().normalize_columns();
    mitk::AffineTransform3D::MatrixType::InternalMatrixType inverseMatrix = matrix.GetInverse();

    /// The index of the sagittal, coronal and axial axes in the reference geometry.
    int dominantAxis = CalculateDominantAxes(inverseMatrix).at(worldAxis);

    ScalarType zPosition = top ? 0.5 : geometry3D->GetExtent(dominantAxis) - 0.5;

    InitializeStandardPlane(geometry3D, planeorientation, zPosition, frontside, rotated, top);
  }

  void PlaneGeometry::InitializeStandardPlane(const Vector3D &rightVector,
                                              const Vector3D &downVector,
                                              const Vector3D *spacing)
  {
    InitializeStandardPlane(rightVector.GetVnlVector(), downVector.GetVnlVector(), spacing);
  }

  void PlaneGeometry::InitializeStandardPlane(const VnlVector &rightVector,
                                              const VnlVector &downVector,
                                              const Vector3D *spacing)
  {
    ScalarType width = rightVector.two_norm();
    ScalarType height = downVector.two_norm();

    InitializeStandardPlane(width, height, rightVector, downVector, spacing);
  }

  void PlaneGeometry::InitializeStandardPlane(mitk::ScalarType width,
                                              ScalarType height,
                                              const Vector3D &rightVector,
                                              const Vector3D &downVector,
                                              const Vector3D *spacing)
  {
    InitializeStandardPlane(width, height, rightVector.GetVnlVector(), downVector.GetVnlVector(), spacing);
  }

  void PlaneGeometry::InitializeStandardPlane(mitk::ScalarType width,
                                              ScalarType height,
                                              const VnlVector &rightVector,
                                              const VnlVector &downVector,
                                              const Vector3D *spacing)
  {
    assert(width > 0);
    assert(height > 0);

    VnlVector rightDV = rightVector;
    rightDV.normalize();
    VnlVector downDV = downVector;
    downDV.normalize();
    VnlVector normal = vnl_cross_3d(rightVector, downVector);
    normal.normalize();
    // Crossproduct vnl_cross_3d is always righthanded, but that is okay here
    // because in this method we create a new IndexToWorldTransform and
    // spacing with 1 or 3 negative components could still make it lefthanded.

    if (spacing != nullptr)
    {
      rightDV *= (*spacing)[0];
      downDV *= (*spacing)[1];
      normal *= (*spacing)[2];
    }

    AffineTransform3D::Pointer transform = AffineTransform3D::New();
    Matrix3D matrix;
    matrix.GetVnlMatrix().set_column(0, rightDV);
    matrix.GetVnlMatrix().set_column(1, downDV);
    matrix.GetVnlMatrix().set_column(2, normal);
    transform->SetMatrix(matrix);
    transform->SetOffset(this->GetIndexToWorldTransform()->GetOffset());

    ScalarType bounds[6] = {0, width, 0, height, 0, 1};
    this->SetBounds(bounds);

    this->SetIndexToWorldTransform(transform);
  }

  void PlaneGeometry::InitializePlane(const Point3D &origin, const Vector3D &normal)
  {
    VnlVector rightVectorVnl(3), downVectorVnl;

    if (Equal(normal[1], 0.0f) == false)
    {
      FillVector3D(rightVectorVnl, 1.0f, -normal[0] / normal[1], 0.0f);
      rightVectorVnl.normalize();
    }
    else
    {
      FillVector3D(rightVectorVnl, 0.0f, 1.0f, 0.0f);
    }
    downVectorVnl = vnl_cross_3d(normal.GetVnlVector(), rightVectorVnl);
    downVectorVnl.normalize();
    // Crossproduct vnl_cross_3d is always righthanded.

    InitializeStandardPlane(rightVectorVnl, downVectorVnl);

    SetOrigin(origin);
  }

  void PlaneGeometry::SetMatrixByVectors(const VnlVector &rightVector,
                                         const VnlVector &downVector,
                                         ScalarType thickness /* = 1.0 */)
  {
    VnlVector normal = vnl_cross_3d(rightVector, downVector);
    normal.normalize();
    normal *= thickness;
    // Crossproduct vnl_cross_3d is always righthanded, but that is okay here
    // because in this method we create a new IndexToWorldTransform and
    // a negative thickness could still make it lefthanded.

    AffineTransform3D::Pointer transform = AffineTransform3D::New();
    Matrix3D matrix;
    matrix.GetVnlMatrix().set_column(0, rightVector);
    matrix.GetVnlMatrix().set_column(1, downVector);
    matrix.GetVnlMatrix().set_column(2, normal);
    transform->SetMatrix(matrix);
    transform->SetOffset(this->GetIndexToWorldTransform()->GetOffset());
    SetIndexToWorldTransform(transform);
  }

  Vector3D PlaneGeometry::GetNormal() const
  {
    Vector3D frontToBack;
    frontToBack.SetVnlVector(this->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2));

    return frontToBack;
  }

  VnlVector PlaneGeometry::GetNormalVnl() const
  {
    return this->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2);
  }

  ScalarType PlaneGeometry::DistanceFromPlane(const Point3D &pt3d_mm) const { return fabs(SignedDistance(pt3d_mm)); }
  ScalarType PlaneGeometry::SignedDistance(const Point3D &pt3d_mm) const { return SignedDistanceFromPlane(pt3d_mm); }
  bool PlaneGeometry::IsAbove(const Point3D &pt3d_mm, bool considerBoundingBox) const
  {
    if (considerBoundingBox)
    {
      Point3D pt3d_units;
      BaseGeometry::WorldToIndex(pt3d_mm, pt3d_units);
      return (pt3d_units[2] > this->GetBoundingBox()->GetBounds()[4]);
    }
    else
      return SignedDistanceFromPlane(pt3d_mm) > 0;
  }

  bool PlaneGeometry::IntersectionLine(const PlaneGeometry *plane, Line3D &crossline) const
  {
    Vector3D normal = this->GetNormal();
    normal.Normalize();

    Vector3D planeNormal = plane->GetNormal();
    planeNormal.Normalize();

    Vector3D direction = itk::CrossProduct(normal, planeNormal);

    if (direction.GetSquaredNorm() < eps)
      return false;

    crossline.SetDirection(direction);

    double N1dN2 = normal * planeNormal;
    double determinant = 1.0 - N1dN2 * N1dN2;

    Vector3D origin = this->GetOrigin().GetVectorFromOrigin();
    Vector3D planeOrigin = plane->GetOrigin().GetVectorFromOrigin();

    double d1 = normal * origin;
    double d2 = planeNormal * planeOrigin;

    double c1 = (d1 - d2 * N1dN2) / determinant;
    double c2 = (d2 - d1 * N1dN2) / determinant;

    Vector3D p = normal * c1 + planeNormal * c2;
    crossline.GetPoint().GetVnlVector() = p.GetVnlVector();

    return true;
  }

  unsigned int PlaneGeometry::IntersectWithPlane2D(const PlaneGeometry *plane, Point2D &lineFrom, Point2D &lineTo) const
  {
    Line3D crossline;
    if (this->IntersectionLine(plane, crossline) == false)
      return 0;

    Point2D point2;
    Vector2D direction2;

    this->Map(crossline.GetPoint(), point2);
    this->Map(crossline.GetPoint(), crossline.GetDirection(), direction2);

    return Line3D::RectangleLineIntersection(
      0, 0, GetExtentInMM(0), GetExtentInMM(1), point2, direction2, lineFrom, lineTo);
  }

  double PlaneGeometry::Angle(const PlaneGeometry *plane) const
  {
    return angle(plane->GetMatrixColumn(2), GetMatrixColumn(2));
  }

  double PlaneGeometry::Angle(const Line3D &line) const
  {
    return vnl_math::pi_over_2 - angle(line.GetDirection().GetVnlVector(), GetMatrixColumn(2));
  }

  bool PlaneGeometry::IntersectionPoint(const Line3D &line, Point3D &intersectionPoint) const
  {
    Vector3D planeNormal = this->GetNormal();
    planeNormal.Normalize();

    Vector3D lineDirection = line.GetDirection();
    lineDirection.Normalize();

    double t = planeNormal * lineDirection;
    if (fabs(t) < eps)
    {
      return false;
    }

    Vector3D diff;
    diff = this->GetOrigin() - line.GetPoint();
    t = (planeNormal * diff) / t;

    intersectionPoint = line.GetPoint() + lineDirection * t;
    return true;
  }

  bool PlaneGeometry::IntersectionPointParam(const Line3D &line, double &t) const
  {
    Vector3D planeNormal = this->GetNormal();

    Vector3D lineDirection = line.GetDirection();

    t = planeNormal * lineDirection;

    if (fabs(t) < eps)
    {
      return false;
    }

    Vector3D diff;
    diff = this->GetOrigin() - line.GetPoint();
    t = (planeNormal * diff) / t;
    return true;
  }

  bool PlaneGeometry::IsParallel(const PlaneGeometry *plane) const
  {
    return ((Angle(plane) < 10.0 * mitk::sqrteps) || (Angle(plane) > (vnl_math::pi - 10.0 * sqrteps)));
  }

  bool PlaneGeometry::IsOnPlane(const Point3D &point) const { return Distance(point) < eps; }
  bool PlaneGeometry::IsOnPlane(const Line3D &line) const
  {
    return ((Distance(line.GetPoint()) < eps) && (Distance(line.GetPoint2()) < eps));
  }

  bool PlaneGeometry::IsOnPlane(const PlaneGeometry *plane) const
  {
    return (IsParallel(plane) && (Distance(plane->GetOrigin()) < eps));
  }

  Point3D PlaneGeometry::ProjectPointOntoPlane(const Point3D &pt) const
  {
    ScalarType len = this->GetNormalVnl().two_norm();
    return pt - this->GetNormal() * this->SignedDistanceFromPlane(pt) / len;
  }

  itk::LightObject::Pointer PlaneGeometry::InternalClone() const
  {
    Self::Pointer newGeometry = new PlaneGeometry(*this);
    newGeometry->UnRegister();
    return newGeometry.GetPointer();
  }

  void PlaneGeometry::ExecuteOperation(Operation *operation)
  {
    vtkTransform *transform = vtkTransform::New();
    transform->SetMatrix(this->GetVtkMatrix());

    switch (operation->GetOperationType())
    {
      case OpORIENT:
      {
        auto *planeOp = dynamic_cast<mitk::PlaneOperation *>(operation);
        if (planeOp == nullptr)
        {
          return;
        }

        Point3D center = planeOp->GetPoint();

        Vector3D orientationVector = planeOp->GetNormal();
        Vector3D defaultVector;
        FillVector3D(defaultVector, 0.0, 0.0, 1.0);

        Vector3D rotationAxis = itk::CrossProduct(orientationVector, defaultVector);
        // double rotationAngle = acos( orientationVector[2] / orientationVector.GetNorm() );

        double rotationAngle = atan2((double)rotationAxis.GetNorm(), (double)(orientationVector * defaultVector));
        rotationAngle *= 180.0 / vnl_math::pi;

        transform->PostMultiply();
        transform->Identity();
        transform->Translate(center[0], center[1], center[2]);
        transform->RotateWXYZ(rotationAngle, rotationAxis[0], rotationAxis[1], rotationAxis[2]);
        transform->Translate(-center[0], -center[1], -center[2]);
        break;
      }
      case OpRESTOREPLANEPOSITION:
      {
        auto *op = dynamic_cast<mitk::RestorePlanePositionOperation *>(operation);
        if (op == nullptr)
        {
          return;
        }

        AffineTransform3D::Pointer transform2 = AffineTransform3D::New();
        Matrix3D matrix;
        matrix.GetVnlMatrix().set_column(0, op->GetTransform()->GetMatrix().GetVnlMatrix().get_column(0));
        matrix.GetVnlMatrix().set_column(1, op->GetTransform()->GetMatrix().GetVnlMatrix().get_column(1));
        matrix.GetVnlMatrix().set_column(2, op->GetTransform()->GetMatrix().GetVnlMatrix().get_column(2));
        transform2->SetMatrix(matrix);
        Vector3D offset = op->GetTransform()->GetOffset();
        transform2->SetOffset(offset);

        this->SetIndexToWorldTransform(transform2);
        ScalarType bounds[6] = {0, op->GetWidth(), 0, op->GetHeight(), 0, 1};
        this->SetBounds(bounds);
        this->Modified();
        transform->Delete();
        return;
      }
      default:
        Superclass::ExecuteOperation(operation);
        transform->Delete();
        return;
    }

    this->SetVtkMatrixDeepCopy(transform);
    this->Modified();
    transform->Delete();
  }

  void PlaneGeometry::PrintSelf(std::ostream &os, itk::Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
    os << indent << " ScaleFactorMMPerUnitX: " << GetExtentInMM(0) / GetExtent(0) << std::endl;
    os << indent << " ScaleFactorMMPerUnitY: " << GetExtentInMM(1) / GetExtent(1) << std::endl;
    os << indent << " Normal: " << GetNormal() << std::endl;
  }

  bool PlaneGeometry::Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const
  {
    assert(this->IsBoundingBoxNull() == false);

    Point3D pt3d_units;
    Superclass::WorldToIndex(pt3d_mm, pt3d_units);
    pt2d_mm[0] = pt3d_units[0] * GetExtentInMM(0) / GetExtent(0);
    pt2d_mm[1] = pt3d_units[1] * GetExtentInMM(1) / GetExtent(1);
    pt3d_units[2] = 0;
    return this->GetBoundingBox()->IsInside(pt3d_units);
  }

  void PlaneGeometry::Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const
  {
    // pt2d_mm is measured from the origin of the world geometry (at leats it called form BaseRendere::Mouse...Event)
    Point3D pt3d_units;
    pt3d_units[0] = pt2d_mm[0] / (GetExtentInMM(0) / GetExtent(0));
    pt3d_units[1] = pt2d_mm[1] / (GetExtentInMM(1) / GetExtent(1));
    pt3d_units[2] = 0;
    // pt3d_units is a continuos index. We divided it with the Scale Factor (= spacing in x and y) to convert it from mm
    // to index units.
    //
    pt3d_mm = GetIndexToWorldTransform()->TransformPoint(pt3d_units);
    // now we convert the 3d index to a 3D world point in mm. We could have used IndexToWorld as well as
    // GetITW->Transform...
  }

  void PlaneGeometry::SetSizeInUnits(mitk::ScalarType width, mitk::ScalarType height)
  {
    ScalarType bounds[6] = {0, width, 0, height, 0, 1};
    ScalarType extent, newextentInMM;
    if (GetExtent(0) > 0)
    {
      extent = GetExtent(0);
      if (width > extent)
        newextentInMM = GetExtentInMM(0) / width * extent;
      else
        newextentInMM = GetExtentInMM(0) * extent / width;
      SetExtentInMM(0, newextentInMM);
    }
    if (GetExtent(1) > 0)
    {
      extent = GetExtent(1);
      if (width > extent)
        newextentInMM = GetExtentInMM(1) / height * extent;
      else
        newextentInMM = GetExtentInMM(1) * extent / height;
      SetExtentInMM(1, newextentInMM);
    }
    SetBounds(bounds);
  }

  bool PlaneGeometry::Project(const mitk::Point3D &pt3d_mm, mitk::Point3D &projectedPt3d_mm) const
  {
    assert(this->IsBoundingBoxNull() == false);

    Point3D pt3d_units;
    Superclass::WorldToIndex(pt3d_mm, pt3d_units);
    pt3d_units[2] = 0;
    projectedPt3d_mm = GetIndexToWorldTransform()->TransformPoint(pt3d_units);
    return this->GetBoundingBox()->IsInside(pt3d_units);
  }

  bool PlaneGeometry::Project(const mitk::Vector3D &vec3d_mm, mitk::Vector3D &projectedVec3d_mm) const
  {
    assert(this->IsBoundingBoxNull() == false);

    Vector3D vec3d_units;
    Superclass::WorldToIndex(vec3d_mm, vec3d_units);
    vec3d_units[2] = 0;
    projectedVec3d_mm = GetIndexToWorldTransform()->TransformVector(vec3d_units);
    return true;
  }

  bool PlaneGeometry::Project(const mitk::Point3D &atPt3d_mm,
                              const mitk::Vector3D &vec3d_mm,
                              mitk::Vector3D &projectedVec3d_mm) const
  {
    MITK_WARN << "Deprecated function! Call Project(vec3D,vec3D) instead.";
    assert(this->IsBoundingBoxNull() == false);

    Vector3D vec3d_units;
    Superclass::WorldToIndex(atPt3d_mm, vec3d_mm, vec3d_units);
    vec3d_units[2] = 0;
    projectedVec3d_mm = GetIndexToWorldTransform()->TransformVector(vec3d_units);

    Point3D pt3d_units;
    Superclass::WorldToIndex(atPt3d_mm, pt3d_units);
    return this->GetBoundingBox()->IsInside(pt3d_units);
  }

  bool PlaneGeometry::Map(const mitk::Point3D &atPt3d_mm,
                          const mitk::Vector3D &vec3d_mm,
                          mitk::Vector2D &vec2d_mm) const
  {
    Point2D pt2d_mm_start, pt2d_mm_end;
    Point3D pt3d_mm_end;
    bool inside = Map(atPt3d_mm, pt2d_mm_start);
    pt3d_mm_end = atPt3d_mm + vec3d_mm;
    inside &= Map(pt3d_mm_end, pt2d_mm_end);
    vec2d_mm = pt2d_mm_end - pt2d_mm_start;
    return inside;
  }

  void PlaneGeometry::Map(const mitk::Point2D & /*atPt2d_mm*/,
                          const mitk::Vector2D & /*vec2d_mm*/,
                          mitk::Vector3D & /*vec3d_mm*/) const
  {
    //@todo implement parallel to the other Map method!
    assert(false);
  }

  void PlaneGeometry::SetReferenceGeometry(const mitk::BaseGeometry *geometry) { m_ReferenceGeometry = geometry; }
  const mitk::BaseGeometry *PlaneGeometry::GetReferenceGeometry() const { return m_ReferenceGeometry; }
  bool PlaneGeometry::HasReferenceGeometry() const { return (m_ReferenceGeometry != nullptr); }
} // namespace
