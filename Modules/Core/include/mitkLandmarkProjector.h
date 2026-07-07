/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLandmarkProjector_h
#define mitkLandmarkProjector_h

#include <itkObject.h>
#include <MitkCoreExports.h>

#include <mitkPlaneGeometry.h>
#include <mitkPointSet.h>

class vtkAbstractTransform;

namespace mitk
{
  /**
   * \brief Base class of landmark projectors, which project the target landmarks
   *        to create source landmarks.
   *
   * \ingroup Geometry
   */
  class MITKCORE_EXPORT LandmarkProjector : public itk::Object
  {
  public:
    mitkClassMacroItkParent(LandmarkProjector, itk::Object);

    /** \brief Set the interpolating (world-space-to-world-space) transform,
     *         which uses the landmarks.
     *
     * \sa GetCompleteAbstractTransform
     */
    virtual void SetInterpolatingAbstractTransform(vtkAbstractTransform *anInterpolatingAbstractTransform);

    /** \brief Get the interpolating (world-space-to-world-space) transform,
     *         which uses the landmarks.
     *
     * \sa GetCompleteAbstractTransform
     * \sa ComputeCompleteAbstractTransform
     */
    itkGetConstMacro(InterpolatingAbstractTransform, vtkAbstractTransform *);

    /** \brief Set frame geometry within which the interpolation shall occur.
     *
     * Used as a hint, may be ignored depending on the concrete sub-classes.
     */
    itkSetConstObjectMacro(FrameGeometry, mitk::BaseGeometry);

    /** \brief Get frame geometry within which the interpolation shall occur.
     *
     * Used as a hint, may be ignored depending on the concrete sub-classes.
     */
    itkGetConstObjectMacro(FrameGeometry, mitk::BaseGeometry);

    /** \brief Get the parameter plane for use in AbstractTransformGeometry::SetPlane. */
    itkGetConstObjectMacro(ParameterPlane, mitk::PlaneGeometry);

    /** \brief Get the projected landmarks.
     *
     * \note Valid only after calling ProjectLandmarks.
     */
    itkGetConstObjectMacro(ProjectedLandmarks, mitk::PointSet::DataType::PointsContainer);

    /** \brief Get the final target landmarks to use for the interpolating transform.
     *
     * \note Valid only after calling ProjectLandmarks.
     */
    itkGetConstObjectMacro(FinalTargetLandmarks, mitk::PointSet::DataType::PointsContainer);

    /** \brief Get the transform from parameter space to world space incorporating
     *         the given interpolating transform, which uses the landmarks.
     *
     * \sa ComputeCompleteAbstractTransform
     * \sa SetInterpolatingAbstractTransform
     */
    virtual vtkAbstractTransform *GetCompleteAbstractTransform() const;

    /** \brief Project the given target landmarks to create source landmarks.
     *
     * Must be implemented by subclasses.
     */
    virtual void ProjectLandmarks(const mitk::PointSet::DataType::PointsContainer *targetLandmarks) = 0;

  protected:
    LandmarkProjector();
    ~LandmarkProjector() override;

    /** \brief Compute the transform from parameter space to world space incorporating
     *         the given interpolating transform, which uses the landmarks.
     *
     * Called after a new interpolating transform is set via
     * SetInterpolatingAbstractTransform().
     *
     * \sa SetInterpolatingAbstractTransform
     * \sa GetCompleteAbstractTransform
     */
    virtual void ComputeCompleteAbstractTransform() = 0;

    vtkAbstractTransform *m_InterpolatingAbstractTransform;
    vtkAbstractTransform *m_CompleteAbstractTransform;
    mitk::BaseGeometry::ConstPointer m_FrameGeometry;
    mutable mitk::PlaneGeometry::ConstPointer m_ParameterPlane;
    mitk::PointSet::DataType::PointsContainer::Pointer m_WritableFinalTargetLandmarks;
    mitk::PointSet::DataType::PointsContainer::ConstPointer m_FinalTargetLandmarks;
    mitk::PointSet::DataType::PointsContainer::Pointer m_ProjectedLandmarks;
  };

} // namespace mitk

#endif
