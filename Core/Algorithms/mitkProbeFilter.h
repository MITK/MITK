/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

////coming from mitkGeometry2DDataToSurfaceFilter.h
=========================================================================*/

#ifndef MITKGEOMETRY2DDATATOSURFACEDATAFILTER_H_HEADER_INCLUDED_C10B22CD
#define MITKGEOMETRY2DDATATOSURFACEDATAFILTER_H_HEADER_INCLUDED_C10B22CD

#include "mitkCommon.h"
#include "mitkSurfaceSource.h"

//#include "mitkGeometry3D.h"

#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkImageData.h>

class vtkPlaneSource;
class vtkTransformPolyDataFilter;
class vtkDataSetToPolyDataFilter;

namespace mitk {

class Surface;

//##Documentation
//## @brief Superclass of all classes having a Geometry2DData as input and
//## generating Images as output
//## @ingroup Process
//## Currently implemented for PlaneGeometry and AbstractTransformGeometry.
//## Currently, this class does not really have subclasses, but does the job
//## for itself. It checks which kind of Geometry2D is stored in the
//## Geometry2DData and - if it knows how - it generates the respective
//## Surface. Of course, this has the disadvantage that for any new type of
//## Geometry2D this class (ProbeFilter) has to be
//## changed/extended. The idea is to move the type specific generation code in
//## subclasses, and internally (within this class) use a factory to create an
//## instance of the required subclass and delegate the surface generation to
//## it.
//## @todo make extension easier
class ProbeFilter : public SurfaceSource
{
public:
  mitkClassMacro(ProbeFilter, SurfaceSource);
  itkNewMacro(Self);

  virtual void GenerateOutputInformation();
  virtual void GenerateInputRequestedRegion();
  virtual void GenerateData();

  const mitk::Surface *GetInput(void);
  const mitk::Image *GetSource(void);

  virtual void SetInput(const mitk::Surface *input);
  virtual void SetSource(const mitk::Image *source);

  //##Documentation
  //## @brief If @a true (default), use Geometry3D::GetParametricBounds() to define the resolution in parameter space, 
  //## otherwise use m_XResolution and m_YResolution
  itkGetMacro(UseGeometryParametricBounds, bool);
  //##Documentation
  //## @brief If @a true (default), use Geometry3D::GetParametricBounds() to define the resolution in parameter space, 
  //## otherwise use m_XResolution and m_YResolution
  itkSetMacro(UseGeometryParametricBounds, bool);

  //##Documentation
  //## @brief Get x-resolution in parameter space
  //##
  //## The m_VtkPlaneSource will create this many sub-rectangles
  //## in x-direction (see vtkPlaneSource::SetXResolution)
  //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
  //## the x-bounds of Geometry3D::GetParametricBounds() are used.
  //## @sa m_XResolution
  itkGetMacro(XResolution, int);
  //##Documentation
  //## @brief Set x-resolution in parameter space
  //##
  //## The m_VtkPlaneSource will create this many sub-rectangles
  //## in x-direction (see vtkPlaneSource::SetXResolution)
  //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
  //## the x-bounds of Geometry3D::GetParametricBounds() are used.
  //## @sa m_XResolution
  itkSetMacro(XResolution, int);

  //##Documentation
  //## @brief Get y-resolution in parameter space
  //##
  //## The m_VtkPlaneSource will create this many sub-rectangles
  //## in y-direction (see vtkPlaneSource::SetYResolution)
  //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
  //## the y-bounds of Geometry3D::GetParametricBounds() are used.
  //## @sa m_YResolution
  itkGetMacro(YResolution, int);
  //##Documentation
  //## @brief Set y-resolution in parameter space
  //##
  //## The m_VtkPlaneSource will create this many sub-rectangles
  //## in y-direction (see vtkPlaneSource::SetYResolution)
  //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
  //## the y-bounds of Geometry3D::GetParametricBounds() are used.
  //## @sa m_YResolution
  itkSetMacro(YResolution, int);

protected:
  ProbeFilter();

  virtual ~ProbeFilter();

  //##Documentation
  //## @brief Source to create the vtk-representation of the parameter space rectangle of the Geometry2D
  vtkPlaneSource* m_VtkPlaneSource;

  //##Documentation
  //## @brief Filter to create the vtk-representation of the Geometry2D, which is a
  //## transformation of the m_VtkPlaneSource
  vtkTransformPolyDataFilter* m_VtkTransformPlaneFilter;

  //##Documentation
  //## @brief If @a true, use Geometry3D::GetParametricBounds() to define the resolution in parameter space, 
  //## otherwise use m_XResolution and m_YResolution
  bool m_UseGeometryParametricBounds;

  //##Documentation
  //## @brief X-resolution in parameter space
  //##
  //## The m_VtkPlaneSource will create this many sub-rectangles
  //## in x-direction (see vtkPlaneSource::SetXResolution)
  //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
  //## the x-bounds of Geometry3D::GetParametricBounds() are used.
  //## @sa m_XResolution
  int m_XResolution;

  //##Documentation
  //## @brief Y-resolution in parameter space
  //##
  //## The m_VtkPlaneSource will create this many sub-rectangles
  //## in y-direction (see vtkPlaneSource::SetYResolution)
  //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
  //## the y-bounds of Geometry3D::GetParametricBounds() are used.
  int m_YResolution;
};

} // namespace mitk

#endif /* MITKGEOMETRY2DDATATOSURFACEDATAFILTER_H_HEADER_INCLUDED_C10B22CD */
