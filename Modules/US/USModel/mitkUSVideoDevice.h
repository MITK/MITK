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
#include <itkObjectFactory.h>
#include "mitkUSImageVideoSource.h"

namespace mitk {

    /**Documentation
    * \brief A VideoDevice is the common class for video only devices. They capture Video Input either from
    *  a file or from a device, and transform the output into an mitkUSImage with attached Metadata.
    *  This simple implementation does only capture and display 2D Images without cropping or registration.
    *  One can simply inherit from this class and overwrite the handle2D and handle 3Dmethods to get full access to the data
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
      * \brief Returns the qualified name of this class. Be sure to override this when inheriting from VideoDevice!
      */
      virtual std::string GetDeviceClass();

      void GenerateData();

      itkGetMacro(Source, mitk::USImageVideoSource::Pointer);
      itkGetMacro(Image, mitk::USImage::Pointer);

      void GrabImage();

    protected:

      static ITK_THREAD_RETURN_TYPE Acquire(void* pInfoStruct);



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
      virtual void OnDeactivation();

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

      // Threading-Related
      itk::MultiThreader::Pointer m_MultiThreader; ///< itk::MultiThreader used for thread handling
      itk::FastMutexLock::Pointer m_ImageMutex; ///< mutex for images provided by the range camera
      itk::FastMutexLock::Pointer m_CameraActiveMutex; ///< mutex for the cameraActive flag
      int m_ThreadID; ///< ID of the started thread
      mitk::USImage::Pointer m_Image;

    };
} // namespace mitk
#endif
