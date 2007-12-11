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


#ifndef MITKPLANELANDMARKPROJECTOR_H_HEADER_INCLUDED_C1C68A2C
#define MITKPLANELANDMARKPROJECTOR_H_HEADER_INCLUDED_C1C68A2C

#include "mitkLandmarkProjector.h"
#include "mitkPointSet.h"

namespace mitk {

//##Documentation
//## @brief Thin-plate-spline-based landmark-based curved geometry
//## 
//## @ingroup Geometry
class PlaneLandmarkProjector : public LandmarkProjector
{
public:
  mitkClassMacro(PlaneLandmarkProjector, LandmarkProjector);
  
  itkNewMacro(Self);
  
  //##Documentation
  //## @brief Set the plane-geometry to project the target-landmarks on.
  //##
  itkSetConstObjectMacro(ProjectionPlane, mitk::PlaneGeometry);
  //##Documentation
  //## @brief Get the plane-geometry to project the target-landmarks on.
  //##
  itkGetConstObjectMacro(ProjectionPlane, mitk::PlaneGeometry);

  virtual void ProjectLandmarks(const mitk::PointSet::DataType::PointsContainer* targetLandmarks);
protected:
  PlaneLandmarkProjector();
  virtual ~PlaneLandmarkProjector();

  virtual void ComputeCompleteAbstractTransform();

  mitk::PlaneGeometry::ConstPointer m_ProjectionPlane;
};

} // namespace mitk

#endif /* MITKPLANELANDMARKPROJECTOR_H_HEADER_INCLUDED_C1C68A2C */
