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

#ifndef mitkDisplayGeometry_h
#define mitkDisplayGeometry_h

#include "mitkPlaneGeometry.h"

namespace mitk
{
  /**
 \brief Describes the geometry on the display/screen for 2D display.

 The main purpose of this class is to convert between display coordinates
 (in display-units) and world coordinates (in mm).
 DisplayGeometry depends on the size of the display area (widget width and
  height, m_SizeInDisplayUnits) and on a PlaneGeometry (m_WoldGeometry). It
 represents a recangular view on this world-geometry. E.g., you can tell
 the DisplayGeometry to fit the world-geometry in the display area by
 calling Fit(). Provides methods for zooming and panning.

 Zooming and panning can be restricted within reasonable bounds by setting
 the ConstrainZoomingAndPanning flag. In these cases you can re-define what
 bounds you accept as "reasonable" by calling

 \warning \em Units refers to the units of the underlying world-geometry.
 Take care, whether these are really the units you want to convert to.
 E.g., when you want to convert a point \a pt_display (which is 2D) given
 in display coordinates into a point in units of a BaseData-object @a datum
 (the requested point is 3D!), use

 \code
 displaygeometry->DisplayToWorld(pt_display, pt2d_mm);
 displaygeometry->Map(pt2d_mm, pt3d_mm);
 datum->GetGeometry()->WorldToIndex(pt3d_mm, pt3d_datum_units);
 \endcode

 Even, if you want to convert the 2D point \a pt_display into a 2D point in
 units on a certain 2D geometry \a certaingeometry, it is safer to use

 \code
 displaygeometry->DisplayToWorld(pt_display, pt_mm);
 certaingeometry->WorldToIndex(pt_mm, pt_certain_geometry_units);
 \endcode

 unless you can be sure that the underlying geometry of \a displaygeometry
 is really the \a certaingeometry.

 \ingroup Geometry
  */
  class MITK_CORE_EXPORT DisplayGeometry : public PlaneGeometry
  {
  public:

    mitkClassMacro(DisplayGeometry,PlaneGeometry);

    /// Method for creation through the object factory.
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /// \brief duplicates the geometry, NOT useful for this sub-class
    virtual itk::LightObject::Pointer InternalClone() const;

    /// \return this objects modified time.
    virtual unsigned long GetMTime() const;

    //virtual const TimeBounds& GetTimeBounds() const;

    // size definition methods

    virtual void SetWorldGeometry(const PlaneGeometry* aWorldGeometry);
    itkGetConstObjectMacro(WorldGeometry, PlaneGeometry);

    /// \return if new origin was within accepted limits
    virtual bool SetOriginInMM(const Vector2D& origin_mm);

    virtual Vector2D GetOriginInMM() const;
    virtual Vector2D GetOriginInDisplayUnits() const;

    /**
     \brief Set the size of the display in display units.

     This method must be called every time the display is resized (normally, the GUI-toolkit
     informs about resizing).
     \param keepDisplayedRegion: if \a true (the default), the displayed contents is zoomed/shrinked
     so that the displayed region is (approximately) the same as before: The point at the center will
     be kept at the center and the length of the diagonal of the displayed region \em in \em units
     will also be kept.
     When the aspect ration changes, the displayed region includes the old displayed region, but
     cannot be exaclty the same.
     */
    virtual void SetSizeInDisplayUnits(unsigned int width, unsigned int height, bool keepDisplayedRegion=true);
    virtual Vector2D GetSizeInDisplayUnits() const;
    virtual Vector2D GetSizeInMM() const;
    unsigned int GetDisplayWidth() const;
    unsigned int GetDisplayHeight() const;

    // zooming, panning, restriction of both

    virtual void SetConstrainZoomingAndPanning(bool constrain);
    virtual bool GetConstrainZommingAndPanning() const;

    /// what percentage of the world should be visible at maximum zoom out (default 1.0, i.e. 100% of width or height)
    itkGetMacro(MaxWorldViewPercentage, float);
    itkSetMacro(MaxWorldViewPercentage, float);

    /// what percentage of the world should be visible at maximum zoom in (default 0.1, i.e. 10% of width or height)
    itkGetMacro(MinWorldViewPercentage, float);
    itkSetMacro(MinWorldViewPercentage, float);

    virtual bool SetScaleFactor(ScalarType mmPerDisplayUnit);
    ScalarType GetScaleFactorMMPerDisplayUnit() const;

    /**
    * \brief Zooms with a factor (1.0=identity) to/from the specified center in display units
    * \return true if zoom request was within accepted limits
    */
    virtual bool Zoom(ScalarType factor, const Point2D& centerInDisplayUnits);

