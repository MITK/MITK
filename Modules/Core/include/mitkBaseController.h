/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBaseController_h
#define mitkBaseController_h

#include <mitkEventStateMachine.h>
#include <mitkOperationActor.h>
#include <mitkStepper.h>
#include <MitkCoreExports.h>
#include <itkObjectFactory.h>

namespace mitk
{
  class BaseRenderer;

  /**
   * \brief Base class for renderer slice, camera, and time control.
   *
   * Tells the renderer (subclass of BaseRenderer) which slice
   * (SliceNavigationController), which camera direction (CameraController),
   * or which time point (TimeNavigationController) to render. Contains a
   * Stepper for stepping through slices, camera views, or time steps.
   *
   * \sa SliceNavigationController
   * \sa CameraController
   * \sa TimeNavigationController
   * \sa Stepper
   * \sa BaseRenderer
   */
  class MITKCORE_EXPORT BaseController : public OperationActor, public itk::Object
  {
  public:

    mitkClassMacroItkParent(BaseController, OperationActor);
    itkFactorylessNewMacro(Self);

    /**
     * \brief Get the stepper for stepping through the controlled dimension.
     * \return Pointer to the mutable Stepper.
     */
    Stepper* GetStepper();

    /**
     * \brief Get the stepper for stepping through the controlled dimension (const).
     * \return Pointer to the const Stepper.
     */
    const Stepper* GetStepper() const;

  protected:
    /** \brief Default constructor. */
    BaseController();

    /** \brief Destructor. */
    ~BaseController() override;

    /**
     * \brief Execute an undo/redo operation.
     */
    void ExecuteOperation(Operation *) override;

    Stepper::Pointer m_Stepper; ///< Stepper through the controlled dimension.

    unsigned long m_LastUpdateTime; ///< Timestamp of the last update.
  };

} // namespace mitk

#endif
