/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
      // To open a devcie (DeviceID, Manufacturer, Model)
      mitkNewMacro3Param(Self, int, std::string, std::string);
      // To open A VideoFile (Path, Manufacturer, Model)
      mitkNewMacro3Param(Self, std::string, std::string, std::string);

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
      virtual ~USVideoDevice();

      mitk::USImageVideoSource::Pointer m_Source;

    };
} // namespace mitk
#endif