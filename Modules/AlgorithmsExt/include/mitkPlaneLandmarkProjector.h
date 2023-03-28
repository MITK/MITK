/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlaneLandmarkProjector_h
#define mitkPlaneLandmarkProjector_h

#include "MitkAlgorithmsExtExports.h"
#include "mitkLandmarkProjector.h"
#include "mitkPointSet.h"

namespace mitk
{
  //##Documentation
  //## @brief Thin-plate-spline-based landmark-based curved geometry
  //##
  //## @ingroup Geometry
  class MITKALGORITHMSEXT_EXPORT PlaneLandmarkProjector : public LandmarkProjector
  {
  public:
    mitkClassMacro(PlaneLandmarkProjector, LandmarkProjector);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      //##Documentation
      //## @brief Set the plane-geometry to project the target-landmarks on.
      //##
      itkSetConstObjectMacro(ProjectionPlane, mitk::PlaneGeometry);
    //##Documentation
    //## @brief Get the plane-geometry to project the target-landmarks on.
    //##
    itkGetConstObjectMacro(ProjectionPlane, mitk::PlaneGeometry);

    void ProjectLandmarks(const mitk::PointSet::DataType::PointsContainer *targetLandmarks) override;

  protected:
    PlaneLandmarkProjector();
    ~PlaneLandmarkProjector() override;

    void ComputeCompleteAbstractTransform() override;

    mitk::PlaneGeometry::ConstPointer m_ProjectionPlane;
  };

} // namespace mitk

#endif
