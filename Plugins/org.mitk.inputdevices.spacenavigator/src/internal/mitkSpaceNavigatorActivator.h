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

#ifndef MITK_SPACENAVIGATORACTIVATOR_H_
#define MITK_SPACENAVIGATORACTIVATOR_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <vector>

#include <mitkIInputDevice.h>
#include <mitkSpaceNavigatorVtkCameraController.h>

namespace mitk
{
  /**
  * Documentation in the interface.
  *
  * @ingroup org_mitk_inputdevices_spacenavigator
  */
  class SpaceNavigatorActivator : public IInputDevice
  {

  public:

    /**
    * @see mitk::IInputDevice::RegisterInputDevice()
    */
    bool RegisterInputDevice();

    /**
    * @see mitk::IInputDevice::UnRegisterInputDevice()
    */
    bool UnRegisterInputDevice();

    /**
    * Default constructor
    */
    SpaceNavigatorActivator();

  protected:

  private:

    bool m_IsRegistered;
    mitk::SpaceNavigatorVtkCameraController::Pointer m_Controller;

  }; // end class SpaceNavigatorActivator
} // end namespace mitk

#endif /* MITK_SPACENAVIGATORACTIVATOR_H_ */
