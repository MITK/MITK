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

#ifndef MITKUSCombinedModality_H_HEADER_INCLUDED_
#define MITKUSCombinedModality_H_HEADER_INCLUDED_

#include <MitkUSCombinedModalityExports.h>

#include "mitkUSDevice.h"

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk {
class NavigationDataSource;

class MitkUSCombinedModality_EXPORT USCombinedModality : public USDevice
{
public:
    static const std::string DeviceClassIdentifier;

    mitkClassMacro(USCombinedModality, USDevice);
    mitkNewMacro2Param(USCombinedModality, std::string, std::string);

    itkGetMacro(UltrasoundDevice, itk::SmartPointer<USDevice>);
    itkGetMacro(TrackingDevice, itk::SmartPointer<NavigationDataSource>);
    itkGetMacro(Calibration, AffineTransform3D::Pointer);

    itkSetMacro(UltrasoundDevice, itk::SmartPointer<USDevice>);
    itkSetMacro(TrackingDevice, itk::SmartPointer<NavigationDataSource>);
    itkSetMacro(Calibration, AffineTransform3D::Pointer);

    /**
      * \brief Returns the Class of the Device.
      */
    virtual std::string GetDeviceClass();

    /**
      * \brief Wrapper for returning USImageSource of the UltrasoundDevice.
      */
    virtual USImageSource::Pointer GetUSImageSource();

    /**
      * \brief Wrapper for returning B mode control interface of the UltrasoundDevice.
      */
    virtual USControlInterfaceBMode::Pointer GetControlInterfaceBMode();

    /**
      * \brief Wrapper for returning probes control interface of the UltrasoundDevice.
      */
    virtual USControlInterfaceProbes::Pointer GetControlInterfaceProbes();

    /**
      * \brief Wrapper for returning doppler control interface of the UltrasoundDevice.
      */
    virtual USControlInterfaceDoppler::Pointer GetControlInterfaceDoppler();

protected:
    USCombinedModality(std::string manufacturer, std::string model);
    virtual ~USCombinedModality();

    /**
      * \brief Initializes UltrasoundDevice.
      */
    virtual bool OnInitialization();

    /**
      * \brief Connects UltrasoundDevice.
      */
    virtual bool OnConnection();

    /**
      * \brief Disconnects UltrasoundDevice.
      */
    virtual bool OnDisconnection();

    /**
      * \brief Activates UltrasoundDevice.
      */
    virtual bool OnActivation();

    /**
      * \brief Deactivates UltrasoundDevice.
      */
    virtual bool OnDeactivation();

    /**
      * \brief Grabs the next frame from the input.
      * This method is called internally, whenever Update() is invoked by an Output.
      */
    void GenerateData();

    USDevice::Pointer                       m_UltrasoundDevice;
    itk::SmartPointer<NavigationDataSource> m_TrackingDevice;
    AffineTransform3D::Pointer              m_Calibration;

private:
    /**
    *  \brief The device's ServiceRegistration object that allows to modify it's Microservice registraton details.
    */
    us::ServiceRegistration<Self>           m_ServiceRegistration;

    /**
    * \brief Properties of the device's Microservice.
    */
    us::ServiceProperties                   m_ServiceProperties;
};

} // namespace mitk

#endif // MITKUSCombinedModality_H_HEADER_INCLUDED_
