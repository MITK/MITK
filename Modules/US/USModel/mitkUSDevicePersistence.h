/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKUSDevicePersistence_H_HEADER_INCLUDED_
#define MITKUSDevicePersistence_H_HEADER_INCLUDED_

// MITK
#include <mitkCommon.h>
#include <mitkUSVideoDevice.h>
#include <mitkUSProbe.h>

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
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      void StoreCurrentDevices();

    std::vector<mitk::USDevice::Pointer> RestoreLastDevices();

  protected:

    USDevicePersistence();
    ~USDevicePersistence() override{}

    QString USVideoDeviceToString(mitk::USVideoDevice::Pointer d);
    mitk::USVideoDevice::Pointer StringToUSVideoDevice(QString s);
    QString USProbeToString(mitk::USProbe::Pointer p);
    mitk::USProbe::Pointer StringToUSProbe(std::string s);

    QSettings m_devices;

    void split(std::string& text, std::string& separators, std::vector<std::string>& words);
    double spacingToDouble(std::string s);
    int depthToInt(std::string s);
  };
} // namespace mitk

#endif
