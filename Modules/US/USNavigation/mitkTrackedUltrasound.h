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

#ifndef __mitkTrackedUltrasound_h
#define __mitkTrackedUltrasound_h

#include <MitkUSNavigationExports.h>
#include "mitkUSDevice.h"
#include "mitkImageSource.h"
#include "mitkAbstractUltrasoundTrackerDevice.h"
#include "mitkNavigationDataSource.h"

// Microservices
#include <mitkServiceInterface.h>
#include <usServiceRegistration.h>

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk {
  class USControlInterfaceBMode;
  class USControlInterfaceProbes;
  class USControlInterfaceDoppler;

  /**
   * \brief Combination of USDevice and NavigationDataSource.
   * This class can be used as an ImageSource subclass. Additionally tracking data be
   * retrieved from the NavigationDataSource returned by GetTrackingDevice().
   *
   * A calibration of the ultrasound image stream to the navigation datas can be set
   * for the currently active zoom level (of the ultrasound device) by SetCalibration().
   * The ultrasound images are transformed according to this calibration in the
   * GenerateData() method.
   */
  class MITKUSNAVIGATION_EXPORT TrackedUltrasound : public mitk::AbstractUltrasoundTrackerDevice
  {
  public:

    //void UnregisterOnService();

    //void RegisterAsMicroservice() override;

    mitkClassMacro(TrackedUltrasound, mitk::AbstractUltrasoundTrackerDevice);
    mitkNewMacro3Param(TrackedUltrasound, USDevice::Pointer, itk::SmartPointer<NavigationDataSource>, bool);


    /**
    * \brief Wrapper for returning custom control interface of the UltrasoundDevice.
    */
    virtual itk::SmartPointer<USAbstractControlInterface> GetControlInterfaceCustom() override;

    /**
    * \brief Wrapper for returning B mode control interface of the UltrasoundDevice.
    */
    virtual itk::SmartPointer<USControlInterfaceBMode> GetControlInterfaceBMode() override;

    /**
    * \brief Wrapper for returning probes control interface of the UltrasoundDevice.
    */
    virtual itk::SmartPointer<USControlInterfaceProbes> GetControlInterfaceProbes() override;

    /**
    * \brief Wrapper for returning doppler control interface of the UltrasoundDevice.
    */
    virtual itk::SmartPointer<USControlInterfaceDoppler> GetControlInterfaceDoppler() override;

  protected:
    TrackedUltrasound(  USDevice::Pointer usDevice,
                        itk::SmartPointer<NavigationDataSource> trackingDevice,
                        bool trackedUltrasoundActive = true );
    virtual ~TrackedUltrasound();

    /**
    * \brief Grabs the next frame from the input.
    * This method is called internally, whenever Update() is invoked by an Output.
    */
    void GenerateData() override;


    private:
      /**
      *  \brief The device's ServiceRegistration object that allows to modify it's Microservice registraton details.
      */
      //us::ServiceRegistration<Self>           m_ServiceRegistration;

      /**
      * \brief Properties of the device's Microservice.
      */
      //us::ServiceProperties                   m_ServiceProperties;
  };
} // namespace mitk
#endif // __mitkTrackedUltrasound_h
