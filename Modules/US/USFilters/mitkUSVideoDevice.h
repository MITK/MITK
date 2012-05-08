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
    * \brief TODO
    * \ingroup US
    */
  class MitkUS_EXPORT USVideoDevice : public mitk::USDevice
    {
    public:
      mitkClassMacro(USVideoDevice, mitk::USDevice);
      mitkNewMacro3Param(Self, int, std::string, std::string);
      mitkNewMacro3Param(Self, std::string, std::string, std::string);

      void GenerateData();

    
      //## getter and setter ##

     // itkGetMacro(Name, std::string);
     // itkSetMacro(Name, std::string);

    protected:
      USVideoDevice(int videoDeviceNumber, std::string manufacturer, std::string model);
      USVideoDevice(std::string videoFilePath, std::string manufacturer, std::string model);
      virtual ~USVideoDevice();

      mitk::USImageVideoSource::Pointer m_Source;

    

    };
} // namespace mitk
#endif