    /**
    * \brief Zooms with a factor (1.0=identity) to/from the specified center, trying to preserve the center of zoom in world coordiantes
    *
    * Same zoom as mentioned above but tries (if it's within view contraints) to match the center in display units with the center in world coordinates.
    *
    * \return true if zoom request was within accepted limits
    */
    virtual bool ZoomWithFixedWorldCoordinates(ScalarType factor, const Point2D& focusDisplayUnits, const Point2D& focusUnitsInMM );

    // \return true if move request was within accepted limits
    virtual bool MoveBy(const Vector2D& shiftInDisplayUnits);

    // \brief align display with world, make world completely visible
    virtual void Fit();

    // conversion methods

    virtual void DisplayToWorld(const Point2D &pt_display, Point2D &pt_mm) const;
    virtual void WorldToDisplay(const Point2D &pt_mm, Point2D &pt_display) const;

    virtual void DisplayToWorld(const Vector2D &vec_display, Vector2D &vec_mm) const;
    virtual void WorldToDisplay(const Vector2D &vec_mm, Vector2D &vec_display) const;

    virtual void ULDisplayToMM(const Point2D &pt_ULdisplay, Point2D &pt_mm) const;
    virtual void MMToULDisplay(const Point2D &pt_mm, Point2D &pt_ULdisplay) const;

    virtual void ULDisplayToMM(const Vector2D &vec_ULdisplay, Vector2D &vec_mm) const;
    virtual void MMToULDisplay(const Vector2D &vec_mm, Vector2D &vec_ULdisplay) const;

    virtual void ULDisplayToDisplay(const Point2D &pt_ULdisplay, Point2D &pt_display) const;
    virtual void DisplayToULDisplay(const Point2D &pt_display, Point2D &pt_ULdisplay) const;

    virtual void ULDisplayToDisplay(const Vector2D &vec_ULdisplay, Vector2D &vec_display) const;
    virtual void DisplayToULDisplay(const Vector2D &vec_display, Vector2D &vec_ULdisplay) const;

    /**
    * \brief projects the given point onto current 2D world geometry plane
    */
    virtual bool Project(const Point3D &pt3d_mm, Point3D &projectedPt3d_mm) const;

    /**
    * \brief projects the given vector onto current 2D world geometry plane.
    * \warning DEPRECATED, please use Project(const Vector3D &vec3d_mm, Vector3D &projectedVec3d_mm) instead
    */
    virtual bool Project(const Point3D & atPt3d_mm, const Vector3D &vec3d_mm, Vector3D &projectedVec3d_mm) const;

    /**
    * \brief projects the given vector onto current 2D world geometry plane
    */
    virtual bool Project(const Vector3D &vec3d_mm, Vector3D &projectedVec3d_mm) const;

    virtual bool Map(const Point3D &pt3d_mm, Point2D &pt2d_mm) const;
    virtual void Map(const Point2D &pt2d_mm, Point3D &pt3d_mm) const;
    virtual bool Map(const Point3D & atPt3d_mm, const Vector3D &vec3d_mm, Vector2D &vec2d_mm) const;
    virtual void Map(const Point2D & atPt2d_mm, const Vector2D &vec2d_mm, Vector3D &vec3d_mm) const;

    virtual bool IsValid() const;

    virtual bool IsAbove( const Point3D &pt3d_mm , bool /*considerBoundingBox=false*/) const { return Superclass::IsAbove(pt3d_mm, true);};

  protected:

    DisplayGeometry();
    virtual ~DisplayGeometry();

    /**
      \brief Called after zooming/panning to restrict these operations to sensible measures.
      \return true if a correction in either zooming or panning was made

      Enforces a couple of constraints on the relation of the current viewport and the current world geometry.

      The basic logic in this lengthy method is:
      <ol>
      <li> Make display region big enough (in case of too large zoom factors)
      <li> Make display region small enough (so that the image cannot be scaled into a single screen pixel
      <li> Correct panning for each border (left, right, bottom, top)
      </ol>

      The little more complicated implementation is illustrated in the code itself.
     */
    virtual bool RefitVisibleRect();

    virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

    Vector2D m_OriginInMM;
    Vector2D m_OriginInDisplayUnits;
    ScalarType m_ScaleFactorMMPerDisplayUnit;
    Vector2D m_SizeInMM;
    Vector2D m_SizeInDisplayUnits;
    PlaneGeometry::ConstPointer m_WorldGeometry;

    bool m_ConstrainZoomingAndPanning;
    float m_MaxWorldViewPercentage;
    float m_MinWorldViewPercentage;
  };
} // namespace

#endif // include guard
