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

#ifndef BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3
#define BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3

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
  class MITKCORE_EXPORT BaseController : public mitk::OperationActor, public itk::Object
  {
  public:
    /** Standard class typedefs. */
    mitkClassMacroItkParent(BaseController, mitk::OperationActor);
    itkFactorylessNewMacro(Self)

      /** Method for creation through ::New */
      // mitkNewMacro(Self);

      //##Documentation
      //## @brief Get the Stepper through the slices
      mitk::Stepper *GetSlice();

    //##Documentation
    //## @brief Get the Stepper through the time
    mitk::Stepper *GetTime();

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

#endif /* BASECONTROLLER_H_HEADER_INCLUDED_C1E745A3 */
