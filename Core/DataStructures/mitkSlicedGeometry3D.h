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


#ifndef MITKSLICEDGEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD
#define MITKSLICEDGEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD

#include "mitkGeometry3D.h"
#include "mitkGeometry2D.h"

namespace mitk {

//##ModelId=3DCBF389032B
//##Documentation
//## @brief Describes the geometry of a data object consisting of slices
//## @ingroup Geometry
//## A Geometry2D can be requested for each slice.
//## In the case of @em evenly-spaced, @em plane geometries (m_EvenlySpaced==true), 
//## only the 2D-geometry of the first slice has to be set (to an instance of 
//## PlaneGeometry). The 2D-geometries of the other slices are calculated 
//## by shifting the first slice in the direction m_DirectionVector by 
//## m_Spacing.z*s (s: slice-number). The m_Spacing member (which is only 
//## relevant in the case m_EvenlySpaced==true) descibes the size of a voxel (in mm),
//## e.g., m_Spacing.x is the voxel width in the x-direction of the plane, which
//## is defined by PlaneGeometry::m_PlaneView::orientation1 (and therefore is in 
//## general @em not the x-direction of the world-coordinate system).
//## 
//## SlicedGeometry3D and the associated Geometry2Ds have to be
//## initialized in the method GenerateOutputInformation() of BaseProcess (or
//## CopyInformation/ UpdateOutputInformation of BaseData, if possible, e.g.,
//## by analyzing pic tags in Image) subclasses. See also
//## itk::ProcessObject::GenerateOutputInformation(),
//## itk::DataObject::CopyInformation() and
//## itk::DataObject::UpdateOutputInformation().
//## 
//## Rule: everything is in mm (or ms for temporal information) if not 
//## stated otherwise.
class SlicedGeometry3D : public mitk::Geometry3D
{
public:
  mitkClassMacro(SlicedGeometry3D, Geometry3D);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  //##ModelId=3DCBF50C0377
  //##Documentation
  //## Return the Geometry2D of the slice (@a s).
  //## If (a) m_EvenlySpaced==true, (b) we don't have a Geometry2D stored
  //## for the requested slice, and (c) the first slice (s=0) 
  //## is a PlaneGeometry instance, then we calculate the geometry of the
  //## requested as the plane of the first slice shifted by m_Spacing.z*s
  //## in the direction of m_DirectionVector.
  //##
  //## @warning The Geometry2Ds are not necessarily up-to-date and not even
  //## initialized. Geometry2Ds (if applicable) have to be initialized in the
  //## method GenerateOutputInformation() of BaseProcess (or CopyInformation/
  //## UpdateOutputInformation of BaseData, if possible, e.g., by analyzing
  //## pic tags in Image) subclasses. See also
  //## itk::ProcessObject::GenerateOutputInformation(),
  //## itk::DataObject::CopyInformation() and
  //## itk::DataObject::UpdateOutputInformation().
  //## mitk::BaseData::GetGeometry2D() makes sure, that the Geometry2D is
  //## up-to-date before returning it (by setting the update extent
  //## appropriately and calling UpdateOutputInformation).
  virtual mitk::Geometry2D* GetGeometry2D(int s) const;

  //##ModelId=3E15578402BD
  //##Documentation
  //## @brief Set Geometry2D of slice @a s.
  virtual bool SetGeometry2D(mitk::Geometry2D* geometry2D, int s);

  virtual void SetTimeBoundsInMS(const mitk::TimeBounds& timebounds);

  //##ModelId=3DCBF5D40253
  virtual const mitk::BoundingBox* GetBoundingBox() const;

  //##Documentation
  //## @brief Get the number of slices
  itkGetConstMacro(Slices, unsigned int);

  //##ModelId=3E3BE1F10106
  //## @brief Check whether a slice exists
  virtual bool IsValidSlice(int s = 0) const;

  //##Documentation
  //## @brief Set the spacing (m_Spacing)
  virtual void SetSpacing(const mitk::Vector3D& aSpacing);

