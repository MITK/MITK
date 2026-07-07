/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlaneLandmarkProjector_h
#define mitkPlaneLandmarkProjector_h

#include <MitkAlgorithmsExtExports.h>
#include <mitkLandmarkProjector.h>
#include <mitkPointSet.h>

namespace mitk
{
  /**
   * \brief Thin-plate-spline-based landmark projector onto a plane geometry.
   *
   * This class projects landmarks onto a PlaneGeometry using a thin-plate
   * spline transformation. The projected landmarks define a curved geometry
   * that is used for deformable surface registration and visualization.
   *
   * \sa LandmarkProjector
   * \sa PlaneGeometry
   * \ingroup Geometry
   */
  class MITKALGORITHMSEXT_EXPORT PlaneLandmarkProjector : public LandmarkProjector
  {
  public:
    mitkClassMacro(PlaneLandmarkProjector, LandmarkProjector);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
       * \brief Set the plane geometry onto which target landmarks are projected.
       * \param[in] _arg The PlaneGeometry for projection.
       */
      itkSetConstObjectMacro(ProjectionPlane, mitk::PlaneGeometry);

    /**
     * \brief Get the plane geometry used for projection.
     * \return Const pointer to the projection PlaneGeometry.
     */
    itkGetConstObjectMacro(ProjectionPlane, mitk::PlaneGeometry);

    /**
     * \brief Project the target landmarks onto the projection plane.
     * \param[in] targetLandmarks Container of target landmark points.
     */
    void ProjectLandmarks(const mitk::PointSet::DataType::PointsContainer *targetLandmarks) override;

  protected:
    PlaneLandmarkProjector();
    ~PlaneLandmarkProjector() override;

    void ComputeCompleteAbstractTransform() override;

    mitk::PlaneGeometry::ConstPointer m_ProjectionPlane;
  };

} // namespace mitk

#endif
