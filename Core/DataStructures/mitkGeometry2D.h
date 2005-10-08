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


#ifndef GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0
#define GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0

#include "mitkCommon.h"
#include "mitkGeometry3D.h"

namespace mitk {

//##ModelId=3DDE4DEC0291
//##Documentation
//## @brief Describes the geometry of a two-dimensional object
//##
//## Describes a two-dimensional manifold, i.e., simply speaken, 
//## an object that can be described using a 2D coordinate-system. 
//##
//## Geometry2D can map points between 3D world coordinates 
//## (in mm) and the described 2D coordinate-system (in mm) by first projecting
//## the 3D point onto the 2D manifold and then calculating the 2D-coordinates
//## (in mm). These 2D-mm-coordinates can be further converted into 
//## 2D-unit-coordinates (e.g., pixels), giving a parameter representation of the 
//## object with parameter values inside a rectangle (e.g., [0,0]..[width, height]),
//## which is the bounding box (bounding range in z-direction always [0]..[1]).
//##
//## A Geometry2D describes the 2D representation within a 3D object and is 
//## therefore itself a Geometry3D (derived from Geometry3D). For example,
//## a single CT-image (slice) is 2D in the sense that you can access the 
//## pixels using 2D-coordinates, but is also 3D, as the pixels are really 
//## voxels, thus have an extension (thickness) in the 3rd dimension.
//## 
//## Most often, instances of Geometry2D will be used to descibe a plane, 
//## which is represented by the sub-class PlaneGeometry, but curved 
//## surfaces are also possible.
//## 
//## @warning The Geometry2Ds are not necessarily up-to-date and not even
//## initialized. As described in the previous paragraph, one of the
//## Generate-/Copy-/UpdateOutputInformation methods have to initialize it.
//## mitk::BaseData::GetGeometry2D() makes sure, that the Geometry2D is
//## up-to-date before returning it (by setting the update extent appropriately
//## and calling UpdateOutputInformation).
//## 
//## Rule: everything is in mm (or ms for temporal information) if not 
//## stated otherwise.
//## @ingroup Geometry
class Geometry2D : public mitk::Geometry3D
{
public:
  mitkClassMacro(Geometry2D, mitk::Geometry3D);
  itkNewMacro(Self);

  //##ModelId=3DDE65E00122
  //##Documentation
  //## @brief Project a 3D point given in mm (@a pt3d_mm) onto the 2D
  //## geometry. The result is a 2D point in mm (@a pt2d_mm).
  //## 
  //## The result is a 2D point in mm (@a pt2d_mm) relative to the upper-left
  //## corner of the geometry. To convert this point into units (e.g., pixels
  //## in case of an image), use WorldToIndex.
  //## @return true projection was possible
  //## @sa Project(const mitk::Point3D &pt3d_mm, mitk::Point3D
  //## &projectedPt3d_mm) 
  virtual bool Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const;

  //##ModelId=3DDE65E301DE
  //##Documentation
  //## @brief Converts a 2D point given in mm (@a pt2d_mm) relative to the
  //## upper-left corner of the geometry into the corresponding
  //## world-coordinate (a 3D point in mm, @a pt3d_mm). 
  //## 
  //## To convert a 2D point given in units (e.g., pixels in case of an
  //## image) into a 2D point given in mm (as required by this method), use
  //## IndexToWorld.
  virtual void Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const;

