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

#ifndef _MITK_POINT_SET_TO_CURVED_GEOMETRY_FILTER_H__
#define _MITK_POINT_SET_TO_CURVED_GEOMETRY_FILTER_H__

#include "mitkPointSetToGeometryDataFilter.h"
#include "MitkExtExports.h"
#include <mitkPlaneGeometry.h>
#include <mitkSphereLandmarkProjector.h>
#include <mitkPlaneLandmarkProjector.h>
#include <mitkPlaneFit.h>
#include <mitkDataNode.h>
#include <mitkImage.h>

namespace mitk
{

class MitkExt_EXPORT PointSetToCurvedGeometryFilter : public PointSetToGeometryDataFilter
{
public:
  /**
   * Standard mitk class macro
   */
  mitkClassMacro ( PointSetToCurvedGeometryFilter, PointSetToGeometryDataFilter );

  itkNewMacro ( Self );

  /**
   * Defines the geometry, onto which the input landmarks are
   * projected. Currently, Plane and Sphere are supported, whilest plane
   * is differentiated into 4 different planes, the xy, xz, and yz plane,
   * as well a plane which is calculated by a principal component analysis
   * of the input point set.
   */
  enum ProjectionMode {Sphere, XYPlane, XZPlane, YZPlane, PCAPlane};

  /**
   * Fills a data tree node with appropriate properties
   * for mapping the output geometry surface
   */
  static void SetDefaultCurvedGeometryProperties( mitk::DataNode* node );

  /**
   * Sets / Gets whether the input land marks are projected onto a plane
   * or sphere
   */
  itkSetMacro ( ProjectionMode, ProjectionMode );

  /**
   * Sets / Gets whether the input land marks are projected onto a plane
   * or sphere
   */
  itkGetMacro ( ProjectionMode, ProjectionMode );

  itkSetObjectMacro( ImageToBeMapped, mitk::Image );
  itkGetObjectMacro( ImageToBeMapped, mitk::Image );

  itkSetMacro( Sigma, float);
  itkGetMacro( Sigma, float);

protected:

  /**
   * Protected constructor. Use ::New instead()
   */
  PointSetToCurvedGeometryFilter();

  /**
   * Protected destructor. Instances are destroyed when reference count is zero
   */
  virtual ~PointSetToCurvedGeometryFilter();

  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  ProjectionMode m_ProjectionMode;

  mitk::PlaneGeometry::Pointer m_XYPlane;

  mitk::PlaneGeometry::Pointer m_XZPlane;

  mitk::PlaneGeometry::Pointer m_YZPlane;

  mitk::PlaneFit::Pointer m_PCAPlaneCalculator;

  mitk::Image::Pointer m_ImageToBeMapped;

  mitk::PlaneLandmarkProjector::Pointer m_PlaneLandmarkProjector;

  mitk::SphereLandmarkProjector::Pointer m_SphereLandmarkProjector;

  float m_Sigma;

};

}

#endif
