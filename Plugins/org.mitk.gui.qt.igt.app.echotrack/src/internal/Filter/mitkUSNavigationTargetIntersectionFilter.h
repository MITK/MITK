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

#ifndef MITKUSNAVIGATIONTARGETINTERSECTIONFILTER_H
#define MITKUSNAVIGATIONTARGETINTERSECTIONFILTER_H

#include "mitkCommon.h"
#include "mitkNumericTypes.h"

#include "itkObjectFactory.h"

#include "vtkSmartPointer.h"
#include "vtkPolyData.h"

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk {
class Surface;
class PointSet;

/**
 * \brief Calculates the intersection between a mitk::Surface and a line.
 * Surface and line have to be set by SetTargetSurface() and SetLine.
 * CalculateIntersection() can be called then to calculate the intersection.
 * The results can be got afterwards by GetIsIntersecting(),
 * GetIntersectionPoint(), GetIntersectionNearestSurfacePointId() and
 * GetDistanceToIntersection().
 */
class USNavigationTargetIntersectionFilter : public itk::Object {
public:
  mitkClassMacroItkParent(USNavigationTargetIntersectionFilter, itk::Object)
  itkNewMacro(Self)

  itkSetMacro(TargetSurface, itk::SmartPointer<mitk::Surface>)
  void SetLine(itk::SmartPointer<mitk::PointSet> line);

  /**
   * \return if line and surface are intersecting
   */
  itkGetMacro(IsIntersecting, bool)

  /**
   * \return the position where the intersection occured, the result is not defined if no intersection occured
   */
  mitk::Point3D GetIntersectionPoint();

  /**
   * \return id of the nearest vertex of the target to the interseciont, the result is not defined if no intersection occured
   */
  int GetIntersectionNearestSurfacePointId();

  /**
   * \return distance between the first line point and the intersection point
   */
  double GetDistanceToIntersection();

  /**
   * \brief Does the intersection calculation.
   * SetTargetSurface() and SetLine() have to be called before.
   */
  void CalculateIntersection();

protected:
  USNavigationTargetIntersectionFilter();
  virtual ~USNavigationTargetIntersectionFilter();

  itk::SmartPointer<mitk::Surface> m_TargetSurface;
  vtkSmartPointer<vtkPolyData>     m_TargetSurfaceVtk;
  double                           m_LinePoint1[3];
  double                           m_LinePoint2[3];

  bool                             m_IsIntersecting;
  double                           m_IntersectionPoint[3];
  int                              m_IntersectionCellId;
  double                           m_Distance;
  };
}

#endif // MITKUSNAVIGATIONTARGETINTERSECTIONFILTER_H
