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


#ifndef MITKUSDevicePersistence_H_HEADER_INCLUDED_
#define MITKUSDevicePersistence_H_HEADER_INCLUDED_

// MITK
#include <mitkCommon.h>
#include <mitkUSVideoDevice.h>

// ITK
#include <itkObjectFactory.h>

// QT
#include <QSettings>


namespace mitk {

    /**Documentation
    * TODO
    */

   class USDevicePersistence : public itk::Object
    {
    public:
      mitkClassMacroItkParent(USDevicePersistence, itk::Object);
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      void StoreCurrentDevices();

      std::vector<mitk::USDevice::Pointer> RestoreLastDevices();

    protected:

      USDevicePersistence();
      virtual ~USDevicePersistence(){}

      QString USVideoDeviceToString(mitk::USVideoDevice::Pointer d);
      mitk::USVideoDevice::Pointer StringToUSVideoDevice(QString s);

      QSettings m_devices;

      void split(std::string& text, std::string& separators, std::vector<std::string>& words);


    };
} // namespace mitk


#endif
