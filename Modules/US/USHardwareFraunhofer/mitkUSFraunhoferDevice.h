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

#ifndef MITKUSFraunhoferDevice_H_HEADER_INCLUDED_
#define MITKUSFraunhoferDevice_H_HEADER_INCLUDED_

#include "mitkUSDevice.h"
#include "mitkUSFraunhoferImageSource.h"
#include "mitkUSFraunhoferProbesControls.h"
#include "mitkUSFraunhoferBModeControls.h"
#include "mitkUSFraunhoferDopplerControls.h"

#include "mitkUSFraunhoferSDKHeader.h"

namespace mitk {
  /**
    * \brief Implementation of mitk::USDevice for Fraunhofer API devices.
    * Connects to a Fraunhofer API device through its COM library interface.
    *
    * This class handles all API communications and creates interfaces for
    * b mode, doppler and probes controls.
    * Images given by the device are put into an object of
    * mitk::USFraunhoferImageSource.
    */
  class USFraunhoferDevice : public USDevice
  {
  public:
    mitkClassMacro(USFraunhoferDevice, mitk::USDevice);
    mitkNewMacro2Param(Self, std::string, std::string);

    /**
    * \brief Returns the class of the device.
    */
    virtual std::string GetDeviceClass();

    virtual USControlInterfaceBMode::Pointer GetControlInterfaceBMode();
    virtual USControlInterfaceProbes::Pointer GetControlInterfaceProbes();
    virtual USControlInterfaceDoppler::Pointer GetControlInterfaceDoppler();

    /**
      * \brief Is called during the initialization process.
      * There is nothing done on the initialization of a mik::USFraunhoferDevive object.
      *
      * \return always true
      */
    virtual bool OnInitialization();

    /**
      * \brief Is called during the connection process.
      * Connect to the Fraunhofer API.
      *
      * \return true if successfull, false if no device is connected to the pc
      * \throws mitk::Exception if something goes wrong at the API calls
      */
    virtual bool OnConnection();

    /**
      * \brief Is called during the disconnection process.
      * Deactivate and remove all Fraunhofer API controls. A disconnect from the
      * Fraunhofer API is not possible for which reason the hardware stays in connected
      * state even after calling this method.
      *
      * \return always true
      * \throws mitk::Exception if something goes wrong at the API calls
      */
    virtual bool OnDisconnection();

    /**
      * \brief Is called during the activation process.
      * After this method is finished, the device is generating images in b mode.
      * Changing scanning mode is possible afterwards by using the appropriate
      * control interfaces.
      *
      * \return always true
      * \throws mitk::Exception if something goes wrong at the API calls
      */
    virtual bool OnActivation();

    /**
      * \brief Is called during the deactivation process.
      * After a call to this method the device is connected, but not producing images anymore.
      *
      * \return always true
      * \throws mitk::Exception if something goes wrong at the API calls
      */
    virtual bool OnDeactivation();

    /**
      * \brief Changes scan state of the device if freeze is toggeled in mitk::USDevice.
      */
    virtual void OnFreeze(bool freeze);

    /** @return Returns the current image source of this device. */
    USImageSource::Pointer GetUSImageSource( );

  protected:
    /**
      * Constructs a mitk::USFraunhoferDevice object by given manufacturer
      * and model string. These strings are just for labeling the device
      * in the micro service.
      *
      * Control interfaces and image source are available directly after
      * construction. Registration at the micro service happens not before
      * initialization method was called.
      */
    USFraunhoferDevice(std::string manufacturer, std::string model);
    virtual ~USFraunhoferDevice();

    void ReleaseUsgControls( );

    /**
      * \brief Stop ultrasound scanning by Fraunhofer API call.
      *
      * \throw mitk::Exception if API call returned with an error
      */
    void StopScanning( );

    /*USFraunhoferProbesControls::Pointer    m_ControlsProbes;
    USFraunhoferBModeControls::Pointer     m_ControlsBMode;
    USFraunhoferDopplerControls::Pointer   m_ControlsDoppler;

    USFraunhoferImageSource::Pointer       m_ImageSource;*/

    /**
    * The Fraunhofer API expects callback functions to pass 
    * both status messages and the processed images to the user.
    * The message callback is here, the data itself is given directly to the image source.
    */
    void stringMessageCallback(char* message);
  };
} // namespace mitk

#endif // MITKUSFraunhoferDevice_H_HEADER_INCLUDED_