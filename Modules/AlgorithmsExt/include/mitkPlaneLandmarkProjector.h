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


#ifndef MITKPLANELANDMARKPROJECTOR_H_HEADER_INCLUDED_C1C68A2C
#define MITKPLANELANDMARKPROJECTOR_H_HEADER_INCLUDED_C1C68A2C

#include "mitkLandmarkProjector.h"
#include "MitkAlgorithmsExtExports.h"
#include "mitkPointSet.h"

namespace mitk {

//##Documentation
//## @brief Thin-plate-spline-based landmark-based curved geometry
//##
//## @ingroup Geometry
class MITKALGORITHMSEXT_EXPORT PlaneLandmarkProjector : public LandmarkProjector
{
public:
  mitkClassMacro(PlaneLandmarkProjector, LandmarkProjector);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  //##Documentation
  //## @brief Set the plane-geometry to project the target-landmarks on.
  //##
  itkSetConstObjectMacro(ProjectionPlane, mitk::PlaneGeometry);
  //##Documentation
  //## @brief Get the plane-geometry to project the target-landmarks on.
  //##
  itkGetConstObjectMacro(ProjectionPlane, mitk::PlaneGeometry);

  virtual void ProjectLandmarks(const mitk::PointSet::DataType::PointsContainer* targetLandmarks) override;
protected:
  PlaneLandmarkProjector();
  virtual ~PlaneLandmarkProjector();

  virtual void ComputeCompleteAbstractTransform() override;

  mitk::PlaneGeometry::ConstPointer m_ProjectionPlane;
};

} // namespace mitk

#endif /* MITKPLANELANDMARKPROJECTOR_H_HEADER_INCLUDED_C1C68A2C */
