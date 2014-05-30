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

#ifndef MITKINTERPOLATELINESFILTER_H_HEADER_INCLUDED_C10B22CD
#define MITKINTERPOLATELINESFILTER_H_HEADER_INCLUDED_C10B22CD

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkSurfaceSource.h"
#include "mitkMesh.h"
#include "mitkPlaneGeometry.h"

class vtkPolyData;
class vtkPoints;
class vtkCardinalSpline;
class vtkPoints;
class vtkCellArray;


namespace mitk {

//##Documentation
//## @brief Interpolate additional points on lines
//##
//## If a PlaneGeometry is set (by SetGeometryForInterpolation),
//## we do an interpolation in the 2D-space of the PlaneGeometry:
//## Map two neighboring original points on
//## the PlaneGeometry, resulting in two 2D-points, interpolate
//## in 2D between them, and map them back via the
//## PlaneGeometry in the 3D-world.
//## @ingroup Process
class MitkExt_EXPORT InterpolateLinesFilter : public SurfaceSource
{
public:
  mitkClassMacro(InterpolateLinesFilter, SurfaceSource);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  const mitk::Mesh *GetInput(void);

  virtual void SetInput(const mitk::Mesh *image);

  //##Documentation
  //## @brief Get spline resolution
  //##
  itkGetMacro(SplineResolution, unsigned int);
  //##Documentation
  //## @brief Set spline resolution
  //##
  itkSetMacro(SplineResolution, unsigned int);

  //##Documentation
  //## @brief Get/set geometry for interpolation
  //##
  //## If this is set (not NULL), we do an interpolation in the
  //## 2D-space of the PlaneGeometry: Map two neighboring original
  //## points on the PlaneGeometry, resulting in two 2D-points,
  //## interpolate in 2D between them, and map them back via the
  //## PlaneGeometry in the 3D-world.
  itkGetConstObjectMacro(GeometryForInterpolation, mitk::PlaneGeometry);
  itkSetObjectMacro(GeometryForInterpolation, mitk::PlaneGeometry);

  //##Documentation
  //## @brief Get the overall length of the interpolated lines
  //##
  //## @warning valid only after Update()
  itkGetMacro(Length, ScalarType);
protected:

  InterpolateLinesFilter();
  virtual ~InterpolateLinesFilter();

  void BuildPointAndVectorList(mitk::Mesh::CellType& cell, vtkPoints* points, vtkCellArray* cellarray);

  //##Documentation
  //## @brief Spline resolution of created Surface
  //##
  unsigned int m_SplineResolution;

  mitk::PlaneGeometry::Pointer m_GeometryForInterpolation;
  vtkCardinalSpline *m_SpX, *m_SpY, *m_SpZ;

  ScalarType m_Length;
};

} // namespace mitk

#endif /* MITKINTERPOLATELINESFILTER_H_HEADER_INCLUDED_C10B22CD */
