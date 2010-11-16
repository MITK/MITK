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

#ifndef MITK_WIIMOTEACTIVATOR_H_
#define MITK_WIIMOTEACTIVATOR_H_

#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include "mitkIInputDevice.h"

#include "mitkWiiMoteVtkCameraController.h"

namespace mitk
{
  class WiiMoteActivator : public IInputDevice
  {
  public:

    WiiMoteActivator();
    ~WiiMoteActivator();

    /**
    * @see mitk::IInputDevice::RegisterInputDevice()
    */
    bool RegisterInputDevice();

    /**
    * @see mitk::IInputDevice::UnRegisterInputDevice()
    */
    bool UnRegisterInputDevice();

  protected:

  private:

    bool m_IsRegistered;

    mitk::WiiMoteVtkCameraController::Pointer m_Controller;

    berry::IPreferences::Pointer m_WiiMotePreferencesNode;
     berry::IPreferencesService::Pointer m_PrefService;

  }; // end class WiiMoteActivator
} // end namespace mitk

#endif // MITK_WIIMOTEACTIVATOR_H_
