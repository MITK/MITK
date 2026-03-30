/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLandmarkProjectorBasedCurvedGeometry_h
#define mitkLandmarkProjectorBasedCurvedGeometry_h

#include <mitkLandmarkProjector.h>

#include <mitkAbstractTransformGeometry.h>
#include <mitkPointSet.h>

#include <vtkSmartPointer.h>

namespace mitk
{
  /**
   * \brief Superclass of AbstractTransformGeometry sub-classes defined
   *        by a set of landmarks.
   *
   * \ingroup Geometry
   */
  class MITKCORE_EXPORT LandmarkProjectorBasedCurvedGeometry : public AbstractTransformGeometry
  {
  public:
    mitkClassMacro(LandmarkProjectorBasedCurvedGeometry, AbstractTransformGeometry);
    itkCloneMacro(Self);

    /** \brief Set the landmark projector used to project target landmarks. */
    void SetLandmarkProjector(mitk::LandmarkProjector *aLandmarkProjector);

    /** \brief Get the landmark projector. */
    itkGetConstObjectMacro(LandmarkProjector, mitk::LandmarkProjector);

    /** \brief Set the frame geometry and forward it to the landmark projector. */
    void SetFrameGeometry(const mitk::BaseGeometry *frameGeometry) override;

    /** \brief Compute the geometry by projecting the target landmarks through the projector. */
    virtual void ComputeGeometry();

    /** \brief Get the interpolating abstract transform. */
    itkGetConstMacro(InterpolatingAbstractTransform, vtkAbstractTransform *);

    /** \brief Set the landmarks through which the geometry shall pass. */
    itkSetConstObjectMacro(TargetLandmarks, mitk::PointSet::DataType::PointsContainer);

    /** \brief Get the landmarks through which the geometry shall pass. */
    itkGetConstObjectMacro(TargetLandmarks, mitk::PointSet::DataType::PointsContainer);

  protected:
    LandmarkProjectorBasedCurvedGeometry();

    LandmarkProjectorBasedCurvedGeometry(const LandmarkProjectorBasedCurvedGeometry &other);

    ~LandmarkProjectorBasedCurvedGeometry() override;

    mitk::LandmarkProjector::Pointer m_LandmarkProjector;

    vtkSmartPointer<vtkAbstractTransform> m_InterpolatingAbstractTransform;

    mitk::PointSet::DataType::PointsContainer::ConstPointer m_TargetLandmarks;

    /** \brief Pre-set spacing hook for subclasses.
     *
     * This virtual function allows different behavior in subclasses.
     * Implement in every subclass of BaseGeometry. If not needed, use
     * \c Superclass::PreSetSpacing().
     */
    void PreSetSpacing(const mitk::Vector3D &aSpacing) override { Superclass::PreSetSpacing(aSpacing); };

    mitkCloneMacro(Self);
  };
} // namespace mitk

#endif
