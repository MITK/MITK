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

#ifndef MITKPLANESPERPENDICULARTOLINES_H_HEADER_INCLUDED_C10B22CD
#define MITKPLANESPERPENDICULARTOLINES_H_HEADER_INCLUDED_C10B22CD

#include "mitkGeometryDataSource.h"
#include "mitkMesh.h"
#include "mitkGeometryData.h"
#include "mitkPlaneGeometry.h"
#include "mitkSlicedGeometry3D.h"

namespace mitk {

//##Documentation
//## @brief Create Planes perpendicular to lines containted in a Mesh
//## @ingroup Process
class PlanesPerpendicularToLinesFilter : public GeometryDataSource
{
public:
  mitkClassMacro(PlanesPerpendicularToLinesFilter, GeometryDataSource);
  itkNewMacro(Self);

  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  const mitk::Mesh *GetInput(void);

  virtual void SetInput(const mitk::Mesh *image);

  //##Documentation
  //## @brief Set plane to be used as an example of the planes to move
  //## along the lines in the input mesh.
  //##
  //## The size and spacing are copied from the plane. The in-plane 
  //## orientation (right-vector) of the created planes are set as
  //## parallel as possible to the orientation (right-vector) of the
  //## the plane set using this method.
  //## @note The PlaneGeometry is cloned, @em not linked/referenced.
  virtual void SetPlane(const mitk::PlaneGeometry* aPlane);

  //##Documentation
  //## @brief Set if all points in the mesh should be interpreted as
  //## one long line.
  //##
  //## Cells are not used in this mode, but all points in the order
  //## of their indices form the line.
  //## Default is @a false.
  itkGetConstMacro(UseAllPoints, bool);
  itkSetMacro(UseAllPoints, bool);
  itkBooleanMacro(UseAllPoints);

  //##Documentation
  //## @brief Set an explicit frame of the created sliced geometry
  //##
  //## Uses the IndexToWorldTransform and bounding box of the
  //## provided geometry.
  //## \sa CalculateFrameGeometry
  virtual void SetFrameGeometry(const mitk::Geometry3D* frameGeometry);

protected:
  PlanesPerpendicularToLinesFilter();

  virtual ~PlanesPerpendicularToLinesFilter();

  void CreatePlane(const Point3D& curr);

  //## @brief Plane to be used as an example of the planes to move
  //## along the lines in the input mesh.
  //##
  //## The size and spacing are copied from the m_Plane. The in-plane 
  //## orientation (right-vector) of the created planes are set as
  //## parallel as possible to the orientation (right-vector) of m_Plane.
  //## \sa SetPlane
  mitk::PlaneGeometry::Pointer m_Plane;

  bool m_UseAllPoints;

  //##Documentation
  //## @brief SlicedGeometry3D containing the created planes
  //##
  SlicedGeometry3D::Pointer m_CreatedGeometries;

  mitk::Geometry3D::Pointer m_FrameGeometry;

private:
  std::deque<mitk::PlaneGeometry::Pointer> planes;
  Point3D last;
  VnlVector normal;
  VnlVector right, down;
  VnlVector targetRight;
  Vector3D targetSpacing;
  ScalarType halfWidthInMM, halfHeightInMM;
  mitk::AffineGeometryFrame3D::BoundsArrayType bounds;
  Point3D origin;
};

} // namespace mitk

#endif /* MITKPLANESPERPENDICULARTOLINES_H_HEADER_INCLUDED_C10B22CD */
