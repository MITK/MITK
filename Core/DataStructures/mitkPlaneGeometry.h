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


#ifndef PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C
#define PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C

#include "mitkCommon.h"
#include "mitkGeometry2D.h"
#include "mitkLine.h"
#include <vnl/vnl_cross.h>

class vtkTransform;

namespace mitk {


//##ModelId=3DDE570F010A
//##Documentation
//## @brief Describes a two-dimensional, plane and rectangular surface
//## @ingroup Geometry
class PlaneGeometry : public Geometry2D
{
public:
  mitkClassMacro(PlaneGeometry,Geometry2D);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  enum PlaneOrientation {Transversal, Sagittal, Frontal};

  virtual void SetIndexToWorldTransform(mitk::AffineTransform3D* transform);

  virtual void SetBounds(const BoundingBox::BoundsArrayType& bounds);

  //##ModelId=3E3B9C730262
  virtual void UnitsToMM(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const;
  //##ModelId=3E3B9C760112
  virtual void MMToUnits(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const;

  //##ModelId=3E3B9C8C0145
  virtual void UnitsToMM(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const;
  //##ModelId=3E3B9C8E0152
  virtual void MMToUnits(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const;

  virtual void Modified() const;

  virtual void Initialize();

  //##Documentation
  //## @brief Initialize a plane with orientation @a planeorientation (default: transversal) with respect to 
  //## @a geometry3D (default: identity). Spacing also taken from @a geometry3D.
  //##
  //## @warning A former version of this method created a geometry with unit spacing. For unit spacing use
  //## \code
  //##   // for in-plane unit spacing:
  //##   thisgeometry->SetSizeInUnits(thisgeometry->GetExtentInMM(0), thisgeometry->GetExtentInMM(1));
  //##   // additionally, for unit spacing in normal direction (former version did not do this):
  //##   thisgeometry->SetExtentInMM(2, 1.0);
  //## \endcode
  virtual void InitializeStandardPlane(const mitk::Geometry3D* geometry3D, PlaneOrientation planeorientation = Transversal, mitk::ScalarType zPosition = 0, bool frontside=true);

  //##Documentation
  //## @brief Initialize a plane with orientation @a planeorientation (default: transversal) with respect to 
  //## @a transform (default: identity) given width and height in units.
  //##
  virtual void InitializeStandardPlane(mitk::ScalarType width, mitk::ScalarType height, const mitk::AffineTransform3D* transform=NULL, PlaneOrientation planeorientation = Transversal, mitk::ScalarType zPosition = 0, bool frontside=true);

  //##Documentation
  //## @brief Initialize plane with orientation @a planeorientation (default: transversal) 
  //## given width, height and spacing.
  //##
  virtual void InitializeStandardPlane(mitk::ScalarType width, mitk::ScalarType height, const Vector3D & spacing, PlaneOrientation planeorientation = Transversal, mitk::ScalarType zPosition = 0, bool frontside=true);

  //##Documentation
  //## @brief Initialize plane by right-/down-vector (itk) and spacing (default: 1.0 mm in all directions).
  //##
  //## The length of the right-/-down-vector is used as width/height in units, respectively. Then, 
  //## the vectors are normalized and multiplied by the respective spacing before they are set in the matrix.
  virtual void InitializeStandardPlane(const Vector3D& rightVector, const Vector3D& downVector, const Vector3D * spacing = NULL);

  //##Documentation
  //## @brief Initialize plane by right-/down-vector (vnl) and spacing (default: 1.0 mm in all directions).
  //##
  //## The length of the right-/-down-vector is used as width/height in units, respectively. Then, 
  //## the vectors are normalized and multiplied by the respective spacing before they are set in the matrix.
  virtual void InitializeStandardPlane(const VnlVector& rightVector, const VnlVector& downVector, const Vector3D * spacing = NULL);

  //##Documentation
  //## @brief Initialize plane by origin and normal (size is 1.0 mm in
  //## all directions, direction of right-/down-vector valid but 
  //## undefined).
  //## 
  virtual void InitializePlane(const Point3D& origin, const Vector3D& normal);

  AffineGeometryFrame3D::Pointer Clone() const;

  //##Documentation
  //## @brief Initialize plane by right-/down-vector.
  //##
  //## @warning The vectors are set into the matrix as they are, @em without normalization! band multiplied by the respective spacing before they are set in the matrix.
  void SetMatrixByVectors(const VnlVector& rightVector, const VnlVector& downVector, mitk::ScalarType thickness=1.0)
  {
    VnlVector normal = vnl_cross_3d(rightVector, downVector);
    normal.normalize();
    normal *= thickness;

    mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
    mitk::Matrix3D matrix;
    matrix.GetVnlMatrix().set_column(0, rightVector);
    matrix.GetVnlMatrix().set_column(1, downVector);
    matrix.GetVnlMatrix().set_column(2, normal);
    transform->SetMatrix(matrix);
    transform->SetOffset(const_cast<Point3D&>(m_Origin).Get_vnl_vector().data_block());
    SetIndexToWorldTransform(transform);
  }

  //##Documentation
  //## @brief Change @a transform so that the third column of the 
  //## transform-martix is perpendicular to the first two columns
  //##
  static void EnsurePerpendicularNormal(mitk::AffineTransform3D* transform);

  //##Documentation
  //## @brief Set origin of the plane (upper-left corner)
  //##
  //## The origin is equal to the offset of the m_IndexToWorldTransform
  void SetOrigin(const Point3D& origin)
  {
    if(origin!=GetOrigin())
    {
      m_IndexToWorldTransform->SetOffset(const_cast<Point3D&>(origin).Get_vnl_vector().data_block());
      m_Origin = origin;
      Modified();
    }
  }

  //##Documentation
  //## @brief Origin of the plane (upper-left corner)
  //##
  //## The origin is equal to the offset of the m_IndexToWorldTransform
  const Point3D& GetOrigin() const
  {
    return m_Origin;
  }

  //##Documentation
  //## @brief Origin of the plane as VnlVector (upper-left corner)
  //##
  //## The origin is equal to the offset of the m_IndexToWorldTransform
  VnlVector GetOriginVnl() const
  {
    return const_cast<Self*>(this)->m_Origin.Get_vnl_vector();
  }

  //##Documentation
  //## @brief Normal of the plane
  //##
  Vector3D GetNormal() const
  {
    return GetAxisVector(2);
  }

  //##Documentation
  //## @brief Normal of the plane as VnlVector
  //##
  VnlVector GetNormalVnl() const
  {
    return GetAxisVector(2).Get_vnl_vector();
  }

  //##Documentation
  //## @brief Distance of the point from the plane
  //## (bounding-box @em not considered)
  //##
  ScalarType Distance(const Point3D& point ) const 
  {
    return fabs(SignedDistance(point));
  }

  //##Documentation
  //## @brief Signed distance of the point from the plane
  //## (bounding-box @em not considered)
  //##
  //## > 0 : point is in the direction of the direction vector.
  inline ScalarType SignedDistance(const Point3D& point ) const 
  {
    ScalarType len = GetNormalVnl().two_norm();

    if( len == 0 )
      return 0;

    return (point-GetOrigin())*GetNormal() / len;
  }
  //##Documentation
  //## @brief Distance of the plane from another plane
  //## (bounding-box @em not considered)
  //##
  //## Result is 0 if planes are not parallel.
  ScalarType Distance(const PlaneGeometry* plane) const 
  {
    return fabs(SignedDistance(plane));
  }
  //##Documentation
  //## @brief Signed distance of the plane from another plane
  //## (bounding-box @em not considered)
  //##
  //## Result is 0 if planes are not parallel.
  inline ScalarType SignedDistance(const PlaneGeometry* plane) const 
  {
    if(IsParallel(plane))
    {
      return SignedDistance(plane->GetOrigin());
    }
    return 0;
  }
  //##Documentation
  //## @brief Calculate the intersecting line of two planes
  //##
  //## @return @a true planes are intersecting
  //## @return @a false planes do not intersect
  bool IntersectionLine( const PlaneGeometry* plane, Line3D& crossline ) const
  {
    VnlVector direction;

    Vector3D normal, planenormal;
    normal = GetNormal();
    planenormal = plane->GetNormal();

    direction = vnl_cross_3d(normal.Get_vnl_vector(), planenormal.Get_vnl_vector());

    if ( direction.squared_magnitude() < eps )
      return false; 

    Vector3D itkdirection;
    itkdirection.Set_vnl_vector(direction);
    crossline.SetDirection(itkdirection);

    double N1sq = normal.GetSquaredNorm();
    double N2sq = planenormal.GetSquaredNorm();
    double N1dN2 = normal*planenormal;

    double determinant = N1sq * N2sq - N1dN2*N1dN2;

    double d1=dot_product(normal.Get_vnl_vector(),GetOriginVnl());
    double d2=dot_product(planenormal.Get_vnl_vector(),plane->GetOriginVnl());

    double c1 = ( d1*N2sq - d2*N1dN2 ) / determinant;
    double c2 = ( d2*N1sq - d1*N1dN2) / determinant ;

    Vector3D vpoint;
    vpoint = normal*((ScalarType)c1)+planenormal*((ScalarType)c2);
    crossline.GetPoint().Get_vnl_vector()=vpoint.Get_vnl_vector();

    return true;
  }

  //##Documentation
  //## @brief Calculate two points where another plane intersects the border of this plane
  //##
  //## @return number of intersection points (0..2). First interection point (if existing) 
  //## is returned in @a lineFrom, second in @a lineTo.
  unsigned int IntersectWithPlane2D(const PlaneGeometry* plane, Point2D& lineFrom, Point2D& lineTo ) const 
  {
    VnlVector ptest;
    ptest = vnl_cross_3d(GetNormalVnl(), plane->GetNormalVnl());
    if(ptest.squared_magnitude()<vnl_math::float_sqrteps)
      return 0;

    Line3D crossline;
    if (IntersectionLine( plane, crossline ) == false)
      return 0;

    Point2D  point2; 
    Vector2D direction2; 

    Map(crossline.GetPoint(), point2);
    Map(crossline.GetPoint(), crossline.GetDirection(), direction2);

    return 
      mitk::Line3D::RectangleLineIntersection( 
      0, 0, GetExtentInMM(0), GetExtentInMM(1), 
      point2, direction2, lineFrom, lineTo );
  }

  //##Documentation
  //## @brief Calculate the angle between two planes
  //##
  //## @return angle in radiants
  double Angle( const PlaneGeometry* plane ) const 
  {
    return angle(plane->GetMatrixColumn(2), GetMatrixColumn(2));
  }

  //##Documentation
  //## @brief Calculate the angle between the plane and a line
  //##
  //## @return angle in radiants
  double Angle( const Line3D& line ) const 
  {
    return vnl_math::pi_over_2-angle(line.GetDirection().Get_vnl_vector(), GetMatrixColumn(2));
  }

  //##Documentation
  //## @brief Calculate intersection point between the plane and a line
  //##
  //## @param intersectionPoint intersection point
  //## @return @a true if @em unique intersection exists, i.e., if line
  //## is @em not on or parallel to the plane
  bool IntersectionPoint( const Line3D& line, mitk::Point3D & intersectionPoint ) const
  {
    Vector3D normal;
    normal.Set_vnl_vector(GetMatrixColumn(2));

    double t = normal*line.GetDirection();

    if ( fabs(t) < mitk::eps )
    {
      return false;
    }

    Vector3D diff;
    diff = GetOrigin() - line.GetPoint();
    t = (normal*diff) / t;

    intersectionPoint = line.GetPoint()+line.GetDirection()*t;

    return true;
  }

  //##Documentation
  //## @brief Calculate line parameter of intersection point between the 
  //## plane and a line
  //##
  //## @param t parameter of line: intersection point is 
  //## line.GetPoint()+t*line.GetDirection()
  //## @return @a true if @em unique intersection exists, i.e., if line
  //## is @em not on or parallel to the plane
  bool IntersectionPointParam( const Line3D& line, double & t ) const
  {
    Vector3D normal;
    normal.Set_vnl_vector(GetMatrixColumn(2));

    t = normal*line.GetDirection();

    if ( fabs(t) < mitk::eps )
    {
      return false;
    }

    Vector3D diff;
    diff = GetOrigin() - line.GetPoint();
    t = (normal*diff) / t;

    return true;
  }

  //##Documentation
  //## @brief Returns whether the plane is parallel to another plane
  //##
  bool IsParallel( const PlaneGeometry* plane ) const 
  {
    return Angle(plane) < mitk::eps;
  } 

  //##Documentation
  //## @brief Returns whether the point is on the plane
  //## (bounding-box @em not considered)
  //##
  bool IsOnPlane( const Point3D& point ) const 
  {
    return Distance(point) < mitk::eps;
  }

  //##Documentation
  //## @brief Returns whether the line is on the plane 
  //## (bounding-box @em not considered)
  //##
  bool IsOnPlane( const Line3D& line ) const
  {
    return ( (Distance( line.GetPoint() ) < mitk::eps) && (Distance( line.GetPoint2() ) < mitk::eps) );
  }

  //##Documentation
  //## @brief Returns whether the plane is on the plane
  //## (bounding-box @em not considered)
  //##
  bool IsOnPlane( const PlaneGeometry* plane ) const 
  {
    return ( IsParallel( plane ) && (Distance( plane->GetOrigin() ) < mitk::eps) );
  }

  //##Documentation
  //## @brief Returns the lot from the point to the plane
  //##
  Point3D Lot( const Point3D& pt ) const
  {
    return pt-GetNormal()*Distance(pt);
  }

  //##Documentation
  //## @brief Compares plane with another plane: @a true if IsOnPlane
  //## (bounding-box @em not considered)
  //##
  bool operator==( const PlaneGeometry* plane ) const 
  {
    return IsOnPlane( plane );
  }

  //##Documentation
  //## @brief Compares plane with another plane: @a false if IsOnPlane
  //## (bounding-box @em not considered)
  //##
  bool operator!=( const PlaneGeometry* plane ) const 
  {
    return !IsOnPlane( plane );
  }

protected:
  //##ModelId=3E395F22035A
  PlaneGeometry();
  //##ModelId=3E395F220382
  virtual ~PlaneGeometry();

  virtual void InitializeGeometry(Self * newGeometry) const;

  //##Documentation
  //## @brief Upper-left corner of the plane
  //## 
  //## The origin is equal to the offset of the m_IndexToWorldTransform
  Point3D m_Origin;

  //##ModelId=3E3BE12C012B
  //##Documentation
  //## @brief factor to convert x-coordinates from mm to units and vice versa
  //## 
  //## Is calculated in SetPlaneView from the value of m_WidthInUnits and the
  //## PlaneView
  mutable mitk::ScalarType m_ScaleFactorMMPerUnitX;
  //##ModelId=3E3BE12C0167
  //##Documentation
  //## @brief factor to convert y-coordinates from mm to units and vice versa
  //## 
  //## Is calculated in SetPlaneView from the value of m_HeightInUnits and
  //## the PlaneView
  mutable mitk::ScalarType m_ScaleFactorMMPerUnitY;
};

} // namespace mitk
#endif /* PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
