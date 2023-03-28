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

  //##Documentation
  //## @brief Baseclass for renderer slice-/camera-control
  //##
  //## Tells the render (subclass of BaseRenderer) which slice (subclass
  //## SliceNavigationController) or from which direction (subclass
  //## CameraController) it has to render. Contains two Stepper for stepping
  //## through the slices or through different camera views (e.g., for the
  //## creation of a movie around the data), respectively, and through time, if
  //## there is 3D+t data.
  //## @note not yet implemented
  //## @ingroup NavigationControl
  class MITKCORE_EXPORT BaseController : public OperationActor, public itk::Object
  {
  public:
    /** Standard class typedefs. */
    mitkClassMacroItkParent(BaseController, OperationActor);
    itkFactorylessNewMacro(Self);

    //##Documentation
    //## @brief Get the Stepper through the slices
    Stepper *GetSlice();
    const Stepper* GetSlice() const;

    //##Documentation
    //## @brief Get the Stepper through the time
    Stepper *GetTime();
    const Stepper* GetTime() const;

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

    //## @brief Stepper through the time
    Stepper::Pointer m_Time;
    //## @brief Stepper through the slices
    Stepper::Pointer m_Slice;

    unsigned long m_LastUpdateTime;
  };

} // namespace mitk

#endif
