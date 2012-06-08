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
    * \brief A VideoDevcie is the common class for video only devcies. They capture Video Input either from
    *  a file or from a devcie, and transform the output into an mitkUSImage with attached Metadata.
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
      virtual std::string GetClassName(){
        return "org.mitk.modules.us.USVideoDevice";
      }

      void GenerateData();

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

      mitk::USImageVideoSource::Pointer m_Source;

    };
} // namespace mitk
#endif