  //##ModelId=3DE7895602F7
  //##Documentation
  //## @brief Convert a 2D point given in units (e.g., pixels in case of an
  //## image) into a 2D point given in mm
  virtual void IndexToWorld(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const;

  //##ModelId=3DE7895C01CE
  //##Documentation
  //## @brief Convert a 2D point given in mm into a 2D point given in mm
  //## (e.g., pixels in case of an image)
  virtual void WorldToIndex(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const;

  //##ModelId=3E3B98C5019F
  //##Documentation
  //## @brief Convert a 2D vector given in units (e.g., pixels in case of an
  //## image) into a 2D vector given in mm
  //## @warning strange: in contrast to vtkTransform the class itk::Transform
  //## does not have the parameter, @em where the vector that is to be
  //## transformed is located. This method here should also need this
  //## information for general transforms.
  virtual void IndexToWorld(const mitk::Point2D &atPt2d_units, const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const;
  //##ModelId=3E3B98C9019B
  //##Documentation
  //## @brief Convert a 2D vector given in mm into a 2D point vector in mm
  //## (e.g., pixels in case of an image)
  //## @warning strange: in contrast to vtkTransform the class itk::Transform
  //## does not have the parameter, @em where the vector that is to be
  //## transformed is located. This method here should also need this
  //## information for general transforms.
  virtual void WorldToIndex(const mitk::Point2D &atPt2d_mm, const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const;

  //##ModelId=3E3C3A92033E
  //##Documentation
  //## @brief Set the width and height of this 2D-geometry in units by calling SetBounds.
  //## This does @a not change the extent in mm!
  //##
  //## For an image, this is the number of pixels in x-/y-direction.
  //## @note In contrast to calling SetBounds directly, this does @a not change the extent in mm!
  virtual void SetSizeInUnits(mitk::ScalarType width, mitk::ScalarType height);

  //##ModelId=3EF48EA10320
  //##Documentation
  //## @brief Project a 3D point given in mm (@a pt3d_mm) onto the 2D
  //## geometry. The result is a 3D point in mm (@a projectedPt3d_mm).
  //## 
  //## @return true projection was possible
  virtual bool Project(const mitk::Point3D &pt3d_mm, mitk::Point3D &projectedPt3d_mm) const;

  //##ModelId=3EF48F170280
  //##Documentation
  //## @brief Project a 3D vector given in mm (@a vec3d_mm) onto the 2D
  //## geometry. The result is a 2D vector in mm (@a vec2d_mm).
  //## 
  //## The result is a 2D vector in mm (@a vec2d_mm) relative to the
  //## upper-left
  //## corner of the geometry. To convert this point into units (e.g., pixels
  //## in case of an image), use WorldToIndex.
  //## @return true projection was possible
  //## @sa Project(const mitk::Vector3D &vec3d_mm, mitk::Vector3D
  //## &projectedVec3d_mm) 
  virtual bool Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const;

  //##ModelId=3EF48F2E00D4
  //##Documentation
  //## @brief Converts a 2D vector given in mm (@a vec2d_mm) relative to the
  //## upper-left corner of the geometry into the corresponding
  //## world-coordinate (a 3D vector in mm, @a vec3d_mm). 
  //## 
  //## To convert a 2D vector given in units (e.g., pixels in case of an
  //## image) into a 2D vector given in mm (as required by this method), use
  //## IndexToWorld.
  virtual void Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const;

  //##ModelId=3EF48F8F01B0
  //##Documentation
  //## @brief Project a 3D vector given in mm (@a vec3d_mm) onto the 2D
  //## geometry. The result is a 3D vector in mm (@a projectedVec3d_mm).
  //## 
  //## @return true projection was possible
  virtual bool Project(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector3D &projectedVec3d_mm) const;

  //##Documentation
  //## @brief Distance of the point from the geometry
  //## (bounding-box @em not considered)
  //##
  inline ScalarType Distance(const Point3D& pt3d_mm) const
  {
    return fabs(SignedDistance(pt3d_mm));
  }

  //##Documentation
  //## @brief Signed distance of the point from the geometry
  //## (bounding-box @em not considered)
  //##
  virtual ScalarType SignedDistance(const Point3D& pt3d_mm) const;

  //##Documentation
  //## @brief Test if the point is above the geometry
  //## (bounding-box @em not considered)
  //##
  virtual bool IsAbove(const Point3D& pt3d_mm) const;

  virtual void SetIndexToWorldTransform(mitk::AffineTransform3D* transform);

  virtual void SetExtentInMM(int direction, ScalarType extentInMM);

  virtual AffineGeometryFrame3D::Pointer Clone() const;

  virtual bool ReadXMLData( XMLReader& xmlReader );

protected:
  //##ModelId=3E395E0802E6
  Geometry2D();
  //##ModelId=3E395E080318
  virtual ~Geometry2D();

	virtual void InitializeGeometry(Self * newGeometry) const;

  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  virtual bool WriteXMLData( XMLWriter& xmlWriter );

  static std::string SCALAR_FACTOR_MM_PRT_UNIT_X;
  static std::string SCALAR_FACTOR_MM_PRT_UNIT_Y;

  //##Documentation
  //## @brief factor to convert x-coordinates from mm to units and vice versa
  //## 
  mutable mitk::ScalarType m_ScaleFactorMMPerUnitX;
  //##Documentation
  //## @brief factor to convert y-coordinates from mm to units and vice versa
  //## 
  mutable mitk::ScalarType m_ScaleFactorMMPerUnitY;
};

} // namespace mitk

#endif /* GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0 */
