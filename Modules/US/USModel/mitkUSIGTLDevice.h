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

#ifndef MITKIGTLDevice_H_HEADER_INCLUDED_
#define MITKIGTLDevice_H_HEADER_INCLUDED_

#include <mitkCommon.h>
#include <MitkUSExports.h>
#include <mitkUSDevice.h>
#include <mitkUSProbe.h>
#include <mitkIGTLClient.h>
#include <mitkIGTLDeviceSource.h>
#include <mitkIGTL2DImageDeviceSource.h>
#include <mitkIGTLTrackingDataDeviceSource.h>
#include <mitkIGTLMessageToUSImageFilter.h>
#include <mitkUSVideoDeviceCustomControls.h>


namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk
{
  class USVideoDeviceCustomControls;
  class USAbstractControlInterface;
  /**
  * \brief A mitk::USIGTLDevice is a USDevice to receive images over an OpenIGTLink
  * connection. It registers an OIGTL device as a Microservice to receive image messages
  * and transforms them to mitk::Images. It can act both as a server (listening for
  * incoming connections) and as a client (connecting to an existing OIGTL server).
  *
  * \ingroup US
  */
  class MITKUS_EXPORT USIGTLDevice : public mitk::USDevice
  {
  public:
    mitkClassMacro(USIGTLDevice, mitk::USDevice);
    // To open a device (Manufacturer, Model, Hostname, Port, IsServer)
    mitkNewMacro5Param(Self, std::string, std::string, std::string, int, bool);

    std::string GetDeviceClass() override;
    USImageSource::Pointer GetUSImageSource() override;

    USIGTLDevice(std::string manufacturer, std::string model, std::string host,
      int port, bool server);

    /**
    * Getter for the custom control interface which was created during the
    * construction process of mitk::USIGTLDevice.
    *
    * \return custom control interface of the video device
    */
    itk::SmartPointer<USAbstractControlInterface> GetControlInterfaceCustom() override;

    /**
    * \brief Remove the IGTLDevice from the micro service.
    */
    void UnregisterOnService();

    // Neu hinzugefügt:
    /**
    * \brief Return all probes for this USVideoDevice or an empty vector it no probes were set
    * Returns a std::vector of all probes that exist for this USVideoDevice if there were probes set while creating or modifying this USVideoDevice.
    * Otherwise it returns an empty vector. Therefore always check if vector is filled, before using it!
    */
    virtual std::vector<mitk::USProbe::Pointer> GetAllProbes() override;

    /**
    * \brief Cleans the std::vector containing all configured probes.
    */
    virtual void DeleteAllProbes() override;

    /**
    * \brief Return current active probe for this USDevice
    * Returns a pointer to the probe that is currently in use. If there were probes set while creating or modifying this USDevice.
    * Returns null otherwise
    */
    virtual mitk::USProbe::Pointer GetCurrentProbe() override;

    /**
    \brief adds a new probe to the device
    */
    virtual void AddNewProbe(mitk::USProbe::Pointer probe) override;

    /**
    * \brief get the probe by its name
    * Returns a  pointer to the probe identified by the given name. If no probe of given name exists for this Device 0 is returned.
    */
    virtual mitk::USProbe::Pointer GetProbeByName(std::string name) override;

    /**
    * \brief Removes the Probe with the given name
    */
    virtual void RemoveProbeByName(std::string name) override;

    /**
    * \brief Sets the first existing probe or the default probe of the video device
    * as the current probe of it.
    */
    virtual void SetDefaultProbeAsCurrentProbe() override;

    /**
    * \brief Sets the probe with the given name as current probe if the named probe exists.
    */
    virtual void SetCurrentProbe(std::string probename) override;

    /**
    * \brief Sets the given spacing of the current depth of the current probe.
    */
    void SetSpacing(double xSpacing, double ySpacing) override;


  protected:
    bool OnInitialization() override;
    bool OnConnection() override;
    bool OnDisconnection() override;
    bool OnActivation() override;
    bool OnDeactivation() override;

    /**
    * \brief Grabs the next frame from the Video input.
    * This method is called internally, whenever Update() is invoked by an Output.
    */
    virtual void GenerateData() override;

  private:
    std::string m_Host;
    int m_Port;
    mitk::IGTLDevice::Pointer m_Device;
    mitk::IGTL2DImageDeviceSource::Pointer m_DeviceSource;
    mitk::IGTLTrackingDataDeviceSource::Pointer m_TransformDeviceSource;
    mitk::IGTLMessageToUSImageFilter::Pointer m_Filter;

    /**
    * \brief custom control interface for us video device
    */
    itk::SmartPointer<USVideoDeviceCustomControls> m_ControlInterfaceCustom;

    /**
    * \brief probes for this USVideoDevice
    */
    std::vector < mitk::USProbe::Pointer > m_Probes;

    /**
    \brief probe that is currently in use
    */
    mitk::USProbe::Pointer m_CurrentProbe;
  };
}  // namespace mitk

#endif  // MITKIGTLDevice_H_HEADER_INCLUDED_
