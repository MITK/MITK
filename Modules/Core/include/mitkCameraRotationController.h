/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCameraRotationController_h
#define mitkCameraRotationController_h

#include <mitkBaseController.h>
#include <MitkCoreExports.h>

class vtkCamera;
class vtkRenderWindow;

namespace mitk
{
  /**
   * \brief Controls azimuthal camera rotation driven by a Stepper.
   *
   * Uses an internal Stepper (inherited from BaseController) with 360 steps
   * and auto-repeat to rotate the VTK camera around its focal point. Each
   * time the stepper position changes, the camera is rotated by the
   * corresponding azimuth delta and the render window is updated.
   *
   * \ingroup NavigationControl
   * \sa BaseController CameraController
   */
  class MITKCORE_EXPORT CameraRotationController : public BaseController
  {
  public:
    mitkClassMacro(CameraRotationController, BaseController);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self)
      // mitkNewMacro1Param(Self, const char *);

    /**
     * \brief Rotate the camera by the azimuth delta since the last stepper position.
     *
     * Called automatically when the internal stepper changes. Acquires the
     * camera on first invocation if it has not been acquired yet.
     */
    void RotateCamera();

    /**
     * \brief Acquire the active VTK camera from the render window.
     *
     * Retrieves the vtkCamera from the VtkPropRenderer associated with
     * the current render window and stores it internally.
     */
    void AcquireCamera();

    /**
     * \brief Set the VTK render window whose camera will be rotated.
     */
    void SetRenderWindow(vtkRenderWindow *renWin) { m_RenderWindow = renWin; }

  protected:
    /** \brief Constructor. Initializes the stepper with 360 steps and auto-repeat. */
    CameraRotationController();
    /** \brief Destructor. */
    ~CameraRotationController() override;

  private:
    int m_LastStepperValue;  ///< Previous stepper position for computing azimuth delta.
    vtkCamera *m_Camera;  ///< The VTK camera being rotated.
    vtkRenderWindow *m_RenderWindow;  ///< The render window providing the camera.
  };
}

#endif
