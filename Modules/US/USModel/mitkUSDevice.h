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

#ifndef MITKUSDevice_H_HEADER_INCLUDED_
#define MITKUSDevice_H_HEADER_INCLUDED_

// STL
#include <vector>

// MitkUS
#include "mitkUSProbe.h"
#include "mitkUSImageMetadata.h"
#include <MitkUSExports.h>
#include "mitkUSImageSource.h"

// MITK
#include <mitkCommon.h>
#include <mitkImageSource.h>

// ITK
#include <itkObjectFactory.h>
#include <itkConditionVariable.h>

// Microservices
#include <usServiceInterface.h>
#include <usServiceRegistration.h>
#include <usServiceProperties.h>

namespace itk {
template<class T> class SmartPointer;
}

namespace mitk {

class USAbstractControlInterface;
class USControlInterfaceBMode;
class USControlInterfaceProbes;
class USControlInterfaceDoppler;

  /**
  * \brief A device holds information about it's model, make and the connected probes. It is the
  * common super class for all devices and acts as an image source for mitkUSImages. It is the base class
  * for all US Devices, and every new device should extend it.
  *
  * US Devices support output of calibrated images, i.e. images that include a specific geometry.
  * To achieve this, call SetCalibration, and make sure that the subclass also calls apply
  * transformation at some point (The USDevice does not automatically apply the transformation to the image)
  *
  * Note that USDevices will be removed from micro servive when their
  * destructor is called. Registering into micro service is done when
  * mitk::USDevice::Initialize() is called.
  *
  * \ingroup US
  */

  class MitkUS_EXPORT USDevice : public mitk::ImageSource
  {
  public:
    enum DeviceStates { State_NoState, State_Initialized, State_Connected, State_Activated };

    mitkClassMacro(USDevice, mitk::ImageSource);

    struct USImageCropArea
    {
      int cropLeft;
      int cropRight;
      int cropBottom;
      int cropTop;
    };

    /**
    *\brief These constants are used in conjunction with Microservices
    */
    static const std::string US_INTERFACE_NAME;       // Common Interface name of all US Devices. Used to refer to this device via Microservices
    static const std::string US_PROPKEY_LABEL;        // Human readable text represntation of this device
    static const std::string US_PROPKEY_ISCONNECTED;  // Whether this device is connected or not.
    static const std::string US_PROPKEY_ISACTIVE;     // Whether this device is active or not.
    static const std::string US_PROPKEY_CLASS;        // Class Name of this Object

    static const std::string US_PROPKEY_PROBES_SELECTED;

    static const std::string US_PROPKEY_BMODE_FREQUENCY;
    static const std::string US_PROPKEY_BMODE_POWER;
    static const std::string US_PROPKEY_BMODE_DEPTH;
    static const std::string US_PROPKEY_BMODE_GAIN;
    static const std::string US_PROPKEY_BMODE_REJECTION;

    /**
    * \brief Default getter for the custom control interface.
    * Has to be implemented in a subclass if a custom control interface is
    * available. Default implementation returns null.
    *
    * \return null pointer
    */
    virtual itk::SmartPointer<USAbstractControlInterface>   GetControlInterfaceCustom();

    /**
    * \brief Default getter for the b mode control interface.
    * Has to be implemented in a subclass if a b mode control interface is
    * available. Default implementation returns null.
    *
    * \return null pointer
    */
    virtual itk::SmartPointer<USControlInterfaceBMode>      GetControlInterfaceBMode();

    /**
    * \brief Default getter for the probes control interface.
    * Has to be implemented in a subclass if a probes control interface is
    * available. Default implementation returns null.
    *
    * \return null pointer
    */
    virtual itk::SmartPointer<USControlInterfaceProbes>     GetControlInterfaceProbes();

    /**
    * \brief Default getter for the doppler control interface.
    * Has to be implemented in a subclass if a doppler control interface is
    * available. Default implementation returns null.
    *
    * \return null pointer
    */
    virtual itk::SmartPointer<USControlInterfaceDoppler>    GetControlInterfaceDoppler();