  //##Documentation
  //## @brief Set/Get whether the SlicedGeometry3D is evenly-spaced (m_EvenlySpaced)
  //## 
  //## If (a) m_EvenlySpaced==true, (b) we don't have a Geometry2D stored
  //## for the requested slice, and (c) the first slice (s=0) 
  //## is a PlaneGeometry instance, then we calculate the geometry of the
  //## requested as the plane of the first slice shifted by m_Spacing.z*s
  //## in the direction of m_DirectionVector.
  //##
  //## \sa GetGeometry2D
 	itkGetConstMacro(EvenlySpaced, bool);
  //##ModelId=3E3C13F802A6
  virtual void SetEvenlySpaced(bool on = true);

  //##Documentation
  //## @brief Set/Get the vector between slices for the evenly-spaced case (m_EvenlySpaced==true)
  //## 
  //## If the direction-vector is (0,0,0) (the default) and the first 2D-geometry is a PlaneGeometry, 
  //## then the direction-vector will be calculated from the plane normal.
  //## \sa m_DirectionVector
  virtual void SetDirectionVector(const mitk::Vector3D& directionVector);
  itkGetConstMacro(DirectionVector, const mitk::Vector3D&);

  virtual AffineGeometryFrame3D::Pointer Clone() const;

protected:
  //##ModelId=3E3453C703AF
  //##Documentation
  //## @brief Tell this instance how many Geometry2Ds it shall manage. Bounding box and the 
  //## Geometry2Ds must be set additionally by calling the respective methods!
  //##
  //## @warning Bounding box and the 2D-geometries must be set additionally: use SetBounds() 
  //## SetGeometry().
  virtual void Initialize(unsigned int slices);
public:
  //##Documentation
  //## @brief Completely initialize this instance as evenly-spaced with slices parallel to the provided
  //## Geometry2D that is used as the first slice and for spacing calculation.
  //##
  //## Initializes the bounding box according to the width/height of the Geometry2D and @a slices.
  //## The spacing is calculated from the Geometry2D.
  virtual void InitializeEvenlySpaced(mitk::Geometry2D* geometry2D, unsigned int slices, bool flipped=false);

  //##Documentation
  //## @brief Completely initialize this instance as evenly-spaced with slices parallel to the provided
  //## Geometry2D that is used as the first slice and for spacing calculation (except z-spacing).
  //##
  //## Initializes the bounding box according to the width/height of the Geometry2D and @a slices.
  //## The x-/y-spacing is calculated from the Geometry2D.
  virtual void InitializeEvenlySpaced(mitk::Geometry2D* geometry2D, mitk::ScalarType zSpacing, unsigned int slices, bool flipped=false);

  virtual void SetImageGeometry(const bool isAnImageGeometry);
protected:
  SlicedGeometry3D();

  //##ModelId=3E3456C50067
  virtual ~SlicedGeometry3D();

  void InitializeGeometry(Self * newGeometry) const;

  //##ModelId=3E3968CC000E
  //##Documentation
  //## Container for the 2D-geometries contained within this SliceGeometry3D .
  mutable std::vector<Geometry2D::Pointer> m_Geometry2Ds;

  //##ModelId=3E3C13B70180
  //##Documentation
  //## If (a) m_EvenlySpaced==true, (b) we don't have a Geometry2D stored
  //## for the requested slice, and (c) the first slice (s=0) 
  //## is a PlaneGeometry instance, then we calculate the geometry of the
  //## requested as the plane of the first slice shifted by m_Spacing.z*s
  //## in the direction of m_DirectionVector.
  //##
  //## \sa GetGeometry2D
  bool m_EvenlySpaced;

  //##Documentation
  //## Vector between slices for the evenly-spaced case (m_EvenlySpaced==true).
  //## If the direction-vector is (0,0,0) (the default) and the first 2D-geometry is a PlaneGeometry, 
  //## then the direction-vector will be calculated from the plane normal.
  mutable mitk::Vector3D m_DirectionVector;

  //##Documentation
  //## Number of slices this SliceGeometry3D is descibing.
  unsigned int m_Slices;
};

} // namespace mitk

#endif /* MITKSLICEDGEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD */
