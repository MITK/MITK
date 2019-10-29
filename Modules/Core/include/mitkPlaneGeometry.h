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

/**
* \brief Describes the geometry of a plane object
*
* Describes a two-dimensional manifold, i.e., to put it simply,
* an object that can be described using a 2D coordinate-system.
*
* PlaneGeometry can map points between 3D world coordinates
* (in mm) and the described 2D coordinate-system (in mm) by first projecting
* the 3D point onto the 2D manifold and then calculating the 2D-coordinates
* (in mm). These 2D-mm-coordinates can be further converted into
* 2D-unit-coordinates (e.g., pixels), giving a parameter representation of
* the object with parameter values inside a rectangle
* (e.g., [0,0]..[width, height]), which is the bounding box (bounding range
* in z-direction always [0]..[1]).
*
* A PlaneGeometry describes the 2D representation within a 3D object (derived from BaseGeometry). For example,
* a single CT-image (slice) is 2D in the sense that you can access the
* pixels using 2D-coordinates, but is also 3D, as the pixels are really
* voxels, thus have an extension (thickness) in the 3rd dimension.
*
*
* Optionally, a reference BaseGeometry can be specified, which usually would
* be the geometry associated with the underlying dataset. This is currently
* used for calculating the intersection of inclined / rotated planes
* (represented as PlaneGeometry) with the bounding box of the associated
* BaseGeometry.
*
* \warning The PlaneGeometry are not necessarily up-to-date and not even
* initialized. As described in the previous paragraph, one of the
* Generate-/Copy-/UpdateOutputInformation methods have to initialize it.
* mitk::BaseData::GetPlaneGeometry() makes sure, that the PlaneGeometry is
* up-to-date before returning it (by setting the update extent appropriately
* and calling UpdateOutputInformation).
*
* Rule: everything is in mm (or ms for temporal information) if not
* stated otherwise.
* \ingroup Geometry
*/

#ifndef PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C
#define PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C

#include "mitkBaseGeometry.h"
#include "mitkRestorePlanePositionOperation.h"
#include <MitkCoreExports.h>

#include <vnl/vnl_cross.h>

namespace mitk
{
  template <class TCoordRep, unsigned int NPointDimension>
  class Line;
  typedef Line<ScalarType, 3> Line3D;

  class PlaneGeometry;
  /** \deprecatedSince{2014_10} This class is deprecated. Please use PlaneGeometry instead. */
  DEPRECATED(typedef PlaneGeometry Geometry2D);

  /**
  * \brief Describes a two-dimensional, rectangular plane
  *
  * \ingroup Geometry
  */
  class MITKCORE_EXPORT PlaneGeometry : public BaseGeometry
  {
  public:
    mitkClassMacro(PlaneGeometry, BaseGeometry);

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      enum PlaneOrientation {
        Axial,
        Sagittal,
        Frontal, // also known as "Coronal" in mitk.
        None     // This defines the PlaneGeometry for the 3D renderWindow which
        // curiously also needs a PlaneGeometry. This should be reconsidered some time.
      };

    virtual void IndexToWorld(const Point2D &pt_units, Point2D &pt_mm) const;

    virtual void WorldToIndex(const Point2D &pt_mm, Point2D &pt_units) const;

    //##Documentation
    //## @brief Convert (continuous or discrete) index coordinates of a \em vector
    //## \a vec_units to world coordinates (in mm)
    //## @deprecated First parameter (Point2D) is not used. If possible, please use void IndexToWorld(const
    // mitk::Vector2D& vec_units, mitk::Vector2D& vec_mm) const.
    //## For further information about coordinates types, please see the Geometry documentation
    virtual void IndexToWorld(const mitk::Point2D &atPt2d_untis,
                              const mitk::Vector2D &vec_units,
                              mitk::Vector2D &vec_mm) const;

