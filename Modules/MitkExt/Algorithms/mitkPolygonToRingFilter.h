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

/**
 * \brief Create ring around polygons contained in a Mesh
 * \ingroup Process
 */
class MITK_CORE_EXPORT PolygonToRingFilter : public SurfaceSource
{
public:
  mitkClassMacro(PolygonToRingFilter, SurfaceSource);
  itkNewMacro(Self);

  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  const mitk::Mesh *GetInput(void);

  virtual void SetInput(const mitk::Mesh *image);

  /** \brief Get ring radius */
  itkGetMacro(RingRadius, float);

  /* \brief Set ring radius */
  itkSetMacro(RingRadius, float);

  /** \brief Get ring resolution of created Surface */
  itkGetMacro(RingResolution, unsigned int);
   
  /** \brief Set ring resolution of created Surface */
  itkSetMacro(RingResolution, unsigned int);

  /** \brief Get spline resolution */
  itkGetMacro(SplineResolution, unsigned int);
   
  /** \brief Set spline resolution */
  itkSetMacro(SplineResolution, unsigned int);

protected:
  typedef std::vector<Vector3D> VectorListType;
  typedef std::vector<Point3D>  PointListType;

  PolygonToRingFilter();

  virtual ~PolygonToRingFilter();

  void BuildVtkTube( vtkPoints *vPoints, vtkCellArray *polys, 
    PointListType& ptList, VectorListType& vecList );
  void BuildPointAndVectorList( mitk::Mesh::CellType& cell, 
    PointListType& ptList, VectorListType& vecList, int timeStep = 0 );
  void DrawCyl(vtkPoints *vPoints, vtkCellArray *polys, 
    VectorListType &sl, VectorListType &sc, int idmax, 
    Point3D & last_p, Point3D & cur_p);

  /** \brief Ring radius */
  float m_RingRadius;

  /** \brief Ring resolution of created Surface */
  unsigned int m_RingResolution;

  /** \brief Spline resolution of created Surface */
  unsigned int m_SplineResolution;

  vtkCardinalSpline *m_SplineX, *m_SplineY, *m_SplineZ;

  PointListType m_PointList;
  VectorListType m_VectorList;
};

} // namespace mitk

#endif /* MITKPOLYGONTORINGFILTER_H_HEADER_INCLUDED_C10B22CD */
