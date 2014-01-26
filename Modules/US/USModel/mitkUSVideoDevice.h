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

#ifndef MITKUSVideoDevice_H_HEADER_INCLUDED_
#define MITKUSVideoDevice_H_HEADER_INCLUDED_

#include <MitkUSExports.h>
#include <mitkCommon.h>
#include "mitkUSDevice.h"
#include "mitkUSImageVideoSource.h"
#include <itkObjectFactory.h>

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk {
  class USVideoDeviceCustomControls;
  class USAbstractControlInterface;

  /**
  * \brief A mitk::USVideoDevice is the common class for video only devices.
  * They capture video input either from a file or from a device and
  * transform the output into an mitk::USImage with attached metadata.
  * This simple implementation does only capture and display 2d images without
  * registration for example.
  *
  * \ingroup US
  */
  class MitkUS_EXPORT USVideoDevice : public mitk::USDevice
  {
  public:
    mitkClassMacro(USVideoDevice, mitk::USDevice);
    // To open a device (DeviceID, Manufacturer, Model)
    mitkNewMacro3Param(Self, int, std::string, std::string);
    // To open A VideoFile (Path, Manufacturer, Model)
    mitkNewMacro3Param(Self, std::string, std::string, std::string);
    // To open a device (DeviceID, Metadata)
    mitkNewMacro2Param(Self, int, mitk::USImageMetadata::Pointer);
    // To open A VideoFile (Path, Metadata)
    mitkNewMacro2Param(Self, std::string, mitk::USImageMetadata::Pointer);

    /**
    * \return the qualified name of this class
    */
    virtual std::string GetDeviceClass();

    /**
    * Getter for the custom control interface which was created during the
    * construction process of mitk::USVideoDevice.
    *
    * \return custom control interface of the video device
    */
    virtual itk::SmartPointer<USAbstractControlInterface> GetControlInterfaceCustom();

    /**
    * \brief Remove this device from the micro service.
    * This method is public for mitk::USVideoDevice, because this devices
    * can be completly removed. This is not possible for API devices, which
    * should be available while their sub module is loaded.
    */
    void UnregisterOnService();

    /**
    * \return mitk::USImageSource connected to this device
    */
    virtual USImageSource::Pointer GetUSImageSource();

    itkGetMacro(Image, mitk::Image::Pointer);
    itkGetMacro(DeviceID,int);
    itkGetMacro(FilePath,std::string);

    static const std::string DeviceClassIdentifier;
  protected:

    /**
    * \brief Creates a new device that will deliver USImages taken from a video device.
    *  under windows, try -1 for device number, which will grab the first available one
    * (Open CV functionality)
    */
    USVideoDevice(int videoDeviceNumber, std::string manufacturer, std::string model);
    /**
    * \brief Creates a new device that will deliver USImages taken from a video file.
    */
    USVideoDevice(std::string videoFilePath, std::string manufacturer, std::string model);
    /**
    * \brief Creates a new device that will deliver USImages taken from a video device.
    *  under windows, try -1 for device number, which will grab the first available one
    * (Open CV functionality)
    */
    USVideoDevice(int videoDeviceNumber, mitk::USImageMetadata::Pointer metadata);
    /**
    * \brief Creates a new device that will deliver USImages taken from a video file.
    */
    USVideoDevice(std::string videoFilePath, mitk::USImageMetadata::Pointer metadata);

    virtual ~USVideoDevice();

    /**
    * \brief Initializes common properties for all constructors.
    */
    void Init();

    /**
    * \brief Is called during the initialization process.
    *  Returns true if successful and false if unsuccessful. Additionally, you may throw an exception to clarify what went wrong.
    */
    virtual bool OnInitialization();

    /**
    * \brief Is called during the connection process.
    *  Returns true if successful and false if unsuccessful. Additionally, you may throw an exception to clarify what went wrong.
    */
    virtual bool OnConnection();

    /**
    * \brief Is called during the disconnection process.
    *  Returns true if successful and false if unsuccessful. Additionally, you may throw an exception to clarify what went wrong.
    */
    virtual bool OnDisconnection();

    /**
    * \brief Is called during the activation process. After this method is finsihed, the device should be generating images
    */
    virtual bool OnActivation();

    /**
    * \brief Is called during the deactivation process. After a call to this method the device should still be connected, but not producing images anymore.
    */
    virtual bool OnDeactivation();

    /**
    * \brief The image source that we use to aquire data
    */
    mitk::USImageVideoSource::Pointer m_Source;

    /**
    * \brief True, if this source plays back a file, false if it recieves data from a device
    */
    bool m_SourceIsFile;

    /**
    * \brief The device id to connect to. Undefined, if m_SourceIsFile == true;
    */
    int m_DeviceID;

    /**
    * \brief The Filepath id to connect to. Undefined, if m_SourceIsFile == false;
    */
    std::string m_FilePath;

    /**
    * \brief custom control interface for us video device
    */
    itk::SmartPointer<USVideoDeviceCustomControls> m_ControlInterfaceCustom;
  };
} // namespace mitk

#endif // MITKUSVideoDevice_H_HEADER_INCLUDED_
