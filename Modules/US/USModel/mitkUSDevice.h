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
#include "mitkUSImage.h"
#include <MitkUSExports.h>

// MITK
#include <mitkCommon.h>
#include <mitkImageSource.h>

// ITK
#include <itkObjectFactory.h>

// Microservices
#include <usServiceInterface.h>
#include <usServiceRegistration.h>
#include <usServiceProperties.h>


namespace mitk {

    /**Documentation
    * \brief A device holds information about it's model, make and the connected probes. It is the
    * common super class for all devices and acts as an image source for mitkUSImages. It is the base class
    * for all US Devices, and every new device should extend it.
    *
    * US Devices support output of calibrated images, i.e. images that include a specific geometry.
    * To achieve this, call SetCalibration, and make sure that the subclass also calls apply
    * transformation at some point (The USDevice does not automatically apply the transformation to the image)
    *
    * Note that SmartPointers to USDevices will not invalidate while the device is still connected.
    * \ingroup US
    */

   class MitkUS_EXPORT USDevice : public mitk::ImageSource
    {
    public:
      mitkClassMacro(USDevice, mitk::ImageSource);

     /**
      *\brief These constants are used in conjunction with Microservices
      */
      static const std::string US_INTERFACE_NAME;     // Common Interface name of all US Devices. Used to refer to this device via Microservices
      static const std::string US_PROPKEY_LABEL;      // Human readable text represntation of this device
      static const std::string US_PROPKEY_ISACTIVE;   // Whether this Device is active or not.
      static const std::string US_PROPKEY_CLASS;      // Class Name of this Object


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

      /**
      * \brief Works analogously to mitk::USDevice::Connect(). Don't override this Method, but onDisconnection instead.
      */
      bool Disconnect();

      /**
      * \brief Activates this device. After the activation process, the device will start to produce images. This Method will fail, if the device is not connected.
      */
      bool Activate();

      /**
      * \brief Deactivates this device. After the deactivation process, the device will no longer produce images, but still be connected.
      */
      void Deactivate();

      /**
      * \brief Add a probe to the device without connecting to it.
      *  This should usually be done before connecting to the probe.
      */
      virtual void AddProbe(mitk::USProbe::Pointer probe);

      /**
      * \brief Connect to a probe and activate it. The probe should be added first.
      *  Usually, a VideoDevice will simply add a probe it wants to connect to,
      *  but an SDK Device might require adding a probe first.
      */
      virtual void ActivateProbe(mitk::USProbe::Pointer probe);

      /**
      * \brief Deactivates the currently active probe.
      */
      virtual void DeactivateProbe();

      /**
      * \brief Removes a probe from the ist of currently added probes.
      */
      //virtual void removeProbe(mitk::USProbe::Pointer probe);

      /**
      *  \brief Returns a vector containing all connected probes.
      */
      std::vector<mitk::USProbe::Pointer> GetConnectedProbes();

      /**
      *\brief return the output (output with id 0) of the filter
      */
      USImage* GetOutput(void);

      /**
      *\brief return the output with id idx of the filter
      */
      USImage* GetOutput(unsigned int idx);


      /**
      *\brief Graft the specified DataObject onto this ProcessObject's output.
      *
      * See itk::ImageSource::GraftNthOutput for details
      */
      virtual void GraftNthOutput(unsigned int idx, itk::DataObject *graft);

      /**
      * \brief Graft the specified DataObject onto this ProcessObject's output.
      *
      * See itk::ImageSource::Graft Output for details
      */
      virtual void GraftOutput(itk::DataObject *graft);

      /**
      * \brief Make a DataObject of the correct type to used as the specified output.
      *
      * This method is automatically called when DataObject::DisconnectPipeline()
      * is called.  DataObject::DisconnectPipeline, disconnects a data object
      * from being an output of its current source.  When the data object
      * is disconnected, the ProcessObject needs to construct a replacement
      * output data object so that the ProcessObject is in a valid state.
      * Subclasses of USImageVideoSource that have outputs of different
      * data types must overwrite this method so that proper output objects
      * are created.
      */
      virtual DataObjectPointer MakeOutput(unsigned int idx);

