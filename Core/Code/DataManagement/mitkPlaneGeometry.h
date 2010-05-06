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


#ifndef PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C
#define PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C

#include "mitkCommon.h"
#include "mitkGeometry2D.h"

#include <vnl/vnl_cross.h>

namespace mitk {

template < class TCoordRep, unsigned int NPointDimension > class Line;
typedef Line<ScalarType, 3> Line3D;



/**
 * \brief Describes a two-dimensional, rectangular plane
 * 
 * \ingroup Geometry
 */
class MITK_CORE_EXPORT PlaneGeometry : public Geometry2D
{
public:
  mitkClassMacro(PlaneGeometry,Geometry2D);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  enum PlaneOrientation { Transversal, Sagittal, Frontal };

  virtual void IndexToWorld(const Point2D &pt_units, Point2D &pt_mm) const;
  
  virtual void WorldToIndex(const Point2D &pt_mm, Point2D &pt_units) const;

  virtual void IndexToWorld(const Point2D &atPt2d_units, 
    const Vector2D &vec_units, Vector2D &vec_mm) const;
  
  virtual void WorldToIndex(const Point2D &atPt2d_mm, 
    const Vector2D &vec_mm, Vector2D &vec_units) const;

  virtual void Initialize();

  /**
   * \brief Initialize a plane with orientation \a planeorientation
   * (default: transversal) with respect to \a geometry3D (default: identity). 
   * Spacing also taken from \a geometry3D.
   *
   * \warning A former version of this method created a geometry with unit 
   * spacing. For unit spacing use
   *
   * \code
   *   // for in-plane unit spacing:
   *   thisgeometry->SetSizeInUnits(thisgeometry->GetExtentInMM(0),
   *     thisgeometry->GetExtentInMM(1));
   *   // additionally, for unit spacing in normal direction (former version
   *   // did not do this):
   *   thisgeometry->SetExtentInMM(2, 1.0);
   * \endcode
   */
  virtual void InitializeStandardPlane( const Geometry3D* geometry3D, 
    PlaneOrientation planeorientation = Transversal, ScalarType zPosition = 0, 
    bool frontside=true, bool rotated=false );


  /**
   * \brief Initialize a plane with orientation \a planeorientation
   * (default: transversal) with respect to \a geometry3D (default: identity).
   * Spacing also taken from \a geometry3D.
   *
   * \param top if \a true, create plane at top, otherwise at bottom 
   * (for PlaneOrientation Transversal, for other plane locations respectively)
   */
  virtual void InitializeStandardPlane( const Geometry3D* geometry3D, bool top,
    PlaneOrientation planeorientation = Transversal,
    bool frontside=true, bool rotated=false );


  /**
   * \brief Initialize a plane with orientation \a planeorientation 
   * (default: transversal) with respect to \a transform (default: identity)
   * given width and height in units.
   *
   */
  virtual void InitializeStandardPlane( ScalarType width, ScalarType height, 
    const AffineTransform3D* transform = NULL, 
    PlaneOrientation planeorientation = Transversal, 
    ScalarType zPosition = 0, bool frontside=true, bool rotated=false );


  /**
   * \brief Initialize plane with orientation \a planeorientation
   * (default: transversal) given width, height and spacing.
   *
   */
  virtual void InitializeStandardPlane( ScalarType width, ScalarType height,
    const Vector3D & spacing, PlaneOrientation planeorientation = Transversal, 
    ScalarType zPosition = 0, bool frontside = true, bool rotated = false );

  /**
   * \brief Initialize plane by width and height in pixels, right-/down-vector 
   * (itk) to describe orientation in world-space (vectors will be normalized)
   * and spacing (default: 1.0 mm in all directions).
   *
   * The vectors are normalized and multiplied by the respective spacing before 
   * they are set in the matrix.
   */
  virtual void InitializeStandardPlane( ScalarType width, ScalarType height, 
    const Vector3D& rightVector, const Vector3D& downVector, 
    const Vector3D *spacing = NULL );


  /**
   * \brief Initialize plane by width and height in pixels,
   * right-/down-vector (vnl) to describe orientation in world-space (vectors
   * will be normalized) and spacing (default: 1.0 mm in all directions).
   *
   * The vectors are normalized and multiplied by the respective spacing 
   * before they are set in the matrix.
   */
  virtual void InitializeStandardPlane( ScalarType width, ScalarType height, 
    const VnlVector& rightVector, const VnlVector& downVector, 
    const Vector3D * spacing = NULL );


  /**
   * \brief Initialize plane by right-/down-vector (itk) and spacing
   * (default: 1.0 mm in all directions).
   *
   * The length of the right-/-down-vector is used as width/height in units,
   * respectively. Then, the vectors are normalized and multiplied by the 
   * respective spacing before they are set in the matrix.
   */
  virtual void InitializeStandardPlane( const Vector3D& rightVector, 
    const Vector3D& downVector, const Vector3D * spacing = NULL );


  /**
   * \brief Initialize plane by right-/down-vector (vnl) and spacing 
   * (default: 1.0 mm in all directions).
   *
   * The length of the right-/-down-vector is used as width/height in units, 
   * respectively. Then, the vectors are normalized and multiplied by the 
   * respective spacing before they are set in the matrix.
   */
  virtual void InitializeStandardPlane( const VnlVector& rightVector, 
    const VnlVector& downVector, const Vector3D * spacing = NULL );

  /**
   * \brief Initialize plane by origin and normal (size is 1.0 mm in
   * all directions, direction of right-/down-vector valid but 
   * undefined).
   * 
   */
  virtual void InitializePlane( const Point3D& origin, const Vector3D& normal);

