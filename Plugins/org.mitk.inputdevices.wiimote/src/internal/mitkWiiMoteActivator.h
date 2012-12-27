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

#ifndef MITK_WIIMOTEACTIVATOR_H_
#define MITK_WIIMOTEACTIVATOR_H_

// berry
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

// mitk
#include "mitkIInputDevice.h"
#include "mitkWiiMoteVtkCameraController.h"
#include <mitkWiiMoteInteractor.h>
#include <mitkDataStorage.h>

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

    // listener for headtracking and interactor for surface interaction
    mitk::WiiMoteVtkCameraController::Pointer m_Controller;
    mitk::WiiMoteInteractor::Pointer m_Interactor;

    // for xml accesss
    berry::IPreferences::Pointer m_WiiMotePreferencesNode;
    berry::IPreferencesService::Pointer m_PrefService;

    // needed for surface interaction
    mitk::DataNode::Pointer m_Node;

    /**
    * Gets the current active data storage. <br>
    *
    * @return the current data storage, if it fails <code>NULL</code>
    */
    mitk::DataStorage::Pointer GetDataStorage();

    void AddSurfaceInteractor();
    void RemoveSurfaceInteractor();


  }; // end class WiiMoteActivator
} // end namespace mitk

#endif // MITK_WIIMOTEACTIVATOR_H_
