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

#ifndef __mitkAbstractUltrasoundTrackerDevice_h
#define __mitkAbstractUltrasoundTrackerDevice_h

#include <MitkUSNavigationExports.h>
#include "mitkUSDevice.h"
#include "mitkImageSource.h"
#include "mitkNavigationDataSource.h"

// Microservices
#include <mitkServiceInterface.h>
#include <usServiceRegistration.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk {
  class NavigationDataSmoothingFilter;
  class NavigationDataDelayFilter;
  class NavigationDataDisplacementFilter;

  /**
  * \brief Abstract class for an easy handling of a combination of an USDevice and
  * a NavigationDataSource.
  * This class can be used as an ImageSource subclass. Additionally tracking data be
  * retrieved from the NavigationDataSource returned by GetTrackingDevice().
  *
  * A calibration of the ultrasound image stream to the navigation datas can be set
  * for the currently active zoom level (of the ultrasound device) by SetCalibration()
  * The ultrasound images are transformed according to this calibration in the
  * GenerateData() method.
  */

  class MITKUSNAVIGATION_EXPORT AbstractUltrasoundTrackerDevice : public mitk::ImageSource
  {
  public:

    static const std::string DeviceClassIdentifier;
    static const char*       DefaultProbeIdentifier;
    static const char*       ProbeAndDepthSeperator;
    static const std::string US_INTERFACE_NAME;
    static const std::string US_PROPKEY_DEVICENAME;
    static const std::string US_PROPKEY_CLASS;
    static const std::string US_PROPKEY_ID;


    mitkClassMacro(AbstractUltrasoundTrackerDevice, mitk::ImageSource);
    mitkNewMacro3Param(AbstractUltrasoundTrackerDevice, USDevice::Pointer, itk::SmartPointer<NavigationDataSource>, bool);

    itkGetMacro(UltrasoundDevice, itk::SmartPointer<USDevice>);
    itkSetMacro(UltrasoundDevice, itk::SmartPointer<USDevice>);
    itkGetMacro(TrackingDeviceDataSource, itk::SmartPointer<NavigationDataSource>);
    itkSetMacro(TrackingDeviceDataSource, itk::SmartPointer<NavigationDataSource>);

    itkGetMacro(IsTrackedUltrasoundActive, bool);

    /**
    * \brief Getter for calibration data of the currently active depth and probe.
    *
    * \return Transformation for calibration or null if no calibration is available.
    */
    AffineTransform3D::Pointer GetCalibration();

    /**
    * \brief Getter for calibration data of the given depth and the currently active probe.
    *
    * \param depth depth of the b mode ultrasound image for which the calibration should be returned
    * \return Transformation for calibration or null if no calibration is available.
    */
    AffineTransform3D::Pointer GetCalibration(std::string depth);

    /**
    * \brief Getter for calibration data of the given depth and probe.
    *
    * \param depth depth of the b mode ultrasound image for which the calibration should be returned
    * \param probe probe of the ultrasound device for which the calibration should be returned
    * \return Transformation for calibration or null if no calibration is available.
    */
    AffineTransform3D::Pointer GetCalibration(std::string depth, std::string probe);

    /** Returns the index to world transform of the US plane. */
    virtual AffineTransform3D::Pointer GetUSPlaneTransform();

    /**
    * \brief Sets a transformation as calibration data.
    * Calibration data is set for the currently activated probe and their current
    * zoom factor. It also marks the device as calibrated.
    */
    void SetCalibration(AffineTransform3D::Pointer calibration);

    /**
    * \brief Removes the calibration data of the currently active depth and probe.
    * \return true on success, false if there was no calibration
    */
    bool RemoveCalibration();

    /**
    * \brief Removes the calibration data of the given depth and the currently active probe.
    *
    * \param depth depth of the b mode ultrasound image for which the calibration should be removed
    * \return true on success, false if there was no calibration
    */
    bool RemoveCalibration(std::string depth);

    /**
    * \brief Removes the calibration data of the given depth and probe.
    *
    * \param depth depth of the b mode ultrasound image for which the calibration should be removed
    * \param probe probe of the ultrasound device for which the calibration should be removed
    * \return true on success, false if there was no calibration
    */
    bool RemoveCalibration(std::string depth, std::string probe);

    /**
    * \brief Returns the Class of the Device.
    */
    std::string GetDeviceClass();

    /**
    * \brief Wrapper for returning USImageSource of the UltrasoundDevice.
    */
    USImageSource::Pointer GetUSImageSource();

    itk::SmartPointer<mitk::NavigationDataSource> GetNavigationDataSource();

    /**
    * \return true if the device is calibrated for the currently selected probe with the current zoom level
    */
    bool GetIsCalibratedForCurrentStatus();

    /**
    * \return true if a calibration was loaded for at least one probe and depth
    */
    bool GetContainsAtLeastOneCalibration();

    /**
    * \brief Serializes all contained calibrations into an xml fragment.
    *
    * The returned string contains one parent node named "calibrations" and several
    * subnodes, one for each calibration that is present.
    */
    std::string SerializeCalibration();

    /**
    * \brief Deserializes a string provided by a prior call to Serialize().
    * If the bool flag is true, all prior calibrations will be deleted.
    * If the flag is set to false, prior calibrations will be retained, but overwritten
    * if one of equal name is present.
    *
    * \throws mitk::Exception if the given string could not be parsed correctly.
    */
    void DeserializeCalibration(const std::string &xmlString, bool clearPreviousCalibrations = true);

    void SetNumberOfSmoothingValues(unsigned int numberOfSmoothingValues);

    void SetDelayCount(unsigned int delayCount);

    /**
    * \brief Remove this device from the micro service.
    * This method is public for mitk::USCombinedModality, because this devices
    * can be completly removed. This is not possible for API devices, which
    * should be available while their sub module is loaded.
    */
    void UnregisterOnService();

    virtual void RegisterAsMicroservice();

    /**
    * \brief Wrapper for returning custom control interface of the UltrasoundDevice.
    */
    virtual itk::SmartPointer<USAbstractControlInterface> GetControlInterfaceCustom();

    /**
    * \brief Wrapper for returning B mode control interface of the UltrasoundDevice.
    */
    virtual itk::SmartPointer<USControlInterfaceBMode> GetControlInterfaceBMode();

    /**
    * \brief Wrapper for returning probes control interface of the UltrasoundDevice.
    */
    virtual itk::SmartPointer<USControlInterfaceProbes> GetControlInterfaceProbes();

    /**
    * \brief Wrapper for returning doppler control interface of the UltrasoundDevice.
    */
    virtual itk::SmartPointer<USControlInterfaceDoppler> GetControlInterfaceDoppler();

    /**
    * \brief Can toggle if the combined modality is currently updated or freezed.
    *
    * \param freeze true to stop updating the ultrasound image and the tracking data, false to start updating again
    */
    virtual void SetIsFreezed(bool freeze);

    /**
    * \return true if device is currently freezed (no image/tracking data update is done), false otherwise
    */
    virtual bool GetIsFreezed();

    /**
    * \brief Called when mitk::AbstractUltrasoundTrackerDevice::SetIsFreezed() is called.
    * Subclasses can overwrite this method to do additional actions. Default
    * implementation does noting.
    */
    virtual void OnFreeze(bool) { }

  protected:
    AbstractUltrasoundTrackerDevice( USDevice::Pointer usDevice,
                                     itk::SmartPointer<NavigationDataSource> trackingDevice,
                                     bool trackedUltrasoundActive );
    virtual ~AbstractUltrasoundTrackerDevice();


    /**
    * \brief Grabs the next frame from the input.
    * Must be implemented by the derived class.
    * This method is called internally, whenever Update() is invoked by an Output.
    */
    virtual void GenerateData() override;

    std::string GetIdentifierForCurrentCalibration();
    std::string GetIdentifierForCurrentProbe();
    std::string GetCurrentDepthValue();

    void RebuildFilterPipeline();

    USDevice::Pointer                                      m_UltrasoundDevice;
    itk::SmartPointer<NavigationDataSource>                m_TrackingDeviceDataSource;
    std::map<std::string, AffineTransform3D::Pointer>      m_Calibrations;

    itk::SmartPointer<mitk::NavigationDataSmoothingFilter> m_SmoothingFilter;
    itk::SmartPointer<mitk::NavigationDataDelayFilter>     m_DelayFilter;
    itk::SmartPointer<mitk::NavigationDataDisplacementFilter> m_DisplacementFilter;
    itk::SmartPointer<mitk::NavigationDataSource> m_LastFilterOfIGTPipeline;

    unsigned int m_NumberOfSmoothingValues;
    unsigned int m_DelayCount;

    /**
    *  \brief The device's ServiceRegistration object that allows to modify it's Microservice registraton details.
    */
    us::ServiceRegistration<Self>           m_ServiceRegistration;

    /**
    * \brief Properties of the device's Microservice.
    */
    us::ServiceProperties                   m_ServiceProperties;

  private:

    bool m_IsFreezed;
    bool m_IsTrackedUltrasoundActive;
  };

} // namespace mitk

MITK_DECLARE_SERVICE_INTERFACE(mitk::AbstractUltrasoundTrackerDevice, "org.mitk.services.AbstractUltrasoundTrackerDevice")

#endif
