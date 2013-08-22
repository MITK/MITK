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

#ifndef MITKUSTelemedDevice_H_HEADER_INCLUDED_
#define MITKUSTelemedDevice_H_HEADER_INCLUDED_

#include "mitkUSDevice.h"
#include "mitkUSTelemedImageSource.h"
#include "mitkUSTelemedScanConverterPlugin.h"
#include "mitkUSTelemedProbesControls.h"
#include "mitkUSTelemedBModeControls.h"
#include "mitkUSTelemedDopplerControls.h"

#include "mitkUSTelemedSDKHeader.h"

namespace mitk {

  class USTelemedDevice : public USDevice
  {
  public:
    mitkClassMacro(USTelemedDevice, mitk::USDevice);
    mitkNewMacro2Param(Self, std::string, std::string);

    /**
    * \brief Returns the Class of the Device. This Method must be reimplemented by every Inheriting Class.
    */
    virtual std::string GetDeviceClass();

    virtual USControlInterfaceBMode::Pointer GetControlInterfaceBMode();
    virtual USControlInterfaceProbes::Pointer GetControlInterfaceProbes();
    virtual USControlInterfaceDoppler::Pointer GetControlInterfaceDoppler();

    /**
    * \brief Is called during the initialization process.
    *  Returns true if successful and false if unsuccessful. Additionally, you may throw an exception to clarify what went wrong.
    */
    virtual bool OnInitialization();

    /**
    * \brief Is called during the connection process. Override this method in your subclass to handle the actual connection.
    *  Return true if successful and false if unsuccessful. Additionally, you may throw an exception to clarify what went wrong.
    */
    virtual bool OnConnection();

    /**
    * \brief Is called during the disconnection process. Override this method in your subclass to handle the actual disconnection.
    *  Return true if successful and false if unsuccessful. Additionally, you may throw an exception to clarify what went wrong.
    */
    virtual bool OnDisconnection();

    /**
    * \brief Is called during the activation process. After this method is finished, the device should be generating images
    */
    virtual bool OnActivation();


    /**
    * \brief Is called during the deactivation process. After a call to this method the device should still be connected, but not producing images anymore.
    */
    virtual bool OnDeactivation();

    /**
    *  \brief Grabs the next frame from the Video input. This method is called internally, whenever Update() is invoked by an Output.
    */
    void GenerateData();

    IUsgfw2* GetUsgMainInterface();
    void SetActiveDataView(IUsgDataView*);
    //void SetActiveProbe(IProbe*);

  protected:
    USTelemedDevice(std::string manufacturer, std::string model);
    virtual ~USTelemedDevice();

    USImageSource::Pointer GetUSImageSource( );

    void ReleaseUsgControls( );
    /*bool CreateProbesCollection( );
    bool SelectProbe(int index);*/
    bool StopScanning( );

    USTelemedProbesControls::Pointer    m_ControlsProbes;
    USTelemedBModeControls::Pointer     m_ControlsBMode;
    USTelemedDopplerControls::Pointer   m_ControlsDoppler;

    USTelemedImageSource::Pointer   m_ImageSource;

    IUsgfw2*                        m_UsgMainInterface;
    IProbe*                         m_Probe;
    IUsgDataView*                   m_UsgDataView;
    IUsgCollection*                 m_ProbesCollection;
  };

}

#endif // MITKUSTelemedDevice_H_HEADER_INCLUDED_