    //##Documentation
    //## @brief Convert (continuous or discrete) index coordinates of a \em vector
    //## \a vec_units to world coordinates (in mm)
    //## For further information about coordinates types, please see the Geometry documentation
    virtual void IndexToWorld(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const;

    //##Documentation
    //## @brief Convert world coordinates (in mm) of a \em vector
    //## \a vec_mm to (continuous!) index coordinates.
    //## @deprecated First parameter (Point2D) is not used. If possible, please use void WorldToIndex(const
    // mitk::Vector2D& vec_mm, mitk::Vector2D& vec_units) const.
    //## For further information about coordinates types, please see the Geometry documentation
    virtual void WorldToIndex(const mitk::Point2D &atPt2d_mm,
                              const mitk::Vector2D &vec_mm,
                              mitk::Vector2D &vec_units) const;

    //##Documentation
    //## @brief Convert world coordinates (in mm) of a \em vector
    //## \a vec_mm to (continuous!) index coordinates.
    //## For further information about coordinates types, please see the Geometry documentation
    virtual void WorldToIndex(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const;

    /**
    * \brief Initialize a plane with orientation \a planeorientation
    * (default: axial) with respect to \a BaseGeometry (default: identity).
    * Spacing also taken from \a BaseGeometry.
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
    virtual void InitializeStandardPlane(const BaseGeometry *geometry3D,
                                         PlaneOrientation planeorientation = Axial,
                                         ScalarType zPosition = 0,
                                         bool frontside = true,
                                         bool rotated = false,
                                         bool top = true);

    /**
    * \brief Initialize a plane with orientation \a planeorientation
    * (default: axial) with respect to \a BaseGeometry (default: identity).
    * Spacing also taken from \a BaseGeometry.
    *
    * \param top if \a true, create plane at top, otherwise at bottom
    * (for PlaneOrientation Axial, for other plane locations respectively)
    */
    virtual void InitializeStandardPlane(const BaseGeometry *geometry3D,
                                         bool top,
                                         PlaneOrientation planeorientation = Axial,
                                         bool frontside = true,
                                         bool rotated = false);

    /**
    * \brief Initialize a plane with orientation \a planeorientation
    * (default: axial) with respect to \a transform (default: identity)
    * given width and height in units.
    *
    * \a Rotated means rotated by 180 degrees (1/2 rotation) within the plane.
    * Rotation by 90 degrees (1/4 rotation) is not implemented as of now.
    *
    * \a Frontside/Backside:
    * Viewed from below = frontside in the axial case;
    * (radiologist's view versus neuro-surgeon's view, see:
    * http://www.itk.org/Wiki/images/e/ed/DICOM-OrientationDiagram-Radiologist-vs-NeuroSurgeon.png )
    * Viewed from front = frontside in the coronal case;
    * Viewed from left = frontside in the sagittal case.
    *
    * \a Cave/Caution: Currently only RPI, LAI, LPS and RAS in the three standard planes are covered,
    * i.e. 12 cases of 144:  3 standard planes * 48 coordinate orientations = 144 cases.
    */
    virtual void InitializeStandardPlane(ScalarType width,
                                         ScalarType height,
                                         const AffineTransform3D *transform = nullptr,
                                         PlaneOrientation planeorientation = Axial,
                                         ScalarType zPosition = 0,
                                         bool frontside = true,
                                         bool rotated = false,
                                         bool top = true);

    /**
    * \brief Initialize plane with orientation \a planeorientation
    * (default: axial) given width, height and spacing.
    *
    */
    virtual void InitializeStandardPlane(ScalarType width,
                                         ScalarType height,
                                         const Vector3D &spacing,
                                         PlaneOrientation planeorientation = Axial,
                                         ScalarType zPosition = 0,
                                         bool frontside = true,
                                         bool rotated = false,
                                         bool top = true);

    /**
    * \brief Initialize plane by width and height in pixels, right-/down-vector
    * (itk) to describe orientation in world-space (vectors will be normalized)
    * and spacing (default: 1.0 mm in all directions).
    *
    * The vectors are normalized and multiplied by the respective spacing before
    * they are set in the matrix.
    *
    * This overloaded version of InitializeStandardPlane() creates only righthanded
    * coordinate orientations, unless spacing contains 1 or 3 negative entries.
    *
    */
    virtual void InitializeStandardPlane(ScalarType width,
                                         ScalarType height,
                                         const Vector3D &rightVector,
                                         const Vector3D &downVector,
                                         const Vector3D *spacing = nullptr);

    /**
    * \brief Initialize plane by width and height in pixels,
    * right-/down-vector (vnl) to describe orientation in world-space (vectors
    * will be normalized) and spacing (default: 1.0 mm in all directions).
    *
    * The vectors are normalized and multiplied by the respective spacing
    * before they are set in the matrix.
    *
    * This overloaded version of InitializeStandardPlane() creates only righthanded
    * coordinate orientations, unless spacing contains 1 or 3 negative entries.
    *
    */
    virtual void InitializeStandardPlane(ScalarType width,
                                         ScalarType height,
                                         const VnlVector &rightVector,
                                         const VnlVector &downVector,
                                         const Vector3D *spacing = nullptr);

    /**
    * \brief Initialize plane by right-/down-vector (itk) and spacing
    * (default: 1.0 mm in all directions).
    *
    * The length of the right-/-down-vector is used as width/height in units,
    * respectively. Then, the vectors are normalized and multiplied by the
    * respective spacing before they are set in the matrix.
    */
    virtual void InitializeStandardPlane(const Vector3D &rightVector,
                                         const Vector3D &downVector,
                                         const Vector3D *spacing = nullptr);

    /**
    * \brief Initialize plane by right-/down-vector (vnl) and spacing
    * (default: 1.0 mm in all directions).
    *
    * The length of the right-/-down-vector is used as width/height in units,
    * respectively. Then, the vectors are normalized and multiplied by the
    * respective spacing before they are set in the matrix.
    */
    virtual void InitializeStandardPlane(const VnlVector &rightVector,
                                         const VnlVector &downVector,
                                         const Vector3D *spacing = nullptr);

    /**
    * \brief Initialize plane by origin and normal (size is 1.0 mm in
    * all directions, direction of right-/down-vector valid but
    * undefined).
    * \warning This function can only produce righthanded coordinate orientation, not lefthanded.
    */
    virtual void InitializePlane(const Point3D &origin, const Vector3D &normal);

    /**
    * \brief Initialize plane by right-/down-vector.
    *
    * \warning The vectors are set into the matrix as they are,
    * \em without normalization!
    * This function creates a righthanded IndexToWorldTransform,
    * only a negative thickness could still make it lefthanded.
    */
    void SetMatrixByVectors(const VnlVector &rightVector, const VnlVector &downVector, ScalarType thickness = 1.0);

    /**
    * \brief Check if matrix is a rotation matrix:
    * - determinant is 1?
    * - R*R^T is ID?
    * Output warning otherwise.
    */
    static bool CheckRotationMatrix(AffineTransform3D *transform, double epsilon=mitk::eps);

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

    virtual ScalarType SignedDistance(const Point3D &pt3d_mm) const;

    /**
    * \brief Calculates, whether a point is below or above the plane. There are two different
    *calculation methods, with or without consideration of the bounding box.
    */
    virtual bool IsAbove(const Point3D &pt3d_mm, bool considerBoundingBox = false) const;

    /**
    * \brief Distance of the point from the plane
    * (bounding-box \em not considered)
    *
    */
    ScalarType DistanceFromPlane(const Point3D &pt3d_mm) const;

    /**
    * \brief Signed distance of the point from the plane
    * (bounding-box \em not considered)
    *
    * > 0 : point is in the direction of the direction vector.
    */
    inline ScalarType SignedDistanceFromPlane(const Point3D &pt3d_mm) const
    {
      ScalarType len = GetNormalVnl().two_norm();

      if (len == 0)
        return 0;

      return (pt3d_mm - GetOrigin()) * GetNormal() / len;
    }

    /**
    * \brief Distance of the plane from another plane
    * (bounding-box \em not considered)
    *
    * Result is 0 if planes are not parallel.
    */
    ScalarType DistanceFromPlane(const PlaneGeometry *plane) const { return fabs(SignedDistanceFromPlane(plane)); }
    /**
    * \brief Signed distance of the plane from another plane
    * (bounding-box \em not considered)
    *
    * Result is 0 if planes are not parallel.
    */
    inline ScalarType SignedDistanceFromPlane(const PlaneGeometry *plane) const
    {
      if (IsParallel(plane))
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
    bool IntersectionLine(const PlaneGeometry *plane, Line3D &crossline) const;

    /**
    * \brief Calculate two points where another plane intersects the border of this plane
    *
    * \return number of intersection points (0..2). First interection point (if existing)
    * is returned in \a lineFrom, second in \a lineTo.
    */
    unsigned int IntersectWithPlane2D(const PlaneGeometry *plane, Point2D &lineFrom, Point2D &lineTo) const;

    /**
    * \brief Calculate the angle between two planes
    *
    * \return angle in radiants
    */
    double Angle(const PlaneGeometry *plane) const;

    /**
    * \brief Calculate the angle between the plane and a line
    *
    * \return angle in radiants
    */
    double Angle(const Line3D &line) const;

    /**
    * \brief Calculate intersection point between the plane and a line
    *
    * \param intersectionPoint intersection point
    * \return \a true if \em unique intersection exists, i.e., if line
    * is \em not on or parallel to the plane
    */
    bool IntersectionPoint(const Line3D &line, Point3D &intersectionPoint) const;

    /**
    * \brief Calculate line parameter of intersection point between the
    * plane and a line
    *
    * \param t parameter of line: intersection point is
    * line.GetPoint()+t*line.GetDirection()
    * \return \a true if \em unique intersection exists, i.e., if line
    * is \em not on or parallel to the plane
    */
    bool IntersectionPointParam(const Line3D &line, double &t) const;

    /**
    * \brief Returns whether the plane is parallel to another plane
    *
    * @return true iff the normal vectors both point to the same or exactly oposit direction
    */
    bool IsParallel(const PlaneGeometry *plane) const;

    /**
    * \brief Returns whether the point is on the plane
    * (bounding-box \em not considered)
    */
    bool IsOnPlane(const Point3D &point) const;

    /**
    * \brief Returns whether the line is on the plane
    * (bounding-box \em not considered)
    */
    bool IsOnPlane(const Line3D &line) const;

    /**
    * \brief Returns whether the plane is on the plane
    * (bounding-box \em not considered)
    *
    * @return true iff the normal vector of the planes point to the same or the exactly oposit direction and
    *  the distance of the planes is < eps
    *
    */
    bool IsOnPlane(const PlaneGeometry *plane) const;

    /**
    * \brief Returns the lot from the point to the plane
    */
    Point3D ProjectPointOntoPlane(const Point3D &pt) const;

    itk::LightObject::Pointer InternalClone() const override;

    /** Implements operation to re-orient the plane */
    void ExecuteOperation(Operation *operation) override;

    /**
    * \brief Project a 3D point given in mm (\a pt3d_mm) onto the 2D
    * geometry. The result is a 2D point in mm (\a pt2d_mm).
    *
    * The result is a 2D point in mm (\a pt2d_mm) relative to the upper-left
    * corner of the geometry. To convert this point into units (e.g., pixels
    * in case of an image), use WorldToIndex.
    * \return true projection was possible
    * \sa Project(const mitk::Point3D &pt3d_mm, mitk::Point3D
    * &projectedPt3d_mm)
    */
    virtual bool Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const;

    /**
    * \brief Converts a 2D point given in mm (\a pt2d_mm) relative to the
    * upper-left corner of the geometry into the corresponding
    * world-coordinate (a 3D point in mm, \a pt3d_mm).
    *
    * To convert a 2D point given in units (e.g., pixels in case of an
    * image) into a 2D point given in mm (as required by this method), use
    * IndexToWorld.
    */
    virtual void Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const;

    /**
    * \brief Set the width and height of this 2D-geometry in units by calling
    * SetBounds. This does \a not change the extent in mm!
    *
    * For an image, this is the number of pixels in x-/y-direction.
    * \note In contrast to calling SetBounds directly, this does \a not change
    * the extent in mm!
    */
    virtual void SetSizeInUnits(mitk::ScalarType width, mitk::ScalarType height);

    /**
    * \brief Project a 3D point given in mm (\a pt3d_mm) onto the 2D
    * geometry. The result is a 3D point in mm (\a projectedPt3d_mm).
    *
    * \return true projection was possible
    */
    virtual bool Project(const mitk::Point3D &pt3d_mm, mitk::Point3D &projectedPt3d_mm) const;

    /**
    * \brief Project a 3D vector given in mm (\a vec3d_mm) onto the 2D
    * geometry. The result is a 2D vector in mm (\a vec2d_mm).
    *
    * The result is a 2D vector in mm (\a vec2d_mm) relative to the
    * upper-left
    * corner of the geometry. To convert this point into units (e.g., pixels
    * in case of an image), use WorldToIndex.
    * \return true projection was possible
    * \sa Project(const mitk::Vector3D &vec3d_mm, mitk::Vector3D
    * &projectedVec3d_mm)
    */
    virtual bool Map(const mitk::Point3D &atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const;

    /**
    * \brief Converts a 2D vector given in mm (\a vec2d_mm) relative to the
    * upper-left corner of the geometry into the corresponding
    * world-coordinate (a 3D vector in mm, \a vec3d_mm).
    *
    * To convert a 2D vector given in units (e.g., pixels in case of an
    * image) into a 2D vector given in mm (as required by this method), use
    * IndexToWorld.
    */
    virtual void Map(const mitk::Point2D &atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const;

    /**
    * \brief Project a 3D vector given in mm (\a vec3d_mm) onto the 2D
    * geometry. The result is a 3D vector in mm (\a projectedVec3d_mm).
    *
    * DEPRECATED. Use Project(vector,vector) instead
    *
    * \return true projection was possible
    */
    virtual bool Project(const mitk::Point3D &atPt3d_mm,
                         const mitk::Vector3D &vec3d_mm,
                         mitk::Vector3D &projectedVec3d_mm) const;

    /**
    * \brief Project a 3D vector given in mm (\a vec3d_mm) onto the 2D
    * geometry. The result is a 3D vector in mm (\a projectedVec3d_mm).
    *
    * \return true projection was possible
    */
    virtual bool Project(const mitk::Vector3D &vec3d_mm, mitk::Vector3D &projectedVec3d_mm) const;

    /**
    * \brief Distance of the point from the geometry
    * (bounding-box \em not considered)
    *
    */
    inline ScalarType Distance(const Point3D &pt3d_mm) const { return fabs(SignedDistance(pt3d_mm)); }
    /**
    * \brief Set the geometrical frame of reference in which this PlaneGeometry
    * is placed.
    *
    * This would usually be the BaseGeometry of the underlying dataset, but
    * setting it is optional.
    */
    void SetReferenceGeometry(const mitk::BaseGeometry *geometry);

    /**
    * \brief Get the geometrical frame of reference for this PlaneGeometry.
    */
    const BaseGeometry *GetReferenceGeometry() const;
    bool HasReferenceGeometry() const;

    static std::vector< int > CalculateDominantAxes(mitk::AffineTransform3D::MatrixType::InternalMatrixType& rotation_matrix);

  protected:
    PlaneGeometry();

    PlaneGeometry(const PlaneGeometry &other);

    ~PlaneGeometry() override;

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    const mitk::BaseGeometry *m_ReferenceGeometry;

    //##Documentation
    //## @brief PreSetSpacing
    //##
    //## These virtual function allows a different beahiour in subclasses.
    //## Do implement them in every subclass of BaseGeometry. If not needed, use
    //## {Superclass::PreSetSpacing();};
    void PreSetSpacing(const mitk::Vector3D &aSpacing) override { Superclass::PreSetSpacing(aSpacing); };
    //##Documentation
    //## @brief CheckBounds
    //##
    //## This function is called in SetBounds. Assertions can be implemented in this function (see PlaneGeometry.cpp).
    //## If you implement this function in a subclass, make sure, that all classes were your class inherits from
    //## have an implementation of CheckBounds
    //## (e.g. inheritance BaseGeometry <- A <- B. Implementation of CheckBounds in class B needs implementation in A as
    // well!)
    void CheckBounds(const BoundsArrayType &bounds) override;

    //##Documentation
    //## @brief CheckIndexToWorldTransform
    //##
    //## This function is called in SetIndexToWorldTransform. Assertions can be implemented in this function (see
    // PlaneGeometry.cpp).
    //## In Subclasses of BaseGeometry, implement own conditions or call Superclass::CheckBounds(bounds);.
    void CheckIndexToWorldTransform(mitk::AffineTransform3D *transform) override;

  private:
    /**
    * \brief Compares plane with another plane: \a true if IsOnPlane
    * (bounding-box \em not considered)
    */
    virtual bool operator==(const PlaneGeometry *) const { return false; };
    /**
    * \brief Compares plane with another plane: \a false if IsOnPlane
    * (bounding-box \em not considered)
    */
    virtual bool operator!=(const PlaneGeometry *) const { return false; };
  };
} // namespace mitk

#endif /* PLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
