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

#ifndef MITKINTERPOLATELINESFILTER_H_HEADER_INCLUDED_C10B22CD
#define MITKINTERPOLATELINESFILTER_H_HEADER_INCLUDED_C10B22CD

#include "mitkCommon.h"
#include "mitkSurfaceSource.h"
#include "mitkMesh.h"
#include "mitkGeometry2D.h"

class vtkPolyData;
class vtkPoints;
class vtkCardinalSpline;
class vtkPoints;
class vtkCellArray;


namespace mitk {

//##Documentation
//## @brief Interpolate additional points on lines
//##
//## If a Geometry2D is set (by SetGeometryForInterpolation), 
//## we do an interpolation in the 2D-space of the Geometry2D: 
//## Map two neighboring original points on 
//## the Geometry2D, resulting in two 2D-points, interpolate 
//## in 2D between them, and map them back via the 
//## Geometry2D in the 3D-world. 
//## @ingroup Process
class InterpolateLinesFilter : public SurfaceSource
{
public:
  mitkClassMacro(InterpolateLinesFilter, SurfaceSource);
  itkNewMacro(Self);

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
  //## 2D-space of the Geometry2D: Map two neighboring original
  //## points on the Geometry2D, resulting in two 2D-points, 
  //## interpolate in 2D between them, and map them back via the 
  //## Geometry2D in the 3D-world. 
  itkGetConstObjectMacro(GeometryForInterpolation, mitk::Geometry2D);
  itkSetObjectMacro(GeometryForInterpolation, mitk::Geometry2D);

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

  mitk::Geometry2D::Pointer m_GeometryForInterpolation;
  vtkCardinalSpline *m_SpX, *m_SpY, *m_SpZ;

  ScalarType m_Length;
};

} // namespace mitk

#endif /* MITKINTERPOLATELINESFILTER_H_HEADER_INCLUDED_C10B22CD */
