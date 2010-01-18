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


#ifndef DISPLAYGEOMETRY_H_HEADER_INCLUDED_C1B77F88
#define DISPLAYGEOMETRY_H_HEADER_INCLUDED_C1B77F88

#include "mitkCommon.h"
#include "mitkGeometry2D.h"
#include "mitkImageSliceSelector.h"

namespace mitk {

//##Documentation
//## @brief Describes the geometry on the display/screen for 2D display.
//##
//## The main purpose of this class is to convert between display coordinates
//## (in display-units) and world coordinates (in mm).
//## DisplayGeometry depends on the size of the display area (widget width and
//## height, m_SizeInDisplayUnits) and on a Geometry2D (m_WoldGeometry). It
//## represents a recangular view on this world-geometry. E.g., you can tell
//## the DisplayGeometry to fit the world-geometry in the display area by
//## calling Fit(). Provides methods for zooming and panning.
//## @warning @em Units refers to the units of the underlying world-geometry.
//## Take care, whether these are really the units you want to convert to.
//## E.g., when you want to convert a point @a pt_display (which is 2D) given
//## in display coordinates into a point in units of a BaseData-object @a datum
//## (the requested point is 3D!), use
//## @code
//## displaygeometry->DisplayToWorld(pt_display, pt2d_mm);
//## displaygeometry->Map(pt2d_mm, pt3d_mm);
//## datum->GetGeometry()->WorldToIndex(pt3d_mm, pt3d_datum_units);
//## @endcode
//## Even, if you want to convert the 2D point @a pt_display into a 2D point in
//## units on a certain 2D geometry @a certaingeometry, it is safer to use
//## @code
//## displaygeometry->DisplayToWorld(pt_display, pt_mm);
//## certaingeometry->WorldToIndex(pt_mm, pt_certain_geometry_units);
//## @endcode
//## unless you can be sure that the underlying geometry of @a displaygeometry
//## is really the @a certaingeometry.
//## @ingroup Geometry
class MITK_CORE_EXPORT DisplayGeometry : public Geometry2D
{
public:
  mitkClassMacro(DisplayGeometry,Geometry2D);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  virtual void SetWorldGeometry(const mitk::Geometry2D* aWorldGeometry);
  itkGetConstObjectMacro(WorldGeometry, Geometry2D);

  virtual void Zoom(mitk::ScalarType factor, const mitk::Point2D& centerInDisplayUnits);
  virtual void MoveBy(const mitk::Vector2D& shiftInDisplayUnits);
  virtual void Fit();

  //##Documentation
  //## @brief Set the size of the display in display units.
  //## 
  //## This method must be called every time the display is resized (normally, the GUI-toolkit 
  //## informs about resizing).
  //## @param keepDisplayedRegion: if @a true (the default), the displayed contents is zoomed/shrinked
  //## so that the displayed region is (approximately) the same as before: The point at the center will 
  //## be kept at the center and the length of the diagonal of the displayed region @em in @em units
  //## will also be kept.
  //## When the aspect ration changes, the displayed region includes the old displayed region, but
  //## cannot be exaclty the same.
  virtual void SetSizeInDisplayUnits(unsigned int width, unsigned int height, bool keepDisplayedRegion=true);
  virtual void SetOriginInMM(const mitk::Vector2D& origin_mm);

  mitk::Vector2D GetOriginInMM() const
  {
    return m_OriginInMM;
  }

  mitk::Vector2D GetOriginInDisplayUnits() const
  {
    return m_OriginInDisplayUnits;
  }

  virtual bool Project(const mitk::Point3D &pt3d_mm, mitk::Point3D &projectedPt3d_mm) const;
  virtual bool Project(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector3D &projectedVec3d_mm) const;
  virtual bool Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const;
  virtual void Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const;

//   virtual void IndexToWorld(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const;
//   virtual void WorldToIndex(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const;

  virtual const TimeBounds& GetTimeBounds() const;

//   virtual void WorldToIndex(const mitk::Point2D &atPt2d_mm, const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const;
//   virtual void IndexToWorld(const mitk::Point2D &atPt2d_units, const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const;

  unsigned int GetDisplayWidth() const
  {
    assert(m_SizeInDisplayUnits[0] >= 0);  
    return (unsigned int)m_SizeInDisplayUnits[0];
  }

  unsigned int GetDisplayHeight() const
  {
    assert(m_SizeInDisplayUnits[1] >= 0);  
    return (unsigned int)m_SizeInDisplayUnits[1];
  }

  mitk::Vector2D GetSizeInDisplayUnits() const
  {
    return m_SizeInDisplayUnits;
  }

  mitk::Vector2D GetSizeInMM() const
  {
    return m_SizeInMM;
  }

  virtual void SetScaleFactor(mitk::ScalarType mmPerDisplayUnit);
  mitk::ScalarType GetScaleFactorMMPerDisplayUnit() const;

  virtual void DisplayToWorld(const mitk::Point2D &pt_display, mitk::Point2D &pt_mm) const;
  virtual void WorldToDisplay(const mitk::Point2D &pt_mm, mitk::Point2D &pt_display) const;

  virtual void DisplayToWorld(const mitk::Vector2D &vec_display, mitk::Vector2D &vec_mm) const;
  virtual void WorldToDisplay(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_display) const;

  virtual void ULDisplayToMM(const mitk::Point2D &pt_ULdisplay, mitk::Point2D &pt_mm) const;
  virtual void MMToULDisplay(const mitk::Point2D &pt_mm, mitk::Point2D &pt_ULdisplay) const;

  virtual void ULDisplayToMM(const mitk::Vector2D &vec_ULdisplay, mitk::Vector2D &vec_mm) const;
  virtual void MMToULDisplay(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_ULdisplay) const;

  virtual void ULDisplayToDisplay(const mitk::Point2D &pt_ULdisplay, mitk::Point2D &pt_display) const;
  virtual void DisplayToULDisplay(const mitk::Point2D &pt_display, mitk::Point2D &pt_ULdisplay) const;

  virtual void ULDisplayToDisplay(const mitk::Vector2D &vec_ULdisplay, mitk::Vector2D &vec_display) const;
  virtual void DisplayToULDisplay(const mitk::Vector2D &vec_display, mitk::Vector2D &vec_ULdisplay) const;

  /** Return this objects modified time.  */
  virtual unsigned long GetMTime() const;
  virtual bool Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const;

  virtual void Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const;

  virtual bool IsValid() const;

  //##Documentation
  //## @brief duplicates the geometry, NOT useful for this sub-class
  virtual AffineGeometryFrame3D::Pointer Clone() const;
protected:
  DisplayGeometry();
  virtual ~DisplayGeometry();

  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  mitk::Vector2D m_OriginInMM;
  mitk::Vector2D m_OriginInDisplayUnits;
  mitk::ScalarType m_ScaleFactorMMPerDisplayUnit;
  mitk::Vector2D m_SizeInMM;
  mitk::Vector2D m_SizeInDisplayUnits;
  mitk::Geometry2D::ConstPointer m_WorldGeometry;
};

} // namespace mitk

#endif /* DISPLAYGEOMETRY_H_HEADER_INCLUDED_C1B77F88 */