  /**
   * \brief Initialize plane by right-/down-vector.
   *
   * \warning The vectors are set into the matrix as they are, 
   * \em without normalization!
   */
  void SetMatrixByVectors( const VnlVector& rightVector, 
    const VnlVector& downVector, ScalarType thickness=1.0 );


  /**
   * \brief Change \a transform so that the third column of the 
   * transform-martix is perpendicular to the first two columns
   *
   */
  static void EnsurePerpendicularNormal( AffineTransform3D* transform );


  /**
   * \brief Normal of the plane
   *
   */
  Vector3D GetNormal() const;


  /**
   * \brief Normal of the plane as VnlVector
   *
   */
  VnlVector GetNormalVnl() const;


  virtual ScalarType SignedDistance( const Point3D& pt3d_mm ) const;


  virtual bool IsAbove( const Point3D& pt3d_mm ) const;


  /**
   * \brief Distance of the point from the plane
   * (bounding-box \em not considered)
   *
   */
  ScalarType DistanceFromPlane( const Point3D& pt3d_mm ) const ;


  /**
   * \brief Signed distance of the point from the plane
   * (bounding-box \em not considered)
   *
   * > 0 : point is in the direction of the direction vector.
   */
  inline ScalarType SignedDistanceFromPlane( const Point3D& pt3d_mm ) const 
  {
    ScalarType len = GetNormalVnl().two_norm();

    if( len == 0 )
      return 0;

    return (pt3d_mm-GetOrigin())*GetNormal() / len;
  }


  /**
   * \brief Distance of the plane from another plane
   * (bounding-box \em not considered)
   *
   * Result is 0 if planes are not parallel.
   */
  ScalarType DistanceFromPlane(const PlaneGeometry* plane) const 
  {
    return fabs(SignedDistanceFromPlane(plane));
  }


  /**
   * \brief Signed distance of the plane from another plane
   * (bounding-box \em not considered)
   *
   * Result is 0 if planes are not parallel.
   */
  inline ScalarType SignedDistanceFromPlane( const PlaneGeometry *plane ) const 
  {
    if(IsParallel(plane))
    {
      return SignedDistance(plane->GetOrigin());
    }
    return 0;
  }
  
  
  /**
   * \brief Calculate the intersecting line of two planes
   *
   * \return \a true planes are intersecting
   * \return \a false planes do not intersect
   */
  bool IntersectionLine( const PlaneGeometry *plane, Line3D &crossline ) const;


  /**
   * \brief Calculate two points where another plane intersects the border of this plane
   *
   * \return number of intersection points (0..2). First interection point (if existing) 
   * is returned in \a lineFrom, second in \a lineTo.
   */
  unsigned int IntersectWithPlane2D(const PlaneGeometry *plane, 
    Point2D &lineFrom, Point2D &lineTo ) const ;


  /**
   * \brief Calculate the angle between two planes
   *
   * \return angle in radiants
   */
  double Angle( const PlaneGeometry *plane ) const;


  /**
   * \brief Calculate the angle between the plane and a line
   *
   * \return angle in radiants
   */
  double Angle( const Line3D &line ) const;


  /**
   * \brief Calculate intersection point between the plane and a line
   *
   * \param intersectionPoint intersection point
   * \return \a true if \em unique intersection exists, i.e., if line
   * is \em not on or parallel to the plane
   */
  bool IntersectionPoint( const Line3D &line,
    Point3D &intersectionPoint ) const;
  

  /**
   * \brief Calculate line parameter of intersection point between the 
   * plane and a line
   *
   * \param t parameter of line: intersection point is 
   * line.GetPoint()+t*line.GetDirection()
   * \return \a true if \em unique intersection exists, i.e., if line
   * is \em not on or parallel to the plane
   */
  bool IntersectionPointParam( const Line3D &line, double &t ) const;


  /**
   * \brief Returns whether the plane is parallel to another plane
   *
   * @return true iff the normal vectors both point to the same or exactly oposit direction
   */
  bool IsParallel( const PlaneGeometry *plane ) const;

  
  /**
   * \brief Returns whether the point is on the plane
   * (bounding-box \em not considered)
   */
  bool IsOnPlane( const Point3D &point ) const;


  /**
   * \brief Returns whether the line is on the plane 
   * (bounding-box \em not considered)
   */
  bool IsOnPlane( const Line3D &line ) const;

  
  /**
   * \brief Returns whether the plane is on the plane
   * (bounding-box \em not considered)
   * 
   * @return true iff the normal vector of the planes point to the same or the exactly oposit direction and 
   *  the distance of the planes is < eps
   *  
   */
  bool IsOnPlane( const PlaneGeometry *plane ) const;


  /**
   * \brief Returns the lot from the point to the plane
   */
  Point3D ProjectPointOntoPlane( const Point3D &pt ) const;


  virtual void SetIndexToWorldTransform( AffineTransform3D *transform);


  virtual void SetBounds( const BoundingBox::BoundsArrayType &bounds );


  AffineGeometryFrame3D::Pointer Clone() const;


  /** Implements operation to re-orient the plane */
  virtual void ExecuteOperation( Operation *operation );


protected:
  PlaneGeometry();

  virtual ~PlaneGeometry();

  virtual void InitializeGeometry( Self *newGeometry ) const;

  virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const;
  
private:
   /**
   * \brief Compares plane with another plane: \a true if IsOnPlane
   * (bounding-box \em not considered)
   */
  virtual bool operator==( const PlaneGeometry *plane ) const;

  /**
   * \brief Compares plane with another plane: \a false if IsOnPlane
   * (bounding-box \em not considered)
   */
  virtual bool operator!=( const PlaneGeometry *plane ) const;

};

} // namespace mitk


#endif /* PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
