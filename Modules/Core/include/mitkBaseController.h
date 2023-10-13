/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBaseController_h
#define mitkBaseController_h

#include "mitkEventStateMachine.h"
#include "mitkOperationActor.h"
#include "mitkStepper.h"
#include <MitkCoreExports.h>
#include <itkObjectFactory.h>

namespace mitk
{
  class BaseRenderer;

  /**
   * \brief Baseclass for renderer slice-/camera-/time-control
   *
   *        Tells the renderer (subclass of BaseRenderer) which slice (SliceNavigationController),
   *        which direction (CameraController) or which time (TimeNavigationController) it has to render.
   *        Contains a Stepper for stepping through the slices, through different
   *        camera views (e.g., for the creation of a movie around the data)
   *        and through time, if there is 3D+t data.
   */
  class MITKCORE_EXPORT BaseController : public OperationActor, public itk::Object
  {
  public:

    mitkClassMacroItkParent(BaseController, OperationActor);
    itkFactorylessNewMacro(Self);

    /**
    * \brief Get the stepper through the corresponding dimension.
    */
    Stepper* GetStepper();
    const Stepper* GetStepper() const;

  protected:
    /**
    * @brief Default Constructor
    **/
    BaseController();

    /**
    * @brief Default Destructor
    **/
    ~BaseController() override;

    void ExecuteOperation(Operation *) override;

    //## @brief Stepper through the different dimensions
    Stepper::Pointer m_Stepper;

    unsigned long m_LastUpdateTime;
  };

} // namespace mitk

#endif
