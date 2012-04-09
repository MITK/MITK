/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