      //########### GETTER & SETTER ##################//

      /**
      * \brief Returns the Class of the Device. This Method must be reimplemented by every Inheriting Class.
      */
      virtual std::string GetDeviceClass() = 0;

      /**
      * \brief True, if the device is currently generating image data, false otherwise.
      */
      bool GetIsActive();

      /**
      * \brief True, if the device is currently ready to start transmitting image data or is already
      * transmitting image data. A disconnected device cannot be activated.
      */
      bool GetIsConnected();


      /**
      * \brief Sets a transformation as Calibration data. It also marks the device as Calibrated. This data is not automatically applied to the image. Subclasses must call ApplyTransformation
      * to achieve this.
      */
      void setCalibration (mitk::AffineTransform3D::Pointer calibration);

      /**
      * \brief Returns the current Calibration
      */
      itkGetMacro(Calibration, mitk::AffineTransform3D::Pointer);

      /**
      * \brief Returns the currently active probe or null, if none is active
      */
      itkGetMacro(ActiveProbe, mitk::USProbe::Pointer);

      std::string GetDeviceManufacturer();
      std::string GetDeviceModel();
      std::string GetDeviceComment();

    protected:
      mitk::USProbe::Pointer m_ActiveProbe;
      std::vector<mitk::USProbe::Pointer> m_ConnectedProbes;
      bool m_IsActive;


      /*
      * \brief This Method constructs the service properties which can later be used to
      *  register the object with the Microservices
      *  Return service properties
      */
      mitk::ServiceProperties ConstructServiceProperties();


      /**
      * \brief Is called during the connection process. Override this method in your subclass to handle the actual connection.
      *  Return true if successful and false if unsuccessful. Additionally, you may throw an exception to clarify what went wrong.
      */
      virtual bool OnConnection() = 0;

      /**
      * \brief Is called during the disconnection process. Override this method in your subclass to handle the actual disconnection.
      *  Return true if successful and false if unsuccessful. Additionally, you may throw an exception to clarify what went wrong.
      */
      virtual bool OnDisconnection() = 0;

      /**
      * \brief Is called during the activation process. After this method is finished, the device should be generating images
      */
      virtual bool OnActivation() = 0;


      /**
      * \brief Is called during the deactivation process. After a call to this method the device should still be connected, but not producing images anymore.
      */
      virtual void OnDeactivation() = 0;


      /**
      * \brief This metadata set is privately used to imprint USImages with Metadata later.
      *        At instantiation time, it only contains Information about the Device,
      *        At scan time, it integrates this data with the probe information and imprints it on
      *        the produced images. This field is intentionally hidden from outside interference.
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
      *  \brief Grabs the next frame from the Video input. This method is called internally, whenever Update() is invoked by an Output.
      */
       void GenerateData() = 0;

      /**
      *  \brief The Calibration Transformation of this US-Device. This will automatically be written into the image once
      */
       mitk::AffineTransform3D::Pointer m_Calibration;

      /**
      *  \brief Convenience method that can be used by subclasses to apply the Calibration Data to the image. A subclass has to call
      * this method or set the transformation itself for the output to be calibrated! Returns true if a Calibration was set and false otherwise
      * (Usually happens when no transformation was set yet).
      */
       bool ApplyCalibration(mitk::USImage::Pointer image);


     private:

      /**
      *  \brief The device's ServiceRegistration object that allows to modify it's Microservice registraton details.
      */
       mitk::ServiceRegistration m_ServiceRegistration;



    };
} // namespace mitk

// This is the microservice declaration. Do not meddle!
US_DECLARE_SERVICE_INTERFACE(mitk::USDevice, "org.mitk.services.UltrasoundDevice")

#endif