    /**
    * \brief Changes device state to mitk::USDevice::State_Initialized.
    * During initialization the virtual method
    * mitk::USDevice::OnInitialization will be called. If this method
    * returns false the initialization process will be canceled. Otherwise
    * the mitk::USDevice is registered in a micro service.
    */
    bool Initialize();

    /**
    * \brief Connects this device. A connected device is ready to deliver images (i.e. be Activated). A Connected Device can be active. A disconnected Device cannot be active.
    *  Internally calls onConnect and then registers the device with the service. A device usually should
    *  override the OnConnection() method, but never the Connect() method, since this will possibly exclude the device
    *  from normal service management. The exact flow of events is:
    *  0. Check if the device is already connected. If yes, return true anyway, but don't do anything.
    *  1. Call OnConnection() Here, a device should establish it's connection with the hardware Afterwards, it should be ready to start transmitting images at any time.
    *  2. If OnConnection() returns true ("successful"), then the device is registered with the service.
    *  3. if not, it the method itself returns false or may throw an expection, depeneding on the device implementation.
    *
    */
    bool Connect();

    void ConnectAsynchron();

    /**
    * \brief Works analogously to mitk::USDevice::Connect(). Don't override this Method, but onDisconnection instead.
    */
    bool Disconnect();

    /**
    * \brief Activates this device.
    * After the activation process, the device will start to produce images.
    * This Method will fail, if the device is not connected.
    */
    bool Activate();

    /**
    * \brief Deactivates this device.
    * After the deactivation process, the device will no longer produce
    * images, but still be connected.
    */
    void Deactivate();

    /**
    * \brief Can toggle if ultrasound image is currently updated or freezed.
    *
    * \param freeze true to stop updating the ultrasound image, false to start updating again
    */
    virtual void SetIsFreezed(bool freeze);

    /**
    * \return true if device is currently freezed (no image update is done), false otherwise
    */
    virtual bool GetIsFreezed();

    void PushFilter(AbstractOpenCVImageFilter::Pointer filter);
    void PushFilterIfNotPushedBefore(AbstractOpenCVImageFilter::Pointer filter);
    bool RemoveFilter(AbstractOpenCVImageFilter::Pointer filter);

    /**
    * \brief Given property is updated in the device micro service.
    * This method is mainly for being used by the control interface
    * superclasses. You do not need to call it by yoursefs in your
    * concrete control interface classes.
    */
    void UpdateServiceProperty(std::string key, std::string value);
    void UpdateServiceProperty(std::string key, double value);
    void UpdateServiceProperty(std::string key, bool value);

    //########### GETTER & SETTER ##################//

    /**
    * \brief Returns the Class of the Device. This Method must be reimplemented by every Inheriting Class.
    */
    virtual std::string GetDeviceClass() = 0;

    /**
    * \brief True, if the device object is created and initialized, false otherwise.
    */
    bool GetIsInitialized();

    /**
    * \brief True, if the device is currently generating image data, false otherwise.
    */
    bool GetIsActive();

    /**
    * \brief True, if the device is currently ready to start transmitting image data or is already
    * transmitting image data. A disconnected device cannot be activated.
    */
    bool GetIsConnected();

    /* @return Returns the area that will be cropped from the US image. Is disabled / [0,0,0,0] by default. */
    mitk::USDevice::USImageCropArea GetCropArea();

    std::string GetDeviceManufacturer();
    std::string GetDeviceModel();
    std::string GetDeviceComment();

    itkGetMacro(DeviceState, DeviceStates);

    void GrabImage();

    virtual USImageSource::Pointer GetUSImageSource() = 0;

  protected:
    itkSetMacro(Image, mitk::Image::Pointer);
    itkSetMacro(SpawnAcquireThread, bool);
    itkGetMacro(SpawnAcquireThread, bool);

    static ITK_THREAD_RETURN_TYPE Acquire(void* pInfoStruct);
    static ITK_THREAD_RETURN_TYPE ConnectThread(void* pInfoStruct);

    mitk::Image::Pointer m_Image;
    mitk::Image::Pointer m_OutputImage;

    bool m_IsFreezed;

    DeviceStates m_DeviceState;

