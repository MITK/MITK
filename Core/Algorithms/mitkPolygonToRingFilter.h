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

#ifndef MITKPOLYGONTORINGFILTER_H_HEADER_INCLUDED_C10B22CD
#define MITKPOLYGONTORINGFILTER_H_HEADER_INCLUDED_C10B22CD

#include "mitkCommon.h"
#include "mitkSurfaceSource.h"
#include "mitkMesh.h"

class vtkPolyData;
class vtkPoints;
class vtkCardinalSpline;
class vtkPoints;
class vtkCellArray;


namespace mitk {

//##Documentation
//## @brief Create Coons-Patch-interpolated Surface from mitk::Mesh
//## @ingroup Process
class PolygonToRingFilter : public SurfaceSource
{
public:
  mitkClassMacro(PolygonToRingFilter, SurfaceSource);
  itkNewMacro(Self);

  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  const mitk::Mesh *GetInput(void);

  virtual void SetInput(const mitk::Mesh *image);

  //##Documentation
  //## @brief Get ring radius
  //##
  itkGetMacro(RingRadius, unsigned int);
  //##Documentation
  //## @brief Set ring radius
  //##
  itkSetMacro(RingRadius, unsigned int);

  //##Documentation
  //## @brief Get ring resolution of created Surface
  //##
  itkGetMacro(RingResolution, unsigned int);
  //##Documentation
  //## @brief Set ring resolution of created Surface
  //##
  itkSetMacro(RingResolution, unsigned int);

  //##Documentation
  //## @brief Get spline resolution
  //##
  itkGetMacro(SplineResolution, unsigned int);
  //##Documentation
  //## @brief Set spline resolution
  //##
  itkSetMacro(SplineResolution, unsigned int);
protected:
  typedef std::vector<Vector3D> VectorListType;
  typedef std::vector<Point3D>  PointListType;

  PolygonToRingFilter();

  virtual ~PolygonToRingFilter();

  void BuildVtkTube(vtkPoints *vPoints, vtkCellArray *polys, PointListType& ptList, VectorListType& vecList);
  void BuildPointAndVectorList(mitk::Mesh::CellType& cell, PointListType& ptList, VectorListType& vecList);
  void DrawCyl(vtkPoints *vPoints, vtkCellArray *polys, VectorListType &sl, VectorListType &sc, int idmax, Point3D & last_p, Point3D & cur_p);

  //##Documentation
  //## @brief Ring radius
  //##
  unsigned int m_RingRadius;

  //##Documentation
  //## @brief Ring resolution of created Surface
  //##
  unsigned int m_RingResolution;

  //##Documentation
  //## @brief Spline resolution of created Surface
  //##
  unsigned int m_SplineResolution;

  vtkCardinalSpline *m_SpX, *m_SpY, *m_SpZ;

  PointListType m_PointList;
  VectorListType m_VectorList;
};

} // namespace mitk

#endif /* MITKPOLYGONTORINGFILTER_H_HEADER_INCLUDED_C10B22CD */
