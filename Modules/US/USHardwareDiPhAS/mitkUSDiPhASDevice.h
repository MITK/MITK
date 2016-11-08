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

#ifndef MITKUSDiPhASDevice_H_HEADER_INCLUDED_
#define MITKUSDiPhASDevice_H_HEADER_INCLUDED_

#include <functional>
#include "mitkUSDiPhASImageSource.h"
#include "mitkUSDevice.h"
#include "mitkUSDiPhASProbesControls.h"
#include "mitkUSDiPhASImageSource.h"
#include "mitkUSDiPhASCustomControls.h"

#include "Framework.IBMT.US.CWrapper.h"

#include <Windows.h>
#include <iostream>
#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>

#include <itkObjectFactory.h>


namespace mitk {
  /**
    * \brief Implementation of mitk::USDevice for DiPhAS API devices.
    * Connects to a DiPhAS API device through its COM library interface.
    *
    * This class handles all API communications and creates interfaces for
    * b mode, doppler and probes controls.
    * Images given by the device are put into an object of
    * mitk::USDiPhASImageSource.
    */
  class USDiPhASImageSource;
	
  class USDiPhASDevice : public USDevice
  {
  public:
    mitkClassMacro(USDiPhASDevice, mitk::USDevice);
    mitkNewMacro2Param(Self, std::string, std::string);

    /**
    * \brief Returns the class of the device.
    */
    virtual std::string GetDeviceClass();

    virtual USControlInterfaceProbes::Pointer GetControlInterfaceProbes();
    virtual itk::SmartPointer<USAbstractControlInterface> GetControlInterfaceCustom();

    /**
      * \brief Is called during the initialization process.
      * There is nothing done on the initialization of a mik::USDiPhASDevive object.
      *
      * \return always true
      */
    virtual bool OnInitialization();

    /**
      * \brief Is called during the connection process.
      * Connect to the DiPhAS API.
      *
      * \return true if successfull, false if no device is connected to the pc
      * \throws mitk::Exception if something goes wrong at the API calls
      */
    virtual bool OnConnection();

    /**
      * \brief Is called during the disconnection process.
      * Deactivate and remove all DiPhAS API controls. A disconnect from the
      * DiPhAS API is not possible for which reason the hardware stays in connected
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

    /** @return Returns the currently used scanmode of this device*/
    ScanModeNative& GetScanMode();

    /** Updates the Scanmode and feeds it to the hardware
      */
    void UpdateScanmode();
    /** This method forwards messages from the API to the user*/
    void MessageCallback(const char* message);
    void SetBursts(int bursts);
    void SetInterleaved(bool interleaved);
    bool IsInterleaved();

    BeamformingParametersInterleaved_OA_US paramsInterleaved;
    BeamformingParametersPlaneWaveCompound paramsPlaneWave;

  protected:
    /**
      * Constructs a mitk::USDiPhASDevice object by given manufacturer
      * and model string. These strings are just for labeling the device
      * in the micro service.
      *
      * Control interfaces and image source are available directly after
      * construction. Registration at the micro service happens not before
      * initialization method was called.
      */
    USDiPhASDevice(std::string manufacturer, std::string model);
    virtual ~USDiPhASDevice();

    /**
    * The DiPhAS API expects callback functions to pass 
    * both status messages and the processed images to the user.
    * The message callback is here, the data itself is given directly to the image source.
    */

    /**
    * This method sets up the scanmode at the begining
    */
    void InitializeScanMode();
    void UpdateTransmitEvents();

    USDiPhASProbesControls::Pointer                m_ControlsProbes;
    itk::SmartPointer<USAbstractControlInterface>  m_ControlInterfaceCustom;

    mitk::USDiPhASImageSource::Pointer                   m_ImageSource;

    bool                                           m_IsRunning;
    ScanModeNative                                 m_ScanMode;
    int                                            m_BurstHalfwaveClockCount;
    Beamforming                                    m_CurrentBeamformingAlgorithm;
    bool                                           m_Interleaved;
  };
} // namespace mitk

#endif // MITKUSDiPhASDevice_H_HEADER_INCLUDED_