    /* @brief defines the area that should be cropped from the US image */
    USImageCropArea m_CropArea;

    /**
    * \brief This Method constructs the service properties which can later be used to
    *  register the object with the Microservices
    *  Return service properties
    */
    us::ServiceProperties ConstructServiceProperties();

    /**
    * \brief Remove this device from the micro service.
    */
    void UnregisterOnService();

    /**
    * \brief Is called during the initialization process.
    * Override this method in a subclass to handle the actual initialization.
    * If it returns false, the initialization process will be canceled.
    *
    * \return true if successful and false if unsuccessful
    * \throw mitk::Exception implementation may throw an exception to clarify what went wrong
    */
    virtual bool OnInitialization() = 0;

    /**
    * \brief Is called during the connection process.
    * Override this method in a subclass to handle the actual connection.
    * If it returns false, the connection process will be canceled.
    *
    * \return true if successful and false if unsuccessful
    * \throw mitk::Exception implementation may throw an exception to clarify what went wrong
    */
    virtual bool OnConnection() = 0;

    /**
    * \brief Is called during the disconnection process.
    * Override this method in a subclass to handle the actual disconnection.
    * If it returns false, the disconnection process will be canceled.
    *
    * \return true if successful and false if unsuccessful
    * \throw mitk::Exception implementation may throw an exception to clarify what went wrong
    */
    virtual bool OnDisconnection() = 0;

    /**
    * \brief Is called during the activation process.
    * After this method is finished, the device should be generating images.
    * If it returns false, the activation process will be canceled.
    *
    * \return true if successful and false if unsuccessful
    * \throw mitk::Exception implementation may throw an exception to clarify what went wrong
    */
    virtual bool OnActivation() = 0;

    /**
    * \brief Is called during the deactivation process.
    * After a call to this method the device should still be connected,
    * but not producing images anymore.
    *
    * \return true if successful and false if unsuccessful
    * \throw mitk::Exception implementation may throw an exception to clarify what went wrong
    */
    virtual bool OnDeactivation() = 0;

    /**
    * \brief Called when mitk::USDevice::SetIsFreezed() is called.
    * Subclasses can overwrite this method to do additional actions. Default
    * implementation does noting.
    */
    virtual void OnFreeze(bool) { }

    /**
    * \brief This metadata set is privately used to imprint USImages with Metadata later.
    * At instantiation time, it only contains Information about the Device.
    * At scan time, it integrates this data with the probe information and
    * imprints it on the produced images. This field is intentionally hidden
    * from outside interference.
    */
    mitk::USImageMetadata::Pointer m_Metadata;

    /**
    * \brief Enforces minimal Metadata to be set.
    */
    USDevice(std::string manufacturer, std::string model);

    /**
    * \brief Constructs a device with the given Metadata. Make sure the Metadata contains meaningful content!
    */
    USDevice(mitk::USImageMetadata::Pointer metadata);

    virtual ~USDevice();

    /**
    * \brief Grabs the next frame from the Video input.
    * This method is called internally, whenever Update() is invoked by an Output.
    */
    virtual void GenerateData();

    std::string GetServicePropertyLabel();

  private:

    bool m_SpawnAcquireThread;

    /**
    *  \brief The device's ServiceRegistration object that allows to modify it's Microservice registraton details.
    */
    us::ServiceRegistration<Self> m_ServiceRegistration;

    /**
    * \brief Properties of the device's Microservice.
    */
    us::ServiceProperties m_ServiceProperties;

    // Threading-Related
    itk::ConditionVariable::Pointer m_FreezeBarrier;
    itk::SimpleMutexLock        m_FreezeMutex;
    itk::MultiThreader::Pointer m_MultiThreader; ///< itk::MultiThreader used for thread handling
    itk::FastMutexLock::Pointer m_ImageMutex; ///< mutex for images provided by the image source
    int m_ThreadID; ///< ID of the started thread

    bool m_UnregisteringStarted;
  };
} // namespace mitk

// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::USDevice, "org.mitk.services.UltrasoundDevice")

#endif // MITKUSDevice_H_HEADER_INCLUDED_
