/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkThinPlateSplineCurvedGeometry_h
#define mitkThinPlateSplineCurvedGeometry_h

#include <mitkLandmarkProjectorBasedCurvedGeometry.h>

class vtkPoints;
class vtkThinPlateSplineTransform;

namespace mitk
{
  /**
   * \brief Thin-plate-spline-based landmark-based curved geometry.
   *
   * Uses a vtkThinPlateSplineTransform to deform a planar geometry based
   * on source and target landmark positions.
   *
   * \ingroup Geometry
   */
  class MITKCORE_EXPORT ThinPlateSplineCurvedGeometry : public LandmarkProjectorBasedCurvedGeometry
  {
  public:
    mitkClassMacro(ThinPlateSplineCurvedGeometry, LandmarkProjectorBasedCurvedGeometry);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /**
     * \brief Compute the geometry by initializing the thin-plate-spline transform
     *        from projected and target landmarks.
     */
    void ComputeGeometry() override;

    /**
     * \brief Return the underlying vtkThinPlateSplineTransform.
     * \return Pointer to the internal vtkThinPlateSplineTransform instance.
     */
    vtkThinPlateSplineTransform *GetThinPlateSplineTransform() const { return m_ThinPlateSplineTransform; }

    /**
     * \brief Set the stiffness parameter (sigma) of the thin-plate-spline transform.
     * \param[in] sigma The sigma value controlling the stiffness of the spline.
     */
    virtual void SetSigma(double sigma);

    /**
     * \brief Get the stiffness parameter (sigma) of the thin-plate-spline transform.
     * \return The current sigma value.
     */
    virtual double GetSigma() const;

    /**
     * \brief Check whether the geometry is valid.
     *
     * A ThinPlateSplineCurvedGeometry is valid when it has at least 3 target
     * landmarks and a landmark projector has been set.
     *
     * \return True if the geometry is valid, false otherwise.
     */
    bool IsValid() const override;

  protected:
    ThinPlateSplineCurvedGeometry();
    ThinPlateSplineCurvedGeometry(const ThinPlateSplineCurvedGeometry &other);

    ~ThinPlateSplineCurvedGeometry() override;

    vtkThinPlateSplineTransform *m_ThinPlateSplineTransform;

    vtkSmartPointer<vtkPoints> m_VtkTargetLandmarks;
    vtkSmartPointer<vtkPoints> m_VtkProjectedLandmarks;

    /**
     * \brief Hook for subclasses to customize spacing behavior.
     *
     * This virtual function allows different behavior in subclasses.
     * Implement in every subclass of BaseGeometry. If not needed, use
     * {Superclass::PreSetSpacing();}.
     */
    void PreSetSpacing(const mitk::Vector3D &aSpacing) override { Superclass::PreSetSpacing(aSpacing); };

    mitkCloneMacro(Self);
  };
} // namespace mitk

#endif
